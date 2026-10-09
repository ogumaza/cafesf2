#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

"""Generate SoundFont and SFZ filter tables for src/filter.cpp.

usage: filter_tables.py <bbq.rpx>

Read a locally supplied Animal Crossing: amiibo Festival executable (code/bbq.rpx); the executable is not distributed
with cafesf2. Extract kLpfFreq (0x10092C68) and the five biquad preset tables in Q14 (from 0x10094B3C).

The track low-pass (0xd8) is a one-pole filter, y = a0 x + b0 y1. AX runs it on the voice at 32 kHz. AXComputeLpfCoefs
computes b0 = (k - sqrt(k^2 - 1)) * 32768 in single precision, with k = 2 - cos(2 pi freq / 32000), and a0 = 32767 - b0.
The biquad presets are Robert Bristow-Johnson cookbook filters: low/high-pass at about -0.5 dB gain and band-pass at
0 dB peak gain. Each coefficient set reduces to a cutoff or centre frequency, Q and gain.

SFZ: sfizz's lpf_1p has pole exp(-2 pi f / rate); lpf_2p, hpf_2p and bpf_2p use cookbook filters with Q expressed in
dB. Match the game's coefficients at 32000 Hz.

SoundFont: FluidSynth supplies one cookbook low-pass (fluid_iir_filter.c), with Q = 10^((resonance / 10 - 3.01) / 20).
It matches the biquad low-pass. Approximate the one-pole by fitting cutoff at 44100 Hz against the game's response at
32000 Hz. Minimize RMS dB error relative to 20 Hz over 400 log-spaced frequencies from 20 Hz to 14 kHz. Floor both
responses at -30 dB.

Requires Python 3 and numpy.
"""
import argparse
import math
import struct
import sys
import zlib
from pathlib import Path

try:
    import numpy as np
except ImportError:
    sys.exit("filter_tables.py requires numpy (pip install numpy)")

LPF_FREQ = 0x10092C68  # kLpfFreq: 24 big-endian uint16 cutoffs in Hz. The last (16000) switches the filter off.
BIQUADS = {  # type: (address, sets, name)
    1: (0x10094B3C, 112, "low-pass"),
    2: (0x10094F9C, 97, "high-pass"),
    3: (0x10095368, 122, "band-pass 512"),
    4: (0x1009582C, 93, "band-pass 1024"),
    5: (0x10095BD0, 93, "band-pass 2048"),
}
GAME_RATE = 32000.0  # Hz, AX's rate for voices
OUT_RATE = 44100.0  # Hz, the rate FluidSynth usually plays at
FREQS = np.geomspace(20.0, 14000.0, 400)
FLOOR_DB = -30.0
CUTOFFS = np.arange(1500, 13501, 10)  # the SoundFont cutoffs tried, in absolute cents
SFZ_LOW_PASS_BASE = 20000.0  # Hz: reference cutoff for the SFZ low-pass table (filter open)
SFZ_BIQUAD_BASE = 1000.0  # Hz: reference cutoff for the SFZ biquad table
SHF_RPL_ZLIB = 0x08000000  # an RPX section flag: the data is a big-endian size and a zlib stream
f32 = np.float32


def sections(path):
    """Return an RPX's loaded sections as (address, data) pairs. Decompress the compressed ones."""
    d = path.read_bytes()
    if d[:4] != b"\x7fELF" or d[7] != 0xCA:
        sys.exit(f"{path} isn't a Wii U executable (RPX)")
    shoff, = struct.unpack_from(">I", d, 0x20)
    shentsize, shnum = struct.unpack_from(">HH", d, 0x2E)
    out = []
    for i in range(shnum):
        _, kind, flags, address, offset, size = struct.unpack_from(">IIIIII", d, shoff + i * shentsize)
        if address == 0 or kind == 8:  # not loaded, or SHT_NOBITS
            continue
        data = d[offset:offset + size]
        if flags & SHF_RPL_ZLIB:
            data = zlib.decompress(data[4:])
        out.append((address, data))
    return out


def read(rpx, fmt, address):
    for start, data in rpx:
        if start <= address and address + struct.calcsize(fmt) <= start + len(data):
            return struct.unpack_from(fmt, data, address - start)
    sys.exit(f"address {address:#x} isn't in the executable")


def one_pole(freq):
    """Compute the pole b0 in Q15 as AXComputeLpfCoefs does in single precision: k = 2 - cos(w), b0 = (k - sqrt(k^2 -
    1)) * 32768, truncated, with k^2 - 1 in one fused step."""
    w = f32(6.2831855) * f32(freq) / f32(GAME_RATE)
    k = f32(2.0) - f32(math.cos(float(w)))
    k2 = f32(float(k) * float(k) - 1.0)  # exact in double, then rounded once like a fused multiply-add
    return int(np.int32((k - f32(np.sqrt(k2))) * f32(32768.0)))


def response_db(b, a, rate, freqs=FREQS):
    """Compute the response of y = b0 x + b1 x1 + b2 x2 - a1 y1 - a2 y2 at `rate`. Return dB relative to the
    first frequency, clamped to the floor."""
    z = np.exp(-2j * np.pi * freqs / rate)
    h = (b[0] + b[1] * z + b[2] * z * z) / (1.0 + a[0] * z + a[1] * z * z)
    db = 20.0 * np.log10(np.maximum(np.abs(h), 1e-12))
    return np.maximum(db - db[0], FLOOR_DB)


def fluidsynth_db(cents, resonance):
    """Compute FluidSynth filter coefficients for cutoff in absolute cents and resonance in cB."""
    fres = min(max(8.176 * 2.0 ** (cents / 1200.0), 5.0), 0.45 * OUT_RATE)
    q = 10.0 ** ((min(max(resonance / 10.0, 0.0), 96.0) - 3.01) / 20.0)
    w = 2.0 * np.pi * fres / OUT_RATE
    alpha = np.sin(w) / (2.0 * q)
    b1 = (1.0 - np.cos(w)) / (1.0 + alpha)
    return response_db((b1 / 2.0, b1, b1 / 2.0), (-2.0 * np.cos(w) / (1.0 + alpha), (1.0 - alpha) / (1.0 + alpha)),
                       OUT_RATE)


def best_cutoff(target):
    """Find the zero-resonance SoundFont cutoff closest to `target`. Return cutoff in absolute cents and RMS
    error in dB."""
    errors = [np.sqrt(np.mean((fluidsynth_db(c, 0) - target) ** 2)) for c in CUTOFFS]
    i = int(np.argmin(errors))
    return int(CUTOFFS[i]), float(errors[i])


def decode_biquad(kind, coefs):
    """Decode a game biquad's cutoff/centre (Hz at the voice rate), Q and gain (dB) from Q14 coefficients in y = b0 x
    + b1 x1 + b2 x2 + a1 y1 + a2 y2. Measure gain at cookbook unity: DC for low-pass, Nyquist for high-pass,
    centre for band-pass."""
    b0, b1, b2, a1, a2 = (v / 16384.0 for v in coefs)
    alpha = (1.0 + a2) / (1.0 - a2)  # a2 = -(1 - alpha) / (1 + alpha)
    cos_w0 = a1 * (1.0 + alpha) / 2.0  # a1 = 2 cos w0 / (1 + alpha)
    w0 = math.acos(max(-1.0, min(1.0, cos_w0)))
    q = math.sin(w0) / (2.0 * alpha)
    if kind == 1:
        z = 1.0
    elif kind == 2:
        z = -1.0
    else:
        z = complex(math.cos(w0), -math.sin(w0))
    gain = abs((b0 + b1 * z + b2 * z * z) / (1.0 - a1 * z - a2 * z * z))
    return w0 * GAME_RATE / (2.0 * math.pi), q, 20.0 * math.log10(gain)


def cents(ratio):
    return int(round(1200.0 * math.log2(ratio)))


def table(ctype, name, comment, values, per_line=10):
    """Format a C++ table with `per_line` values per line, followed by a blank line."""
    lines = [f"// {comment}", f"constexpr {ctype} {name}[{len(values)}] = {{"]
    for i in range(0, len(values), per_line):
        lines.append("    " + ", ".join(str(v) for v in values[i:i + per_line]) + ",")
    lines.append("};\n")
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser(description="Generate the filter tables for src/filter.cpp.")
    ap.add_argument("rpx", type=Path, help="Animal Crossing: amiibo Festival's code/bbq.rpx")
    args = ap.parse_args()
    rpx = sections(args.rpx)

    lpf_freq = read(rpx, ">24H", LPF_FREQ)
    if lpf_freq[0] != 80 or lpf_freq[-1] != 16000:
        sys.exit(f"{args.rpx} isn't Animal Crossing: amiibo Festival's executable (kLpfFreq reads {lpf_freq[:3]}...)")

    # Derive the track low-pass pole and fit the SoundFont cutoff.
    low_pass_sf2, low_pass_sfz, sf2_errors = [], [], []
    for freq in lpf_freq[:-1]:
        b0 = one_pole(freq)
        cutoff, error = best_cutoff(response_db(((32767 - b0) / 32768.0, 0.0, 0.0), (-b0 / 32768.0, 0.0), GAME_RATE))
        low_pass_sf2.append(cutoff)
        sf2_errors.append(error)
        low_pass_sfz.append(cents(-GAME_RATE * math.log(b0 / 32768.0) / (2.0 * math.pi) / SFZ_LOW_PASS_BASE))

    print(table("uint16_t", "kLowPassCutoff", "The SoundFont cutoffs, kLpfFreq[0] to kLpfFreq[22].", low_pass_sf2))
    print(table("int16_t", "kSfzLowPassCutoff", "The SFZ cutoffs, kLpfFreq[0] to kLpfFreq[22].", low_pass_sfz))

    # The biquad presets: cutoff (cents from SFZ_BIQUAD_BASE), Q (0.01 dB) and gain (0.01 dB) of each step.
    for kind, (address, sets, name) in BIQUADS.items():
        steps = [decode_biquad(kind, read(rpx, ">5h", address + 10 * i)) for i in range(sets)]
        values = []
        for freq, q, gain in steps:
            cutoff, resonance = cents(freq / SFZ_BIQUAD_BASE), round(2000.0 * math.log10(q))
            values.append(f"{{{cutoff}, {resonance}, {round(100.0 * gain)}}}")
        print(table("SfzBiquad", f"kSfzBiquad{kind}", f"Type {kind}, {name}: steps 0 to {sets - 1}.", values, 5))
        if kind == 1:
            sf2 = [f"{{{cents(freq / 8.176)}, {round(10.0 * (20.0 * math.log10(q) + 3.01))}}}" for freq, q, _ in steps]
            print(table("SoundFontFilter", "kBiquadLowPass", "The SoundFont filters, steps 0 to 111.", sf2, 6))

    print(f"// RMS difference, SoundFont low-pass: {min(sf2_errors):.2f} to {max(sf2_errors):.2f} dB", file=sys.stderr)


if __name__ == "__main__":
    main()

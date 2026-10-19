# cafesf2

cafesf2 converts Nintendo Wii U sound archives (`.bfsar`) to MIDI, SoundFont 2 and SFZ.
Each sequence gets a MIDI file and a SoundFont containing its instruments. SFZ output includes
filters that SoundFont cannot reproduce (see [SFZ files](#sfz-files)). cafesf2 also converts the
music of Wii U games that play their sound through JAudio (see [JAudio games](#jaudio-games)).

Drop archives onto `cafesf2.exe` on Windows or `Cafesf2.app` on macOS. Output goes in a folder
beside each archive. An input can also be a JAudio game's start-up archive (`.baa` or `.aaf`).
From a terminal:

```
cafesf2 [options] <archive.bfsar> [<archive.bfsar> ...]

  --output DIR   write to DIR (one archive only)
  --only REGEX   convert matching sequence names (for example --only BGM)
  --loops N      repeat the main loop N times (default 1)
                 MIDI loop markers are included for players that support them
  --seed N       initial game PRNG state (default 0x12345678)
  --var N=V      set variable N (0-15 player, 16-31 global) to V before each
                 sequence starts; repeat for more variables
  --voices N     drop the notes that the game drops for lack of a voice,
                 with N voices (the game has 96 unless it sets another
                 count). The result only approximates the game.
  --voices off   keep every note (default)
  --list         list sequences without converting
  --quiet        print errors only
  --version      show the version
```

Some sequences play nothing until the game sets one of their variables. `--var` sets one before
each sequence starts. As in the game, player and global variables start at -1. Use `--only` to
choose the sequences that `--var` applies to.

cafesf2 keeps every note. In the game, a note is dropped when every voice is in use. nw::snd has
96 voices unless a game sets another count in its program. `--voices N` drops the notes that
cafesf2's model of the game drops with N voices. Which notes the console drops isn't known
exactly. The result only approximates the game.

Files use the sequence names, with adjustments for Windows: forbidden characters and trailing
dots or spaces become `_`, and reserved device names such as `CON` get an added `_`. If names
differ only in ASCII letter case, later sequences get a sound-index suffix to avoid collisions
on Windows and macOS. The same applies to a sequence named `samples`. That name would otherwise
collide with the SFZ sample folder. The default output folder is `<archive name>_cafesf2`, and
the same rules apply to its name. Use `--output` to choose another location.

Wii U games play their sequences with Nintendo's sound library, nw::snd. cafesf2 plays them as
nw::snd does. The model of the Wii U's timing, voices and commands comes from Animal Crossing:
amiibo Festival's executable.

## JAudio games

Some Wii U games play their sound through JAudio, the sound library in Nintendo EAD's JSystem,
and not through nw::snd. Twilight Princess HD and The Wind Waker HD are two of them. cafesf2
converts their music. Give it the game's start-up archive, with the game's other sound files
beside it as in the title's `content` folder. The files go in `<archive name>_cafesf2` beside
the archive.

JAudio comes in two generations. They lay out their files differently:

- **JAudio2** (Twilight Princess HD): the start-up archive (`.baa`) contains the sound table and
  the sounds' names, the instrument banks and the wave systems. The sequence archive (an RARC
  archive in a `Seqs` folder beside it) contains the sequences. The wave archives (`.aw` files
  in a `Waves` folder) contain the samples.
- **The first JAudio** (The Wind Waker HD): the start-up archive (`.aaf`) contains the sound
  table, the instrument banks and the wave systems. A sequence table (`.bdt`) in a `Seqs` folder
  beside it contains the sequences. The wave archives (`.aw` files in a `Banks` folder) contain
  the samples. A name file (`.asn`) beside the start-up archive gives the sounds' names.

The sequences come from the game's sound table, in its order. JAudio2's sound effects are
sequences too. They convert with the music. The first JAudio plays a game's sound effects in one
sequence that the game's code steers. cafesf2 leaves that sequence out. Files take the names
that the game gives the sounds. A sound without a name becomes
`SOUND_<section>_<group>_<index>`. A sequence that no sound plays comes last. It keeps the name
of its file in the sequence archive. In a sequence table, it becomes `SEQUENCE_<number>`.

`--loops`, `--seed` and `--only` work as for a sound archive. The game's random state starts at
0 unless `--seed` gives another. The game steers some sequences through ports. cafesf2 starts
each port at 0, as the game does. JAudio's sequences have no variables. `--var` changes nothing
in them. By default, cafesf2 keeps every note. `--voices N` drops the notes that JAudio would
drop with N voices. On the GameCube, JAudio has 64. The result only approximates the game.

JAudio times its music for the GameCube's rate of about 32,028 Hz. The Wii U plays it at
32,000 Hz. There the music plays 0.089% slower and 1.5 cents lower than in the files.

MIDI and SoundFont can't hold everything that JAudio does. The fitted envelopes can't follow an
oscillator's table that rises again or loops. Random instrument effects play at their base
values. Tremolos, track filters, pitch envelopes and notes on the DSP's built-in oscillators are
left out. A vibrato deeper than a semitone is kept to a semitone. In the first JAudio's
sequences, a track that shares its parent's voices gets separate voices. The oscillators that a
track sets and the tempos of child tracks are left out there. cafesf2 reports each of these for
each sequence.

## Downloads

The Releases page has cafesf2 for 64-bit Windows. On macOS on Apple Silicon, build it from
source (see [Building](#building)).

## Drag and drop

- **Windows:** drop one or more `.bfsar`, `.baa` or `.aaf` files onto `cafesf2.exe`. The console
  shows the results and waits for Enter before closing.
- **macOS:** drop archives onto `Cafesf2.app`, or open it to choose files. The app shows the
  results and can open the output folders. The app wraps the command-line program because Finder
  cannot drop files onto a bare executable. The [build](#building) includes the app.

cafesf2 reports an unreadable archive and goes on with the others. If two inputs would use the
same output folder, cafesf2 skips the second to protect the first input's files. This can happen
when names differ only in extension, or only in case on a filesystem that ignores case. Windows
and macOS filesystems usually do.

## The conversion

### Sequence playback

FSEQ is bytecode, with variables, conditions, random arguments, calls, jumps, loops and tracks
that start other tracks. cafesf2 executes the bytecode and records the resulting notes and
controllers. The performer follows the Wii U's nw::snd player, including:

- All commands and their variable, comparison, random and `if` forms, among them the Wii U's
  additions: calls and loops nested ten deep, main and second volumes that ramp like track
  volume, a third effect send, and four LFOs per track with curves, phases and periods.
- Note lengths, damper, key groups, tie/mono, `notewait off` and waits for sounding notes to
  finish. Note ends are calculated frame by frame. nw::snd works in 3 ms sound frames. The
  Wii U's audio system, AX, starts playing a voice two frames after note-on. A one-shot wave
  ends when AX has played all of it, at the pitch that the note's sweep, the track's bend and
  the pitch LFOs give. nw::snd works that pitch out again each frame. Envelopes start at
  -90.4 dB and first advance at frame end. Notes released or stopped before then are inaudible
  and produce no MIDI note.
- Tick scheduling in 1/65536 ms, in single precision. The game's rounding occasionally
  places a tick in an adjacent sound frame.
- The game's random generator, including its once-per-frame advance. Random choices therefore
  depend on timing.
- Main-loop detection and MIDI loop markers. The main loop belongs to the lowest-numbered track
  that can still advance. A pass qualifies only if earlier tracks make no contribution. Tracks
  that wait forever, on held notes or for a negative number of ticks, do not block this choice.
  A pass that changes nothing, such as one that only polls or counts a variable, qualifies only
  while no other track changes the sound or plays on. Setting a variable that another track
  reads counts as a change. A tie or mono legato on a note below hearing (see below) changes
  nothing that can be heard. A track that has sounded nothing and changed nothing for 10 seconds
  doesn't count as playing on. A sequence whose tracks all reach their end has no main loop.
  Backward jumps to unvisited commands and passes without waits, such as random retries or
  variable polling, are not loops. Inside a subroutine, a jump back isn't a loop when its pass
  played no note that can be heard and ran a conditional jump, call, return or end that could
  leave the loop. A tie or mono legato on a note below hearing (see below) plays nothing that
  can be heard. A held note that sounds until the game stops it keeps such a loop. A loop that a
  track runs inside a pass of another loop, such as a countdown of waits between notes, is an
  inner loop. An inner loop's passes don't qualify until the track has played no pass of another
  loop for 10 seconds. A main loop in which no note starts isn't a loop either once every
  earlier note has ended or is fading out. The sequence then ends with its last sound. The
  markers go on the first pass that a player looping on them repeats as the game plays the next
  one: the same notes at the same times, with the same settings and tempo. The first pass often
  differs. Setup before the loop can land in its first tick. The game can keep a value from pass
  to pass. Later passes may only tie again a note that the first pass strikes. If no pass so far
  repeats that way but the next pass does, the performance plays that pass too. Otherwise the
  markers stay on the first pass. They also stay there in a loop that draws random numbers. At
  the loop's start, the MIDI file sends each controller, pitch, preset and tempo that the game
  gives back there when the loop ends with another value. Without those events, a player that
  loops on the markers would keep the value from the loop's end.

Short loops, such as repeating sound effects, run until the MIDI is at least 10 seconds long.
Indefinitely held sounds are capped at 10 seconds after their last change. A note of 9,600 ticks
or more that its track waits out counts as held. Games use such notes, minutes long at a
typical tempo, for sounds that they stop themselves. The cap also
covers a main loop longer than 10 seconds that ends with more than 10 seconds of no change and
sounds only endless notes held with tie or mono. The game's next pass usually continues those
notes. A sequence that finishes naturally ends with its last sound, including release. A note
counts as below hearing once its envelope has decayed 40 dB. A note that plays until its track
releases it, such as a tie, ends where its envelope has decayed 60 dB. If its sustain level
stays above that, the note ends at 40 dB down. The game's channel goes on in silence.
Performances stop after 15 minutes. One that has been silent for more than 10 seconds by then
ends with its last sound too. Tempo or timebase 0 stops sequence timing while notes keep
playing. Those notes finish normally, or take the same 10-second limit if held indefinitely.

### MIDI

Each sequence track gets a MIDI track and channel of its own. Channel 10, reserved for drums in
General MIDI, is allocated last. SoundFont synths use bank 128 for that channel. cafesf2 copies
that channel's presets to bank 128.

MIDI output keeps tempo changes and uses at least 7680 ticks per quarter note, with an integer
number of MIDI ticks per sequence tick. The game processes ticks in 3 ms sound frames. AX plays
their changes two frames later. cafesf2 places events at the start of the frame that contains
each tick, rounded to the nearest MIDI tick. This shifts the whole sequence two frames earlier
and keeps the spacing of events. At 120 BPM, the rounding error is at most 1/30 ms. No moved
event crosses a loop marker. A loop keeps its contents. A sweep or a portamento with a
portamento time moves on in every frame, between ticks too. cafesf2 writes a pitch bend at the
start of each of those frames. After its track closes, the sweep goes on while the note's
release sounds. Sweeps also go on after the sequence's last tick, at the last tempo.

nw::snd applies track settings to notes at frame end. A note starts with the settings that its
tick ends with. MIDI controller changes are written before the notes of the same tick.

| Game setting | MIDI output |
|---|---|
| Track volume, up to 127 | CC7 |
| Track volume above 127 × second volume × main volume × sound volume, plus legato velocity changes | CC11. Like nw::snd's volumes, CC7 and CC11 both act squared. Values are scaled for sequences exceeding MIDI's range (see [Differences from the game](#differences-from-the-game)). |
| Track pan | CC10, mapped to the SoundFont pan law |
| Pitch bend, sweep, portamento, legato key changes and a pitch LFO that changes during a note | Pitch bend, with RPN 0 range set per channel |
| First LFO's depth | CC1 = depth / 2, mapped to vibrato or tremolo in the SoundFont |
| Effect send A | CC91 (reverb) |
| Immediate track silence | CC120 |
| Filters that change during sounding notes | CC102, mapped to SoundFont cutoff by a modulator |
| Track low-pass (0xd8), for SFZ | CC103 = 64 minus the argument; values <= 6 mean off |
| Biquad filter (0xb4/0xb5), for SFZ | CC104 = filter value; 0 means off |
| Instrument and track envelope, LFO, initial pan and filter settings | Bank select (CC0) and program change |

A MIDI channel can hold only one note per key. Repeated strikes that overlap, common in drums,
get extra channels called layers, with matching controllers, up to the 16-channel limit.

A MIDI channel has one set of controllers for all its notes. In the game, each note keeps its
settings. While notes that the game plays differently from the track's later notes still sound,
the track moves on to a free channel when there is one. That covers release tails that the track
has detached. Those keep their settings. The same goes for notes still sounding when a new note
starts that sweeps, starts at a raised velocity or requires a level of its own. Each channel
gets a MIDI track named after the sequence track, in the same way as a layer.

### SoundFont

Each distinct combination of bank instrument and track settings becomes a preset. Instrument
regions become zones with:

- Key and velocity ranges resolved by nw::snd's first-match rules.
- The region's sample, root key, tuning, linear volume and pan.
- Envelope parameters after track overrides. nw::snd advances an envelope 3 ms in each 3 ms
  sound frame. nw::snd's attack is exponential in decibels. The SoundFont attack is linear, and
  fits the game's attack best at 0.55 of its time. A note that the game releases during its
  attack gets a longer attack, in a preset of its own. That attack reaches the game's level at
  the release. The release then starts from there. SoundFont decays and releases fall 100 dB in
  101.6 s at the slowest. The game's slowest decays and releases take up to six times as long.
  Such a decay gets a longer hold, up to SoundFont's limit of 18 s: half the time by which the
  game's decay reaches its sustain level later. The note is then as far above the game's level
  at the end of the hold as below it when its decay reaches the sustain level. Such a release
  plays at SoundFont's slowest release time. The SFZ files keep the game's times.
- Key groups mapped to exclusive classes.
- A low-pass for AX's resampling filters on the keys whose notes play their wave at more than
  AX's mixing rate of 32 kHz. Such a region splits into zones where its notes' rate passes 1
  and 4/3. See "Resampling filters" under "Differences from the game".
- Loops repeated as needed to meet SoundFont padding rules: 8 samples before, at least 32
  within, and 32 after. This preserves playback.
- Continued playback for regions that ignore note-off. Their MIDI notes end when the game
  would release them, with release settings that also preserve decay after a repeated strike.

cafesf2 reads Wii U FSAR archives and their FSEQ, FBNK, FWAR and FWAV files. Supported wave
encodings are PCM8, PCM16 and DSP-ADPCM. Banks and wave archives stored only in FGRP groups are
also supported. Groups can have wave archives of their own. Copies of a bank may then reference
different files. cafesf2 uses the first group's copy. The copies play the same waves.

Some games keep groups outside the archive, in files that the game loads itself. cafesf2 looks
for each such group as `<group name>.bfgrp` beside the archive, or in a `userManagementFiles`
folder beside it. Some games keep these group files in SARC archives under other names. While a
group is still missing, cafesf2 also reads the group files in the `.sarc` files beside the
archive. A folder can hold another archive's group files too. cafesf2 takes a group file from a
SARC only if its files are of the kinds that the archive gives them. When cafesf2 finds none of
the groups, a warning says so. Archives from other consoles are refused: the 3DS's `.bcsar`, the
Wii's `.brsar` and the Switch's `.bfsar`. The Switch's archives are little-endian, and the Wii
U's archives are big-endian.

## SFZ files

SoundFont has one two-pole low-pass filter per voice. That filter cannot reproduce the game's
high-pass or band-pass presets, and only approximates the one-pole track low-pass. SFZ supports
all of these filters. cafesf2 also writes these SFZ files:

- `sfz/<sequence>/Track <n>.sfz`: instruments for MIDI `Track <n>` and its layers, for each
  active sequence track.
- `sfz/samples/`: 16-bit WAV samples shared by all sequences in the archive. Stereo channels
  are separate files, panned left and right.

SFZ players ignore MIDI channels, and each file contains one instrument. Load the MIDI in a DAW
and assign an SFZ player, such as sfizz, to each track with its matching file. Output targets
sfizz 1.2.

Presets use program change, plus bank select (CC0) when a sequence has more than 128 presets.
Regions match the SoundFont, with these differences:

- **Filters.** sfizz's `lpf_1p` matches the track low-pass. `lpf_2p`, `hpf_2p` and `bpf_2p`
  match the biquad presets coefficient for coefficient at AX's 32 kHz. When both filters are
  active, they run in series. CC103/CC104 and SFZ curves map track values to cutoff, resonance
  and gain, including changes during notes. An inactive low-pass stays at sfizz's highest
  cutoff, 20 kHz. There it lowers 10 kHz by about 0.5 dB and 16 kHz by 1 dB. SFZ has no biquad
  bypass like the game's value 0 or absent preset. Instead, CC104 selects filtered or unfiltered
  regions at note-on, and that choice lasts for the note. As in the game, the choice is the one
  that the note's sound frame ends with. Each track uses its first biquad type. Later types are
  omitted and reported.
- **Envelopes.** Decays and releases in sfizz fall linearly in dB, like those in the game, by
  78.17 dB over their given time. The SFZ files give them the game's time for that fall. Attack
  uses the SoundFont fit described above.
- **LFO.** SFZ uses a sine, the game's default curve. SFZ also supports pan LFO.
- **Pan.** A curve maps CC10 to `(value - 64) / 64`. That's the SoundFont convention that the
  MIDI output uses.
- **Level.** SFZ region `volume` restores any gain lost when MIDI CC11 is scaled. Playback keeps
  the game's level.
- **Controllers.** CC7, CC10 and CC11 act at once, without sfizz's default 10 ms smoothing. A
  note whose track sets the volume or pan in the note's first tick starts at that level and pan.
- **Loops.** sfizz crossfades the loop end with data before the loop start over at least 1 ms.
  Samples include 4 ms of repeated loop data before the start. The crossfade is then inaudible.
- **Interpolation.** A region with AX's linear interpolation gets sfizz's linear interpolation
  (`sample_quality=1`). Other regions use sfizz's default, a cubic interpolation. That's the
  closest that sfizz has to AX's 4-tap filters. Above a rate of 1, a region also gets a low-pass
  for the highs that AX's filters lose beyond the cubic interpolation: `lpf_1p`, with a cutoff
  that tracks the key (`fil_keytrack=100`). The low-pass takes a filter that the track's filters
  leave free. A track that uses both its low-pass and its biquad leaves none while the biquad is
  on. Its regions then play without this low-pass.

Two sfizz 1.2.3 quirks affect playback:

- **Sample rate.** Loading a file configures its filters for 48 kHz. A later change of sample
  rate corrects them. At other rates, load files before setting the rate. `sfizz_render` and
  `sfizz_jack` set the rate first. `sfizz_render` uses 48 kHz unless given another rate.
- **Streaming.** During offline rendering, streamed samples can fall silent after their
  preloaded part. The files load samples in full with `hint_ram_based=1`. sfizz prints
  "Unsupported value for hint_stealing: 1" for this. The message is harmless.

## Differences from the game

cafesf2 reports approximations for each sequence. Some behavior cannot be represented exactly
in MIDI, SoundFont or SFZ:

- **Per-note pitch.** Game sweeps, portamento and legato key changes affect individual notes.
  MIDI pitch bend affects the whole channel. A note that sweeps beside another sounding note
  goes on another channel (see "MIDI").
- **Legato.** Tie/mono normally continue the existing note. Pitch bend gives the key changes,
  and CC11 the velocity changes. A note that legatos take above its velocity starts at the
  highest velocity they reach. CC11 lowers it until then. That works as long as the instrument
  plays the same region at that velocity. Past that, CC11 goes above 127 (see "Volume limits").
  Beside another note's release, such a note goes on another channel (see "MIDI"). CC11 would
  lower that release too. When no channel is left, the rising legato requires a new MIDI note. A
  changed envelope/LFO preset requires a new MIDI note too. The new note restarts the sample and
  envelope. The game would continue both. The game keeps the volume of the region that a tied
  note started on. When the new MIDI note plays a region with another volume, CC11 makes up the
  difference.
- **Detached notes.** Toggling tie/mono, muting with release or restarting a track detaches its
  sounding notes. They finish releasing with their old settings. In MIDI, they keep those
  settings on a channel of their own (see "MIDI"). When no channel is left, they stay on the
  track's channel and receive its later changes. Changes affecting a detached note above -40 dB
  are reported.
- **Combined pan.** nw::snd adds region pan, track initial pan and track pan before applying
  its pan law. Synths receive track pan separately through CC10 and combine it with zone pan
  using the synth's pan law. For example, a region at +0.38 on a hard-left track is 6.3 dB
  left in the game but 14 dB left in FluidSynth.
- **Pan curves and modes.** cafesf2 always uses the default curve and dual mode. Other curves
  and balance mode are reported. Balance mode pans stereo channels together.
- **LFOs.** Only a track's first LFO is converted. The converted LFO is a sine that starts at
  the beginning of its cycle. The other three LFOs, and other curves (triangle, saw, square and
  random) and phases, are reported. A note keeps the speed, range, type and delay that the track
  has at the end of the note-on's tick. The game gives the note those settings then. The game
  also updates them during playback. A note whose pitch LFO changes while it sounds plays on a
  MIDI channel of its own. On that channel, pitch bends follow the curve and phase of the game's
  LFO frame by frame. Only a random curve is still reported there. The random numbers of such a
  curve can differ from those in the game. A volume or pan LFO that changes during a note is
  reported. So is a pitch LFO that changes when no MIDI channel is free. Depth changes work
  through CC1.
- **Deep tremolo.** In the game, a volume LFO can lower a note as far as silence but can raise
  it by no more than 6 dB. The SoundFont and SFZ LFOs swing as far up as down. A volume LFO
  deeper than 6 dB is kept to a 6 dB swing and reported. Such a tremolo dips less than in the
  game.
- **SoundFont filters.** The two-pole low-pass closely matches the biquad low-pass. For the
  one-pole track filter, the two-pole low-pass passes slightly more just above cutoff and less
  in the top octave. The two-pole filter's slope is twice as steep. Cutoffs come from tables
  fitted by ntdlib's `tools/sequence_export/filter_tables.py`. With a table's cutoff, a sound
  whose power lies just above a low cutoff can play several decibels louder than in the game.
  Where a table's cutoff misses the game's level by more than 1 dB at a key that the sequence
  plays, the zone takes the cutoff at which FluidSynth passes as much of the zone's wave as the
  game does. High-pass and band-pass are omitted. Two active low-passes use the lower cutoff. A
  resampling low-pass (see "Resampling filters") counts as one of them. SFZ has all these
  filters.
- **Routing.** Surround pan, main send and effect sends B and C are omitted. Pan LFO is omitted
  from SoundFont but included in SFZ. Effect send A becomes the synth's reverb send. Each game
  sets up the effects on its aux buses. A send to a bus without an effect is lost. In amiibo
  Festival, the TV's bus A has an effect and bus B has none. Output to the Wii U GamePad isn't
  converted.
- **Volume limits.** Archive sound volume, track volume, second/main volume and region volume
  can reach 255. MIDI CC7 holds track volume up to 127. CC11 holds the rest, also up to 127. If
  necessary, all CC11 values are scaled equally. That keeps the balance between tracks and
  lowers the sequence's level. SoundFont also cannot amplify a sample: if any used region
  exceeds volume 127, every zone is attenuated equally to preserve region balance. cafesf2
  reports the reduction. SFZ restores the MIDI gain and keeps each region's original volume.
- **AX output.** Interpolation and output limiting in synths differ from those in AX. Region
  requests for linear or no interpolation are reported. The SFZ files follow linear
  interpolation, and SoundFont synths use their usual interpolation. A region with no
  interpolation plays its wave without rate conversion in the game. cafesf2 pitches such a
  region as usual.
- **Resampling filters.** AX converts each voice's wave to the 32 kHz mix through one of three
  sets of 4-tap filters. AX selects the set when the voice starts, by the voice's rate: the
  wave's sample rate times the pitch ratio, over 32 kHz. Above a rate of 1, the selected set
  cuts the highest frequencies. Above 4/3, the set cuts more of them. Regions with linear or no
  interpolation play without these filters. cafesf2 splits each zone and region at the keys
  where the rate passes 1 and 4/3. The upper parts get a low-pass whose cutoff tracks the key.
  The cutoffs are fitted for FluidSynth at 44.1 kHz and sfizz at 48 kHz, their default rates. A
  zone takes its filter by the key, without bend, sweep or LFO. A note that starts with its
  bend, sweep or LFO across a threshold therefore plays through its key's low-pass. A SoundFont
  preset whose filter changes during notes (CC102) gets no such low-pass. Its notes above a rate
  of 1 are reported. The SFZ files keep the low-pass.
- **Voice limits.** nw::snd has 96 voices unless a game sets fewer. Synths usually have more. By
  default, cafesf2 keeps the notes that the game drops for lack of a voice. With `--voices N`,
  cafesf2 drops them as its model of the game does. If a track then starts over 96 notes in one
  tick, cafesf2 keeps the first 96. The game keeps the first 96 too. Without `--voices`, the
  limit is 1024 notes a tick. After 96 legatos in a tick, the MIDI note continues with its
  existing preset and no gain above its starting level. Legatos that would otherwise require a
  new note are reported.

SoundFont output follows the specification and targets FluidSynth. Player differences remain:

- **Level.** FluidSynth runs the SoundFont low-pass filter on every note. That includes notes
  whose filter is open. The filter divides the level by the square root of its Q. FluidSynth
  converts the default resonance of 0 dB to a Q of 0.707. Every note therefore plays 1.5 dB
  louder than the SoundFont specifies. Multiplying FluidSynth's gain by 0.84 gives the specified
  level.
- **Pan.** Pans are mapped from nw::snd's square-root law to FluidSynth's sin/cos law.
  TinySoundFont uses a square-root law. That gives a slightly narrower result.
- **Envelopes.** Times follow the specification's 100 dB range. FluidSynth uses 96 dB. Decays
  and releases there fall 4% slower in decibels, and sustain levels are 4% shallower: -11.9 dB
  plays as -11.4 dB. FluidSynth also slows any release faster than 15.6 ms (-7200 timecents) to
  that time.
- **Tremolo.** In FluidSynth, a volume LFO can't raise a note above full level. On a note that
  sustains at full level, a tremolo only dips. In the game, a tremolo swings as far up as down.
- **Attenuation.** Like E-mu hardware, FluidSynth applies 0.4 of a zone's initial attenuation.
  The specification applies all of it. A region at volume 64 therefore plays 2.4 dB below one
  at 127 in FluidSynth, rather than 6 dB. Attenuation for regions above 127 is reduced the same
  way.
- **Low-pass level.** In FluidSynth, a note through the SoundFont's version of the biquad
  low-pass plays 0.85 dB below an unfiltered one. The game's filter lowers it by 0.5 dB.
- **Pitch.** FluidSynth rounds pitch down to whole cents. Fractional bends can be almost a cent
  flat. A long note's loop can then drift by up to 35 ms per minute against the game.
- **Key groups.** The game releases interrupted group notes at rate 126. They fade out within
  40 ms. FluidSynth uses -200 timecents for exclusive-class cuts, about 0.9 s over the full
  envelope range.
- **Modulators.** TinySoundFont doesn't support SoundFont modulators. CC1 LFO depth and CC102
  filter changes have no effect there.
- **Tempo changes.** FluidSynth rounds its position to the nearest MIDI tick at each change and
  restarts its clock there. This can advance playback by half a tick per change: 1/30 ms at
  120 BPM with 7680 ticks per quarter note. TinySoundFont rounds tempo-change times down to
  milliseconds. Playback then advances by about half a millisecond per change on average.
- **Note timing.** FluidSynth applies MIDI events at the start of each 64-sample render block.
  Notes can start up to 64 samples late, about 1.5 ms at 44.1 kHz.
- **Short notes.** FluidSynth holds every note for at least 10 ms (`synth.min-note-length`).
  Shorter notes therefore sound longer than in the game.

## Building

Requires C++20 and CMake 3.20 or later. cafesf2 supports macOS on Apple Silicon and Windows x64
with clang-cl. CI builds both. It runs the tests on Windows.

- **Windows:** install Visual Studio 2022 with "Desktop development with C++", including CMake.
  Add the "C++ Clang tools for Windows" component. Use Developer PowerShell or Developer Command
  Prompt.
- **macOS:** install Xcode command line tools (`xcode-select --install`) and CMake
  (`brew install cmake` or the installer from cmake.org).

cafesf2 builds on three libraries: seqlib, xsflib and ntdlib. The repository holds them as Git
submodules in `externals/`. Clone it with its submodules:

```sh
git clone --recursive <repository URL>
```

In a clone made without `--recursive`, run `git submodule update --init` first.

From the repository root:

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release
```

On Windows, the first command is `cmake -S . -B build -A x64 -T ClangCL`. It selects clang-cl
and 64-bit output.

The executable is `build/cafesf2`, or `build/Release/cafesf2.exe` with Visual Studio. Xcode and
other multi-configuration generators also use `build/Release`. The Windows build links the
runtime statically. The executable requires no runtime DLLs. CI uploads the Windows executable as an
artifact. macOS builds also produce `build/Cafesf2.app`. The app holds a copy of the program.
You can move the app elsewhere.

C++ formatting uses clang-format with the repository's `.clang-format`.

## Tests and tools

`ctest` checks that the program reports its version and rejects a variable that the game
doesn't have. It converts a small JAudio2 game that the test program writes, from the game's
start-up archive. The conversion's unit tests are in the seqlib and ntdlib libraries. They run
on synthetic files, without game data.

Other checks and tools:

- `-DCAFESF2_TEST_ARCHIVE=path/to/archive.bfsar` adds the conversion of every sequence in that
  archive. Independent readers check each output file.
- seqlib's `seqlib_check` program checks the structure of SoundFont and MIDI files. Configure
  with `-DSEQLIB_BUILD_TOOLS=ON`, then build the `seqlib_check` target.
- ntdlib's `tools/sequence_export/filter_tables.py cafe` derives the Wii U's SoundFont and SFZ
  filter tables in ntdlib's `src/sequence_export/filter_tables_cafe.cpp` from Animal Crossing:
  amiibo Festival's tables in ntdlib's `include/ntdlib/nwsnd/tables.h`. Requires Python 3 and
  numpy.

The archive check requires an archive from a game, supplied separately. cafesf2 doesn't
distribute game files. Normal conversion requires only the input: a `.bfsar` archive and any
group files beside it, or a JAudio game's sound files.

## Licence

cafesf2 is under the [MIT](LICENSE) licence. The libraries it builds on, seqlib, xsflib and
ntdlib, are under the MIT licence too. `THIRD-PARTY-NOTICES.md` gives their licence texts.
Game data is not covered by this licence.

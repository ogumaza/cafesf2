# Third-party notices

cafesf2 is under the MIT licence (`LICENSE`). The libraries it builds on are under the MIT licence too.

| Component | Licence | Use in cafesf2 |
|---|---|---|
| seqlib, Copyright (c) 2026 the seqlib authors | MIT (`externals/seqlib/LICENSE`) | In `externals/seqlib`, a submodule, compiled into cafesf2: the MIDI, SoundFont 2 and SFZ files and their samples. |
| xsflib, Copyright (c) 2026 the xsflib authors | MIT (`externals/xsflib/LICENSE`) | In `externals/xsflib`, a submodule, compiled into cafesf2: bounds-checked reads in either byte order, SHA-1, file input and output with UTF-8 paths, the printable form of names, UTF-8 arguments and output on the console, the Windows version resource and the droplet script of the macOS app. |
| ntdlib, Copyright (c) 2026 the ntdlib authors | MIT (`externals/ntdlib/LICENSE`) | In `externals/ntdlib`, a submodule, compiled into cafesf2: the sound archive and wave readers, the readers of the JAudio sound data in both generations, the sequence players of nw::snd and JAudio, the starts of the sequences of known games, the conversion of instruments and filters, and the options, help and messages of the command line. |

Binary distributions must include the MIT notices of cafesf2, seqlib, xsflib and ntdlib. Shipping `LICENSE` and this file
does that. The licence texts follow.

## Console data

Nothing in cafesf2 licenses game data. The sound archives that cafesf2 reads belong to their copyright holders.

`include/ntdlib/nwsnd/tables.h` in ntdlib contains the numeric tables of the NintendoWare sound libraries: powers of two,
decibel levels, envelope attack multipliers, pan curves, low-pass cutoffs, biquad filter coefficients and sines. The
models use the exact values to reproduce the output of the consoles. The files `src/sequence_export/filter_tables_*.cpp` in
ntdlib contain the filters of each console, fitted to SoundFont 2 and SFZ settings.

## Licence texts

### seqlib

```
MIT License

Copyright (c) 2026 the seqlib authors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### xsflib

```
MIT License

Copyright (c) 2026 the xsflib authors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### ntdlib

```
MIT License

Copyright (c) 2026 the ntdlib authors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

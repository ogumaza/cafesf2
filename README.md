# cafesf2

cafesf2 converts Nintendo Wii U sound archives (`.bfsar`) to MIDI, SoundFont 2 and SFZ.
Each sequence gets a MIDI file and a SoundFont that contains its instruments. The SFZ files add
the filters that SoundFont can't reproduce (see [SFZ files](#sfz-files)). cafesf2 also converts
the music of Wii U games that play their sound through JAudio (see [JAudio games](#jaudio-games)).

Drop archives onto `cafesf2.exe` on Windows or `Cafesf2.app` on macOS. The output goes in a
folder beside each archive. An input can also be the start-up archive of a JAudio game (`.baa`
or `.aaf`). From a terminal:

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

cafesf2 has profiles for the sound archives of six games, all from the USA: Animal Crossing: amiibo
Festival, Mario Party 10, Nintendo Land, Super Mario 3D World, New Super Mario Bros. U and
Wii Party U (Rev 1). It finds the profile of an archive by the archive contents. In those
archives, a sequence that waits for the game's variables starts with the values that the game
code gives it, where a reading of the code found them. Other values that change what the
sequence plays convert as arrangements. Each arrangement is one more set of files after the
sequence. Its name is the sequence name with each variable and its value added, such as
`SQ_SE_MG0B05_TANK_L_AXEL_15_1`. Where the game code computes a value as it plays, such as the
speed of a lift, or where no reading found it, the sequence starts with that variable at -1. Its
arrangements then contain the values that change what it plays. The values of a profile replace
those of `--var`.

cafesf2 also has a profile for Kirby and the Rainbow Curse (USA). The stage select of that game
plays one piece, `BGM_SEQ_MENU_INTERACTIVE`, with one to three of its tracks. The game picks them
by the view: the area view, the list of stages of a world, or the boss stage of a world. The
files of the piece play the track of the area view alone. Each of the 14 other states converts
as an arrangement named after the state, such as `BGM_SEQ_MENU_INTERACTIVE world 3 boss`. A track
that a state turns down plays at CC11 0 from the start. Its notes stay in the MIDI file.

cafesf2 also has a profile for Game & Wario (USA). In the game Gamer, the music follows the BPM of
the game in play, and a session starts at 120, 140 or 200 BPM. The files of each of Gamer's 32
pieces play at 120 BPM. Two arrangements, such as `BGM_NI_GAME_FISH1 140 BPM`, play each piece
as a session at 140 or 200 BPM starts it: at 7/6 of the tempo and 2 semitones higher, or at 5/3
of the tempo and 8 semitones higher. Some sessions then change BPM from game to game. Those later
BPMs aren't converted.

By default, cafesf2 keeps every note. In the game, a note is dropped when every voice is in use.
nw::snd has 96 voices unless the game code sets another count. `--voices N` drops the notes that
the cafesf2 model of the game drops with N voices. Which notes the console drops isn't known
exactly. The result only approximates the game.

Files use the sequence names, with changes for Windows. Forbidden characters become `_`. So does
a dot or a space at the end of a name. A reserved device name such as `CON` gets an added
`_`. If names differ only in ASCII letter case, the later sequences get the sound index as a
suffix. That avoids collisions on Windows and macOS. The same applies to a sequence named
`samples`, a name that would otherwise collide with the SFZ sample folder. The default output
folder is `<archive name>_cafesf2`, and the same rules apply to its name. Use `--output` to
choose another location.

Wii U games play their sequences with Nintendo's sound library, nw::snd. cafesf2 plays them as
nw::snd does. The model of the timing, the voices and the commands comes from the executable of
Animal Crossing: amiibo Festival.

## JAudio games

Some Wii U games play their sound through JAudio, the sound library in Nintendo EAD's JSystem,
and not through nw::snd. Twilight Princess HD and The Wind Waker HD are two of them. cafesf2
converts their music and sound effects. Give it the start-up archive of the game, with the other
sound files beside it as in the `content` folder of the title. The files go in
`<archive name>_cafesf2` beside the archive.

JAudio comes in two generations. They lay out their files differently:

- **JAudio2** (Twilight Princess HD): the start-up archive (`.baa`) contains the sound table, the
  names of the sounds, the instrument banks and the wave systems. The sequence archive (an RARC
  archive in a `Seqs` folder beside it) contains the sequences. The wave archives (`.aw` files in
  a `Waves` folder) contain the samples.
- **The first JAudio** (The Wind Waker HD): the start-up archive (`.aaf`) contains the sound
  table, the instrument banks and the wave systems. A sequence table (`.bdt`) in a `Seqs` folder
  beside it contains the sequences. The wave archives (`.aw` files in a `Banks` folder) contain
  the samples. A name file (`.asn`) beside the start-up archive contains the names of the sounds.

The sequences come from the sound table of the game, in table order. In JAudio2, the sound
effects are sequences too, and they convert with the music. The first JAudio plays the sound
effects of a game in one sequence that the game code steers. cafesf2 converts each effect as
that sequence plays it when the game starts the effect (see
[The first JAudio's sound effects](#the-first-jaudios-sound-effects)). Files take the names that
the game gives the sounds. A sound without a name becomes `SOUND_<section>_<group>_<index>`. A
sequence that no sound plays comes after the sequences of the sounds. In a sequence archive, such
a sequence keeps the name of its file. In a sequence table, it becomes `SEQUENCE_<number>`. The
sound effects of the first JAudio come last, in the order of the sound table. An effect that the
name file doesn't name becomes `se<category>-<index>`, such as `se4-0123`.

`--loops`, `--seed` and `--only` work as they do for a sound archive. The random state of the game
starts at 0 unless `--seed` sets another. The game steers some sequences through ports. Like the
game, cafesf2 starts each port at 0. JAudio sequences have no variables, and `--var` changes
nothing in them. By default, cafesf2 keeps every note. `--voices N` drops the notes that JAudio
would drop with N voices. On the GameCube, JAudio has 64. The result only approximates the game.

The sound layer of The Wind Waker HD mutes some tracks of 14 pieces as it starts them. The game
state picks the tracks, such as the boat's sail or an enemy near Link. Some of the game's IDs
start a piece in another arrangement. cafesf2 converts each piece as the game starts it, in the
state that the sound layer starts in: the sail down, Link off the boat, his sword sheathed and no
enemy near. It also converts each other arrangement. Each one is named after the piece and the
game ID or the state, for example `JA_BGM_KAMI_TOWER_80000125` or `JA_BGM_SEA_SAILING`. The
battle music and three other pieces fade in over two seconds. The game fades them in too. While
some pieces play, the game code writes a status to their port 9, such as the phase of a boss fight.
Such a piece converts once more for each value that the game writes and that changes what the
piece plays, for example `JA_BGM_P_GANON_1_PORT9_2`. The battle music also converts with 1 on
port 11, as when Link hits an enemy. In the game, a track that comes in during a fight fades in
over 5 or 10 frames. In the files, it plays from the start.

JAudio times its music for the GameCube rate of about 32,028 Hz. The Wii U plays it at
32,000 Hz. On the Wii U, the music therefore plays 0.089% slower and 1.5 cents lower than in the
files.

At start-up, Twilight Princess HD sets a volume for each category of sound effects. Category 1,
Link's voice, keeps the volume of the sound table. The others play at 120/127 of it. cafesf2
does the same. During a conversation and in the pause menu, the game plays most categories
quieter for a while. cafesf2 leaves that out.

As Twilight Princess HD starts five pieces of music, it mutes some of their tracks. The day music
of Hyrule Field has two arrangements. One plays while Link walks, and the other while he rides.
cafesf2 converts the piece twice and leaves out the notes of the muted tracks.
`Z2BGM_FIELD_LINK_DAY` contains the walking arrangement and `Z2BGM_FIELD_LINK_DAY_riding` the
riding one. `Z2BGM_TOAL_VILLEGE`, `Z2BGM_HORSE_BATTLE`, `Z2BGM_BOSS_SNOWWOMAN_1` and
`Z2BGM_VS_GANON_02` leave out the tracks that the game mutes as they start.

Later, the scenes, fights and other code of the game change the tracks of 17 pieces. Some states
play a track at part of its volume, such as the tracks of Ordon Village at 0.65. The file of a
piece plays the state that the game starts the piece in, such as the state of the first room of
a dungeon. cafesf2 converts each later state as one more file of the piece. The name of that file
adds the status that the game gives the piece, such as `Z2BGM_TOAL_VILLEGE_status1`. The states
of the battle music follow the fight: `_near`, `_searched`, `_near_searched` and `_port9_1`.

The day music of Hyrule Field also follows the field code: what Link does, the enemies near him
and the time of day. cafesf2 converts each way through the piece as one more file in each
arrangement. `_status12` plays the sections of a long walk or ride. `_status4` to `_status7` play
the sections of a fight by their four entries. `_status10` and `_status11` play the two endings
of a day. `_status8` and `_status9` play the starts that the field code makes once no music
plays. `_riding_status3` plays the section in which Link stands on his horse.

The game moves the tracks to a new state gradually. cafesf2 plays each state from the start and
leaves out the states in between. The states come from runs of the game code on the GameCube,
the Wii and the Wii U. The HD code gives `Z2BGM_DUNGEON_LV9_02` other rooms and a start that
leaves out some tracks. cafesf2 converts that piece with the HD states.

Twilight Princess HD sets up an echo at start-up, like the GameCube and Wii releases. The HD echo
repeats every 152 ms, a little sooner than the 160 ms of the Wii echo. A file that sends to the
echo ends when the echo would fall silent. The echo itself isn't in the file.

Programs 0xf0 and up play the built-in oscillators of the DSP. The square wave, the sawtooth, the
pulse of 25% duty, the triangle and the sine become looping samples of one cycle, one for each
program. Each preset of such a program has one zone, for every key and velocity. The zone
contains the envelope of the oscillators: full volume until the release, then a fall to silence
over about 17 ms. The DSP changes its noise and its ramp as it plays, and no sample can
reproduce them.

MIDI and SoundFont can't express everything that JAudio does. The fitted envelopes can't follow
an oscillator table that rises again or loops. Random instrument effects play at their base
values. Tremolos and pitch envelopes are left out. So are the notes on the noise, the ramp and
the other oscillators of the DSP that no sample can reproduce. The track filters of sound effects
and the filters that change from note to note are left out too. A music track plays its other
filters through two zones of each region: one unfiltered and one low-passed. A vibrato deeper
than a semitone is kept to a semitone. In the sequences of the first JAudio, a track that shares
the voices of its parent gets separate voices. The oscillators that a track sets and the tempos
of child tracks are left out there. cafesf2 reports each of these for each sequence.

### The first JAudio's sound effects

The first JAudio plays all the sound effects of a game in one sequence: the first sequence of the
sound table. The sound library starts it when the game starts. In The Wind Waker HD, the sequence
gives each category of effects four slots. A slot is a track that waits for an effect. To start
an effect, the library writes four ports of the slot: a wait for the distance, information about
the map, the number of the effect and a request. The track of the slot then plays the effect.
cafesf2 plays each effect that way, in the first slot of its category. The library writes the
ports after the first tick of the effects sequence. The first note of an effect therefore comes a
tick after the start of its file. Only the slot track and the tracks below it count for the loops
and the end. The effect ends when its slot is free again.

The volume of an effect is its volume in the sound table times the volume of its category. The
Wind Waker HD sets a volume for each of the eight categories of sound effects in the table.
Category 1 keeps the volume of the sound table. Categories 3, 6 and 7 play at 0.82 of it. The
others play at 0.88 of it. cafesf2 recognizes the start-up archives of the USA and Japanese
releases and does the same. An effect also plays at the pitch that the table gives it. The MIDI
file sets that pitch with a pitch bend.

A track can send its notes through the automatic mixer of the DSP. That mixer sets their gains
from their volume and pan. cafesf2 plays those notes at half the level of the others. That level
approximates the mix of the game.

The game code passes values with some effects. Information about the map, such as the material of
the ground, is one of them. cafesf2 passes 0 for each. Some effects, such as footsteps, then play
nothing. Some other effects play nothing in any case. Most of those end at once, and their files
contain no notes.

A game keeps some effects going by starting them again and again. cafesf2 plays each effect from
one start: to its end, or with loop markers where it loops.

## Downloads

The Releases page has cafesf2 for 64-bit Windows. On macOS on Apple Silicon, build it from
source (see [Building](#building)).

## Drag and drop

- **Windows:** drop one or more `.bfsar`, `.baa` or `.aaf` files onto `cafesf2.exe`. The console
  shows the results and waits for Enter before it closes.
- **macOS:** drop archives onto `Cafesf2.app`, or open it to choose files. The app shows the
  results and can open the output folders. The app wraps the command-line program because
  Finder can't drop files onto a bare executable. The [build](#building) includes the app.

cafesf2 reports an unreadable archive and goes on with the others. If two inputs would use the
same output folder, cafesf2 skips the second to protect the files of the first. This can happen
when names differ only in extension, or only in case on a filesystem that ignores case. Windows
and macOS filesystems usually do.

## The conversion

### Sequence playback

FSEQ is bytecode, with variables, conditions, random arguments, calls, jumps, loops and tracks
that start other tracks. cafesf2 executes the bytecode and records the resulting notes and
controllers. The conversion follows the nw::snd player of the Wii U, including:

- All commands and their variable, comparison, random and `if` forms, among them the additions of
  the Wii U: calls and loops nested ten deep, main and second volumes that ramp like track
  volume, a third effect send, and four LFOs per track with curves, phases and periods.
- Note lengths, damper, key groups, tie/mono, `notewait off` and waits for sounding notes to
  finish. Note ends are calculated frame by frame. nw::snd works in 3 ms sound frames. AX, the
  audio system of the Wii U, starts playing a voice two frames after note-on. A one-shot wave
  ends when AX has played all of it, at the pitch that comes from the note sweep, the track bend
  and the pitch LFOs. nw::snd computes that pitch again each frame. Envelopes start at -90.4 dB
  and first advance at the end of a frame. Notes released or stopped before then are inaudible
  and produce no MIDI note.
- Tick scheduling in 1/65536 ms, in single precision. The rounding in the game occasionally
  places a tick in an adjacent sound frame.
- The random generator of the game, including its once-per-frame advance. Random choices
  therefore depend on timing.
- Main-loop detection and MIDI loop markers. The main loop belongs to the lowest-numbered track
  that can still advance. A pass qualifies only if earlier tracks make no contribution. Tracks
  that wait forever, on held notes or for a negative number of ticks, don't block this choice.
  A pass that changes nothing, such as one that only polls or counts a variable, qualifies only
  while no other track changes the sound or plays on. Setting a variable that another track
  reads counts as a change. A tie or mono legato on a note below hearing (see below) changes
  nothing that can be heard. A track that has sounded nothing and changed nothing for 10 seconds
  doesn't count as playing on. A sequence whose tracks all reach their end has no main loop.
  Backward jumps to unvisited commands and passes without waits, such as random retries or
  variable polling, aren't loops. Inside a subroutine, a jump back isn't a loop when its pass
  played no note that can be heard and ran a conditional jump, call, return or end that could
  leave the loop. A tie or mono legato on a note below hearing (see below) plays nothing that
  can be heard. A held note that sounds until the game stops it keeps such a loop. A loop that a track
  runs inside a pass of another loop, such as a countdown of waits between notes, is an inner
  loop. The passes of an inner loop don't qualify until the track has played no pass of another
  loop for 10 seconds. A main loop in which no note starts isn't a loop either once every
  earlier note has ended or is fading out. The sequence then ends with its last sound.

  The markers go on the first pass that a player looping on them repeats as the game plays the
  next one: the same notes at the same times, with the same settings and tempo. The first pass
  often differs. Setup before the loop can land in its first tick. The game can keep a value from
  pass to pass. Later passes may only tie again a note that the first pass strikes. If no pass so
  far repeats that way but the next pass does, the performance plays that pass too. Otherwise the
  markers stay on the first pass. They also stay there in a loop that draws random numbers. At
  the start of the loop, the MIDI file sends each controller, pitch, preset and tempo that the
  game sets again there when the loop ends with another value. Without those events, a player
  that loops on the markers would keep the value from the end of the loop. `--loops N` plays the
  marked pass N times. The passes before it play once, ahead of the loops. A file whose markers
  are on the second pass therefore contains N + 1 passes of the music of the loop.

Short loops, such as repeating sound effects, run until the MIDI file is at least 10 seconds
long. Sounds held without end stop 10 seconds after their last change. A note of 9,600 ticks or
more that its track waits out counts as held. Games use such notes, minutes long at a typical
tempo, for sounds that they stop themselves. The same limit covers a main loop longer than
10 seconds that ends with more than 10 seconds of no change and sounds only endless notes held
with tie or mono. The next pass in the game usually continues those notes. A sequence that
finishes naturally ends with its last sound, including release. A note counts as below hearing
once its envelope has decayed 40 dB. A note that plays until its track releases it, such as a
tie, ends where its envelope has decayed 60 dB. If its sustain level stays above that, the note
ends at 40 dB down. In the game, the channel goes on in silence. Performances stop after
15 minutes. A performance that has been silent for more than 10 seconds by then also ends with
its last sound. Tempo or timebase 0 stops sequence timing while notes keep playing. Those notes
finish normally, or stop at the same 10-second limit if they're held without end.

### MIDI

Each sequence track gets a MIDI track and a channel of its own. Channel 10, reserved for drums in
General MIDI, is allocated last. SoundFont synths use bank 128 for that channel. cafesf2 copies
the presets of that channel to bank 128.

MIDI output keeps tempo changes and uses at least 7680 ticks per quarter note, with an integer
number of MIDI ticks per sequence tick. The game processes ticks in 3 ms sound frames. AX plays
their changes two frames later. cafesf2 places events at the start of the frame that contains
each tick, rounded to the nearest MIDI tick. This shifts the whole sequence two frames earlier
and keeps the spacing of events. At 120 BPM, the rounding error is at most 1/30 ms. No moved
event crosses a loop marker. A loop keeps its contents. A sweep or a portamento with a
portamento time moves on in every frame, between ticks too. cafesf2 writes a pitch bend at the
start of each of those frames. After its track closes, the sweep goes on while the release of
the note sounds. Sweeps also go on after the last tick of the sequence, at the last tempo.

nw::snd applies track settings to notes at the end of a frame. A note starts with the settings
that its tick ends with. MIDI controller changes are written before the notes of the same tick.

| Game setting | MIDI output |
|---|---|
| Track volume, up to 127 | CC7 |
| Track volume above 127 × second volume × main volume × sound volume, plus legato velocity changes | CC11. Like the nw::snd volumes, CC7 and CC11 both act squared. Values are scaled for sequences that exceed the MIDI range (see [Differences from the game](#differences-from-the-game)). |
| Track pan | CC10, mapped to the SoundFont pan law |
| Pitch bend, sweep, portamento, legato key changes and a pitch LFO that changes during a note | Pitch bend, with the RPN 0 range set per channel |
| Depth of the first LFO | CC1 = depth / 2, mapped to vibrato or tremolo in the SoundFont |
| Effect send A | CC91 (reverb) |
| Immediate track silence | CC120 |
| Tempo ratio and pitch that the game sets for the sound | Tempo events scaled by the ratio. The pitch becomes RPN 2 (coarse tuning) and RPN 1 (fine tuning) on every channel at tick 0. |
| Filters that change during sounding notes | CC102, mapped to the SoundFont cutoff by a modulator |
| Track low-pass (0xd8), for SFZ | CC103 = 64 minus the argument; values <= 6 mean off |
| Biquad filter (0xb4/0xb5), for SFZ | CC104 = filter value; 0 means off |
| Instrument and track envelope, LFO, initial pan and filter settings | Bank select (CC0) and program change |

A MIDI channel can play only one note per key. Repeated strikes that overlap, common in drums,
get extra channels called layers, with matching controllers, up to the 16-channel limit.

A MIDI channel has one set of controllers for all its notes. In the game, each note keeps its
settings. While notes that the game plays differently from the later notes of the track still
sound, the track moves on to a free channel when there is one. That covers release tails that the
track has detached. Those keep their settings. The same goes for notes still sounding when a new
note starts that sweeps, starts at a raised velocity or requires a level of its own. Each channel
gets a MIDI track named after the sequence track, in the same way as a layer.

### SoundFont

Each distinct combination of bank instrument and track settings becomes a preset. Instrument
regions become zones with:

- Key and velocity ranges resolved by the first-match rules of nw::snd.
- The sample, root key, tuning, linear volume and pan of the region.
- Envelope parameters after track overrides. nw::snd advances an envelope 3 ms in each 3 ms
  sound frame. The nw::snd attack is exponential in decibels. The SoundFont attack is linear, and
  fits the game attack best at 0.55 of its time. A note that the game releases during its attack
  gets a longer attack, in a preset of its own. That attack reaches the level of the game at the
  release. The release then starts from there. SoundFont decays and releases fall 100 dB in
  101.6 s at the slowest. The slowest decays and releases of the game take up to six times as
  long. Such a decay gets a longer hold, up to the SoundFont limit of 18 s. The hold is half the
  time by which the game decay reaches its sustain level later. At the end of the hold, the note
  is then as far above the level of the game as it is below that level when its decay reaches the
  sustain level. Such a release plays at the slowest SoundFont release time. The SFZ files keep
  the times of the game.
- Key groups mapped to exclusive classes.
- A low-pass for the resampling filters of AX, on the keys whose notes play their wave at a rate
  above the AX mixing rate of 32 kHz. Such a region splits into zones where the rate of its notes
  passes 1 and 4/3. See "Resampling filters" under "Differences from the game".
- Loops repeated as needed to meet the SoundFont padding rules: 8 samples before, at least 32
  within, and 32 after. This preserves playback.
- Continued playback for regions that ignore note-off. Their MIDI notes end when the game would
  release them, with release settings that also preserve decay after a repeated strike.

cafesf2 reads Wii U FSAR archives and their FSEQ, FBNK, FWAR and FWAV files. Supported wave
encodings are PCM8, PCM16 and DSP-ADPCM. Banks and wave archives stored only in FGRP groups are
also supported. Groups can have wave archives of their own. Copies of a bank may then reference
different files. cafesf2 uses the copy in the first group. The copies play the same waves.

Some games keep groups outside the archive, in files that the game loads itself. cafesf2 looks
for each such group as `<group name>.bfgrp` beside the archive, or in a `userManagementFiles`
folder beside it. Some games keep these group files in SARC archives under other names. While a
group is still missing, cafesf2 also reads the group files in the `.sarc` files beside the
archive. A folder can also contain the group files of another archive. cafesf2 takes a group file
from a SARC only if its files are of the kinds that the archive gives them. When cafesf2 finds
none of the groups, a warning says so.

cafesf2 refuses archives from other consoles: `.bcsar` from the 3DS, `.brsar` from the Wii and
`.bfsar` from the Switch. Switch archives are little-endian, and Wii U archives are big-endian.

## SFZ files

SoundFont has one two-pole low-pass filter per voice. That filter can't reproduce the high-pass
or band-pass presets of the game, and only approximates the one-pole track low-pass. SFZ supports
all of these filters. cafesf2 also writes these SFZ files:

- `sfz/<sequence>/Track <n>.sfz`: instruments for MIDI `Track <n>` and its layers, for each
  active sequence track.
- `sfz/samples/`: 16-bit WAV samples shared by all sequences in the archive. Stereo channels
  are separate files, panned left and right.

SFZ players ignore MIDI channels, and each file contains one instrument. Load the MIDI file in a
DAW and assign an SFZ player, such as sfizz, to each track with its matching file. The player
tested is sfizz 1.2.

Presets use program change, plus bank select (CC0) when a sequence has more than 128 presets.
Regions match the SoundFont, with these differences:

- **Filters.** The sfizz `lpf_1p` filter matches the track low-pass. `lpf_2p`, `hpf_2p` and
  `bpf_2p` match the biquad presets coefficient for coefficient at the 32 kHz rate of AX. When
  both filters are active, they run in series. CC103/CC104 and SFZ curves map track values to
  cutoff, resonance and gain, including changes during notes. An inactive low-pass stays at the
  highest cutoff of sfizz, 20 kHz. There it lowers 10 kHz by about 0.5 dB and 16 kHz by 1 dB.
  SFZ has no biquad bypass like the value 0 or the absent preset of the game. Instead, CC104
  selects filtered or unfiltered regions at note-on, and that choice lasts for the note. As in
  the game, the value at the end of the sound frame of the note-on decides. Each track uses its
  first biquad type. Later types are omitted and reported.
- **Envelopes.** As in the game, decays and releases fall linearly in dB. The documented curve
  falls by a factor of e^-8 over the specified time. That's 69.49 dB. The SFZ files give each
  decay and release the time that the game takes for that fall. Attack uses the SoundFont fit
  described above.
- **LFO.** SFZ uses a sine, the default curve of the game. SFZ also supports pan LFO.
- **Pan.** A curve maps CC10 to `(value - 64) / 64`. That's the SoundFont convention that the
  MIDI output uses.
- **Level.** The SFZ region `volume` restores any gain lost when MIDI CC11 is scaled. Playback
  keeps the level of the game.
- **Controllers.** CC7, CC10 and CC11 act at once, without the default 10 ms smoothing of sfizz.
  A note whose track sets the volume or pan in the first tick of the note starts at that level
  and pan.
- **Loops.** sfizz crossfades the loop end with data before the loop start over at least 1 ms.
  Samples include 4 ms of repeated loop data before the start. The crossfade is then
  inaudible.
- **Key groups.** A note of a key group ends the other notes of the group with the fade that the
  game uses at release rate 126 (`off_mode=time` and `off_time`). The exclusive class of a
  SoundFont can't set that fade.
- **Interpolation.** A region with linear interpolation in AX gets linear interpolation in sfizz
  (`sample_quality=1`). Other regions use the sfizz default, a cubic interpolation. That's the
  closest that sfizz has to the 4-tap filters of AX. Above a rate of 1, a region also gets a
  low-pass for the highs that the AX filters lose beyond the cubic interpolation: `lpf_1p`, with
  a cutoff that tracks the key (`fil_keytrack=100`). The low-pass uses a filter that the track
  filters leave free. A track that uses both its low-pass and its biquad leaves none while the
  biquad is on. Its regions then play without this low-pass.

Six quirks of sfizz 1.2.3 affect playback:

- **Decays and releases.** sfizz ends them sooner than the documented curve. Over the specified
  time, sfizz falls by a factor of e^-9: 78.17 dB. Where the game has fallen 40 dB, sfizz has
  fallen 45 dB.
- **Sample rate.** Loading a file configures its filters for 48 kHz. A later change of sample
  rate corrects them. At other rates, load files before setting the rate. `sfizz_render` and
  `sfizz_jack` set the rate first. `sfizz_render` uses 48 kHz unless given another rate.
- **Streaming.** During offline rendering, streamed samples can fall silent after their
  preloaded part. The files load samples in full with `hint_ram_based=1`. sfizz prints
  "Unsupported value for hint_stealing: 1" for this. The message is harmless.
- **Mono samples.** sfizz pans a mono sample by the sin/cos law and then raises both sides by
  3 dB. Every sample of the files is mono. Every region therefore plays 3.0 dB above the level of
  the game, at every pan. The balance of the mix stays the same. Lowering the sfizz volume by
  3 dB gives the level of the game.
- **Repeated keys.** A key group in sfizz never cuts a note of the same key as the new note. The
  documented `off_by` cuts it. A key of a group struck again therefore leaves the earlier note
  playing in sfizz. The game fades that note out at release rate 126.
- **Short decays and releases.** sfizz makes each decay and release last at least 6 ms. The
  documentation sets no such floor. A shorter time plays over 6 ms.

The opcodes follow their documentation at sfzformat.com, with these exceptions:

- **Long decays and releases.** A decay or release can last longer than the documented limit of
  100 s, because the envelope of the game falls that slowly. sfizz 1.2 plays the time as written.
- **Long attacks.** A note that the game releases during its attack gets a longer attack (see
  "SoundFont" above). That attack can last up to 101.6 s. The documented limit is 100 s.
  sfizz 1.2 plays the time as written.
- **Loud regions.** The `volume` of a region can exceed the documented limit of 6 dB. sfizz 1.2
  accepts volumes up to 48 dB.
- **Low resonances.** The biquad presets of the game have resonances down to -46 dB. The
  documented range starts at 0 dB. The curve of each biquad track reaches resonances below 0 dB.
  sfizz 1.2 plays those resonances.
- **A wide resonance amount.** Biquad type 5 has steps down to -46 dB. A track of that type
  moves its resonance in units of 50 dB. The documentation gives the amount a range of -40 to
  40 dB.
- **High cutoffs.** A region pitched far up can get a low-pass cutoff above half the output
  sample rate. The documentation gives half the rate as the limit.

## Differences from the game

cafesf2 reports approximations for each sequence. Some of what the game does can't be represented
exactly in MIDI, SoundFont or SFZ:

- **Per-note pitch.** Sweeps, portamento and legato key changes in the game affect individual
  notes. MIDI pitch bend affects the whole channel. A note that sweeps beside another sounding
  note goes on another channel (see "MIDI").
- **Legato.** Tie/mono normally continue the existing note. Pitch bend sets the key changes, and
  CC11 the velocity changes. A note that legatos take above its velocity starts at the highest
  velocity they reach. CC11 lowers it until then. That works as long as the instrument plays the
  same region at that velocity. Past that, CC11 goes above 127 (see "Volume limits"). Beside the
  release of another note, such a note goes on another channel (see "MIDI"). CC11 would
  otherwise lower that release too. When no channel is left, the rising legato requires a new
  MIDI note. A changed envelope/LFO preset requires a new MIDI note too. The new note restarts
  the sample and envelope, where the game would continue both. The game keeps the volume of the
  region that a tied note started on. When the new MIDI note plays a region with another volume,
  CC11 makes up the difference.
- **Detached notes.** Toggling tie/mono, muting with release or restarting a track detaches its
  sounding notes. They finish releasing with their old settings. In MIDI, they keep those
  settings on a channel of their own (see "MIDI"). When no channel is left, they stay on the
  channel of the track and receive its later changes. Changes that affect a detached note above
  -40 dB are reported.
- **Combined pan.** nw::snd adds region pan, track initial pan and track pan before it applies
  its pan law. Synths receive track pan separately through CC10 and combine it with zone pan by
  their pan law. For example, a region at +0.38 on a hard-left track is 6.3 dB left in the game
  but 14 dB left in FluidSynth.
- **Pan curves and modes.** cafesf2 always uses the default curve and dual mode. Other curves
  and balance mode are reported. Balance mode pans stereo channels together.
- **LFOs.** Only the first LFO of a track is converted. The converted LFO is a sine that starts
  at the beginning of its cycle. The other three LFOs, and other curves (triangle, saw, square
  and random) and phases, are reported. A note keeps the speed, range, type and delay that the
  track has at the end of the tick of the note-on. The game gives the note those settings then.
  The game also updates them during playback. A note whose pitch LFO changes while it sounds
  plays on a MIDI channel of its own. On that channel, pitch bends follow the curve and phase of
  the LFO in the game frame by frame. Only a random curve is still reported there. The random
  numbers of such a curve can differ from those in the game. A volume or pan LFO that changes
  during a note is reported. So is a pitch LFO that changes when no MIDI channel is free. Depth
  changes work through CC1.
- **Deep tremolo.** In the game, a volume LFO can lower a note as far as silence but can raise
  it by no more than 6 dB. The SoundFont and SFZ LFOs swing as far up as down. A volume LFO
  deeper than 6 dB is kept to a 6 dB swing and reported. Such a tremolo dips less than in the
  game.
- **SoundFont filters.** The two-pole low-pass closely matches the biquad low-pass. For the
  one-pole track filter, the two-pole low-pass passes slightly more just above cutoff and less
  in the top octave. The slope of the two-pole filter is twice as steep. Cutoffs come from
  tables fitted by `tools/sequence_export/filter_tables.py` in ntdlib. With the cutoff from a
  table, a sound whose power is just above a low cutoff can play several decibels louder than in
  the game. Where that cutoff misses the level of the game by more than 1 dB at a key that the
  sequence plays, the zone uses the cutoff at which FluidSynth passes as much of the zone wave as
  the game does. High-pass and band-pass are omitted. Two active low-passes use the lower cutoff.
  A resampling low-pass (see "Resampling filters") counts as one of them. SFZ has all these
  filters.
- **Routing.** Surround pan, main send and effect sends B and C are omitted. Pan LFO is omitted
  from SoundFont but included in SFZ. Effect send A becomes the reverb send of the synth. Each
  game sets up the effects on its aux buses. A send to a bus without an effect is lost. In amiibo
  Festival, bus A of the TV output has an effect and bus B has none. Output to the Wii U GamePad
  isn't converted.
- **Volume limits.** Archive sound volume, track volume, second/main volume and region volume
  can reach 255. MIDI CC7 sets the track volume up to 127. CC11 sets the rest, also up to 127.
  If necessary, all CC11 values are scaled equally. That keeps the balance between tracks and
  lowers the level of the sequence. SoundFont also can't amplify a sample. If any region in use
  exceeds volume 127, every zone is attenuated equally to preserve the balance of the regions.
  cafesf2 reports the reduction. SFZ restores the MIDI gain and keeps the original volume of
  each region.
- **AX output.** Interpolation and output limiting in synths differ from those in AX. Region
  requests for linear or no interpolation are reported. The SFZ files follow linear
  interpolation, and SoundFont synths use their usual interpolation. A region with no
  interpolation plays its wave without rate conversion in the game. cafesf2 pitches such a
  region as usual.
- **Resampling filters.** AX converts the wave of each voice to the 32 kHz mix through one of
  three sets of 4-tap filters. AX selects the set when the voice starts, by the rate of the
  voice: the sample rate of the wave times the pitch ratio, over 32 kHz. Above a rate of 1, the
  selected set cuts the highest frequencies. Above 4/3, the set cuts more of them. Regions with
  linear or no interpolation play without these filters. cafesf2 splits each zone and region at
  the keys where the rate passes 1 and 4/3. The upper parts get a low-pass whose cutoff tracks
  the key. The cutoffs are fitted for FluidSynth at 44.1 kHz and sfizz at 48 kHz, their default
  rates. A zone gets its filter from the key alone, without bend, sweep or LFO. A note that
  starts with its bend, sweep or LFO across a threshold therefore plays through the low-pass of
  its key. A SoundFont preset whose filter changes during notes (CC102) gets no such low-pass.
  Its notes above a rate of 1 are reported. The SFZ files keep the low-pass.
- **Rate limit.** AX plays a voice at a rate of 8 at most: 8 samples of its wave for each sample
  of the 32 kHz mix. A note whose pitch asks for a higher rate plays at 8 in the game. A note
  whose key starts past the limit and whose pitch stays still gets zones and regions that play
  at 8 too. Every key past the limit then plays at one pitch, with a scale tuning of 0 in the
  SoundFont and `pitch_keytrack=0` in the SFZ files. Those keys get no resampling low-pass. A
  note that a bend, a sweep or an LFO moves plays every key at its pitch. In the game, such a
  note stays at the limit for as long as its pitch would pass it. In the files, it plays above
  the limit then.
- **Voice limits.** nw::snd has 96 voices unless a game sets fewer. Synths usually have more. By
  default, cafesf2 keeps the notes that the game drops for lack of a voice. With `--voices N`,
  cafesf2 drops them as its model of the game does. If a track then starts over 96 notes in one
  tick, cafesf2 keeps the first 96. The game keeps the first 96 too. Without `--voices`, the
  limit is 1024 notes a tick. After 96 legatos in a tick, the MIDI note continues with its
  existing preset and no gain above its starting level. Legatos that would otherwise require a new note are reported.

SoundFont output follows the specification, apart from the attenuation of the zones and the
resonance of the biquad low-pass. Differences between players remain:

- **Level.** FluidSynth runs the SoundFont low-pass filter on every note, including notes whose
  filter is open. The filter divides the level by the square root of its Q. FluidSynth converts
  the default resonance of 0 dB to a Q of 0.707. Every note therefore plays 1.5 dB louder than
  the SoundFont specifies. Multiplying the FluidSynth gain by 0.84 gives the specified level.
- **Pan.** Pans are mapped from the square-root law of nw::snd to the sin/cos law of FluidSynth.
  TinySoundFont uses a square-root law. That gives a slightly narrower result.
- **Envelopes.** Times follow the 100 dB range of the specification. FluidSynth uses 96 dB. Its
  decays and releases fall 4% slower in decibels, and its sustain levels are 4% shallower:
  -11.9 dB plays as -11.4 dB. FluidSynth also slows any release faster than 15.6 ms
  (-7200 timecents) to that time.
- **Tremolo.** In FluidSynth before 2.4.0, a volume LFO can't raise a note above full level. On
  a note that sustains at full level, a tremolo only dips. In the game, a tremolo swings as far
  up as down.
- **Attenuation.** FluidSynth, BASSMIDI and the E-mu chips apply 0.4 of the initial attenuation
  of a zone. The specification calls for all of it. cafesf2 therefore writes 2.5 times the
  attenuation of a zone in the generator, up to the generator limit of 1440 cB. Those players
  play that limit as 57.6 dB. A modulator without a source contains any attenuation beyond
  57.6 dB. FluidSynth 2.4.5 and later apply that modulator. BASSMIDI and earlier versions of
  FluidSynth ignore it. A player that follows the specification plays every attenuated zone too
  quietly. TinySoundFont plays a quarter of the attenuation. Before 27 January 2025,
  TinySoundFont played 2.5 times the attenuation.
- **Low-pass level.** In FluidSynth, a note through the SoundFont version of the biquad low-pass
  plays 0.85 dB below an unfiltered one. The filter of the game lowers it by 0.5 dB.
- **Resonance.** FluidSynth, BASSMIDI and the E-mu chips play a resonance of r dB as r - 3 dB at
  the cutoff. The specification calls for r dB. cafesf2 writes the resonance of the biquad
  low-pass for those players. A player that follows the specification plays 3 dB more at the
  cutoff.
- **Pitch.** FluidSynth rounds pitch down to whole cents. Fractional bends can be almost a cent
  flat. The loop of a long note can then drift by up to 35 ms per minute against the game.
- **Key groups.** The game releases interrupted group notes at rate 126. They fade out within
  40 ms. FluidSynth 2.4.3 and later cut such notes with -2000 timecents, about 0.3 s over the
  full envelope range. Earlier versions use -200 timecents, about 0.9 s. BASSMIDI ends them in
  about 4 ms.
- **Modulators.** BASSMIDI 2.4.16 ignores the CC1 and CC102 modulators and the modulator without
  a source (see "Attenuation"). CC1 then gives the default vibrato of the specification in every
  zone: 50 cents at CC1 127. A tremolo becomes that vibrato. Filters that change during a note
  stay open. TinySoundFont doesn't support SoundFont modulators. CC1 LFO depth and CC102 filter
  changes have no effect there.
- **Tempo changes.** FluidSynth rounds its position to the nearest MIDI tick at each change and
  restarts its clock there. This can advance playback by half a tick per change: 1/30 ms at
  120 BPM with 7680 ticks per quarter note. TinySoundFont rounds the times of tempo changes down
  to milliseconds. Playback then advances by about half a millisecond per change on average.
- **Note timing.** FluidSynth applies MIDI events at the start of each 64-sample render block.
  Notes can start up to 64 samples late, about 1.5 ms at 44.1 kHz.
- **Short notes.** FluidSynth holds every note for at least 10 ms (`synth.min-note-length`).
  Shorter notes therefore sound longer than in the game.

## Building

cafesf2 requires C++20 and CMake 3.20 or later. It supports macOS on Apple Silicon and Windows
x64 with clang-cl. CI builds both and runs the tests on Windows.

- **Windows:** install Visual Studio 2022 with "Desktop development with C++", including CMake.
  Add the "C++ Clang tools for Windows" component. Use Developer PowerShell or Developer Command
  Prompt.
- **macOS:** install the Xcode command line tools (`xcode-select --install`) and CMake
  (`brew install cmake` or the installer from cmake.org).

cafesf2 builds on three libraries: seqlib, xsflib and ntdlib. The repository includes them as Git
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

On Windows, the first command is `cmake -S . -B build -A x64 -T ClangCL`. The extra options
select clang-cl and 64-bit output.

The executable is `build/cafesf2`, or `build/Release/cafesf2.exe` with Visual Studio. Xcode and
other multi-configuration generators also use `build/Release`. The Windows build links the
runtime statically. The executable requires no runtime DLLs. CI uploads the Windows
executable as an artifact. macOS builds also produce `build/Cafesf2.app`. The app contains a copy
of the program, and you can move it elsewhere.

The C++ code is formatted with clang-format and the `.clang-format` file of the repository.

## Tests and tools

`ctest` checks that the program reports its version and refuses a variable that the game doesn't
have. A unit test checks how a profile sets the variables, the tracks, the tempo ratio and the
pitch of a sequence. ctest also converts a small JAudio2 game that the test program writes, from
the start-up archive of the game. The unit tests of the conversion are in the seqlib and ntdlib
libraries. They run on synthetic files, without game data.

Other checks and tools:

- `-DCAFESF2_TEST_ARCHIVE=path/to/archive.bfsar` adds the conversion of every sequence in that
  archive. Independent readers check each output file.
- The `seqlib_check` program of seqlib checks the structure of SoundFont and MIDI files.
  Configure with `-DSEQLIB_BUILD_TOOLS=ON`, then build the `seqlib_check` target.
- `tools/sequence_export/filter_tables.py cafe` in ntdlib derives the SoundFont and SFZ filter
  tables of the Wii U (`src/sequence_export/filter_tables_cafe.cpp` in ntdlib). It derives them
  from the tables of Animal Crossing: amiibo Festival in `include/ntdlib/nwsnd/tables.h`. The
  script requires Python 3 and numpy.

The archive check requires an archive from a game, supplied separately. cafesf2 doesn't
distribute game files. Normal conversion requires only the input: a `.bfsar` archive and any
group files beside it, or the sound files of a JAudio game.

## Licence

cafesf2 is under the [MIT](LICENSE) licence. The libraries it builds on, seqlib, xsflib and
ntdlib, are under the MIT licence too. `THIRD-PARTY-NOTICES.md` contains their licence texts.
Game data isn't covered by this licence.

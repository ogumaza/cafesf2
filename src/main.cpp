// SPDX-License-Identifier: MIT

// cafesf2: convert Wii U sound archive sequences (.bfsar) to MIDI, SoundFont and SFZ.
//
//     cafesf2 [options] <archive.bfsar> [<archive.bfsar> ...]
//
// Output goes beside each archive. cafesf2 also converts the music of Wii U games that play it with JAudio, from the
// game's start-up archive (.baa or .aaf) with the game's other sound files beside it. ntdlib's converter of JAudio
// games converts them. Windows accepts files dropped on the executable; macOS uses Cafesf2.app.

#include "ntdlib/sar/formats.h"
#include "ntdlib/sequence_export/archive_converter.h"
#include "ntdlib/sequence_export/consoles.h"
#include "ntdlib/sequence_export/jaudio/archive_games.h"

int main(int argc, char** argv)
{
    ntdlib::sequence_export::ArchiveConverter<ntdlib::sar::SoundArchive> converter;
    converter.program = "cafesf2";
    converter.version_line = "cafesf2 " CAFESF2_VERSION;
    converter.description =
        "cafesf2: convert Wii U sound archive sequences (.bfsar) to MIDI, SoundFont 2 and\n"
        "SFZ. It also converts the music of Wii U games that use JAudio: give the game's\n"
        "start-up archive (.baa or .aaf), with the game's other sound files beside it as\n"
        "in the title's content folder.\n";
    converter.extension = ".bfsar";
    converter.settings = ntdlib::sequence_export::CafeSettings();
    converter.group_files = true;

    // The first JAudio plays a game's sound effects in one sequence. jaudio1_effects converts each effect as that
    // sequence plays it, after the game's other sequences.
    return ntdlib::sequence_export::jaudio::RunArchiveConverterWithGames(argc, argv, converter,
                                                                         {.jaudio1_effects = true});
}

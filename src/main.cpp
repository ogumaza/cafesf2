// SPDX-License-Identifier: MIT

// cafesf2: convert Wii U sound archive sequences (.bfsar) to MIDI, SoundFont and SFZ.
//
//     cafesf2 [options] <archive.bfsar> [<archive.bfsar> ...]
//
// Output goes beside each archive. Windows accepts files dropped on the executable; macOS uses Cafesf2.app.

#include "ntdlib/sar/formats.h"
#include "ntdlib/sequence_export/archive_converter.h"
#include "ntdlib/sequence_export/consoles.h"

int main(int argc, char** argv)
{
    ntdlib::sequence_export::ArchiveConverter<ntdlib::sar::SoundArchive> converter;
    converter.program = "cafesf2";
    converter.version_line = "cafesf2 " CAFESF2_VERSION;
    converter.description =
        "cafesf2: convert Wii U sound archive sequences (.bfsar) to MIDI, SoundFont 2 and\n"
        "SFZ.\n";
    converter.extension = ".bfsar";
    converter.settings = ntdlib::sequence_export::CafeSettings();
    converter.group_files = true;

    return ntdlib::sequence_export::RunArchiveConverter(argc, argv, converter);
}

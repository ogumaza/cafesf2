// SPDX-License-Identifier: MIT

// Converts every sequence of a sound archive (.bfsar) with the Wii U's settings and checks each conversion. With
// --jaudio, writes a small JAudio game for the test of cafesf2's JAudio inputs instead (ntdlib's WriteGameFiles). The
// unit tests of the code in ntdlib and seqlib are in those libraries.
//
//     cafesf2_test <archive.bfsar>
//     cafesf2_test --jaudio <folder>

#include <string>

#include "ntdlib/sequence_export/consoles.h"
#include "ntdlib/testing/jaudio_files.h"
#include "ntdlib/testing/sequence_export_check.h"

int main(int argc, char** argv)
{
    if (argc == 3 && std::string(argv[1]) == "--jaudio")
    {
        ntdlib::testing::jaudio_files::WriteGameFiles(argv[2]);
        return 0;
    }

    return ntdlib::testing::RunArchiveCheck(argc, argv, ntdlib::sequence_export::CafeSettings(), "cafesf2", ".bfsar");
}

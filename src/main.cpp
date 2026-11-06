// SPDX-License-Identifier: MIT

// cafesf2: convert Wii U sound archive sequences (.bfsar) to MIDI, SoundFont and SFZ.
//
//     cafesf2 [options] <archive.bfsar> [<archive.bfsar> ...]
//
// Output goes beside each archive. cafesf2 also converts the music of Wii U games that play it with JAudio, from the
// game's start-up archive (.baa or .aaf) with the game's other sound files beside it. ntdlib's converter of JAudio
// games converts them. Windows accepts files dropped on the executable; macOS uses Cafesf2.app.

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "ntdlib/sar/formats.h"
#include "ntdlib/sequence_export/archive_converter.h"
#include "ntdlib/sequence_export/consoles.h"
#include "ntdlib/sequence_export/convert.h"
#include "ntdlib/sequence_export/frame.h"
#include "ntdlib/sequence_export/jaudio/game_input.h"
#include "ntdlib/sequence_export/performance.h"

#include "profiles.h"

namespace frame = ntdlib::sequence_export;
namespace jaudio = ntdlib::sequence_export::jaudio;

using ntdlib::sar::SoundArchive;

int main(int argc, char** argv)
{
    frame::ArchiveConverter<SoundArchive> converter;
    converter.program = "cafesf2";
    converter.version_line = "cafesf2 " CAFESF2_VERSION;
    converter.description =
        "cafesf2: convert Wii U sound archive sequences (.bfsar) to MIDI, SoundFont 2 and\n"
        "SFZ. It also converts the music of Wii U games that use JAudio: give the game's\n"
        "start-up archive (.baa or .aaf), with the game's other sound files beside it as\n"
        "in the title's content folder.\n";
    converter.extension = ".bfsar";
    converter.settings = frame::CafeSettings();
    converter.group_files = true;

    // An archive's profile starts the sequences that wait for the game's variables with the values that the game's
    // code gives them, and a layered piece with the tracks that the game's code leaves playing (profiles.h). The
    // profile comes from the archive's bytes, before the frame reads the archive.
    cafesf2::Profile profile = cafesf2::Profile::kNone;
    frame::Converter<frame::PerformOptions, SoundArchive, frame::Conversion> archives =
        frame::MakeArchiveConverter(converter);
    const auto open = archives.open;
    archives.open = [open, &profile](const std::filesystem::path& input, const std::string& name)
    {
        const std::optional<std::vector<uint8_t>> bytes = frame::ReadFile(input);
        profile = bytes ? cafesf2::ProfileOf(*bytes) : cafesf2::Profile::kNone;
        return open(input, name);
    };

    // A sequence's arrangements follow the sequence (cafesf2::ArrangementsOf). Arrangement k of sound s gets the index
    // (k + 1) * n + s. n is the archive's sound count.
    const auto sequences = archives.sequences;
    archives.sequences = [sequences, &profile](const SoundArchive& archive)
    {
        const std::size_t sounds = archive.Sounds().size();
        std::vector<frame::Sequence> listed;
        for (const frame::Sequence& sequence : sequences(archive))
        {
            listed.push_back(sequence);
            const std::span<const cafesf2::Arrangement> arrangements = cafesf2::ArrangementsOf(profile, sequence.name);
            for (std::size_t k = 0; k < arrangements.size(); k++)
            {
                listed.push_back({.index = (k + 1) * sounds + sequence.index,
                                  .name = sequence.name + std::string(arrangements[k].suffix)});
            }
        }

        return listed;
    };
    const auto convert = archives.convert;
    archives.convert =
        [convert, &profile](SoundArchive& archive, std::size_t index, const frame::PerformOptions& perform)
    {
        const std::size_t sounds = archive.Sounds().size();
        const std::string& name = archive.Sounds()[index % sounds].name;
        frame::PerformOptions options = perform;
        if (index < sounds)
        {
            cafesf2::SetUpSound(profile, name, options);
        }
        else
        {
            cafesf2::SetUpArrangement(cafesf2::ArrangementsOf(profile, name)[index / sounds - 1], options);
        }

        return convert(archive, index % sounds, options);
    };

    // The first JAudio plays a game's sound effects in one sequence. jaudio1_effects converts each effect as that
    // sequence plays it, after the game's other sequences. The games run on the Wii U (wii_u). Twilight Princess HD
    // shares its start-up archive with the Wii's release. With wii_u, HD's rules hold the echo that HD's code sets up.
    // As jaudio::RunArchiveConverterWithGames does, a JAudio game takes the program's options. Of those beyond the
    // frame's, only --voices applies to its sequences.
    const jaudio::GameConverter games = jaudio::MakeGameConverter(
        archives.program, archives.version_line, jaudio::ArchiveProgramOptions(archives.option), archives.usage,
        {.jaudio1_effects = true, .wii_u = true});
    const auto run = [&archives, &games](const std::vector<std::string>& args)
    {
        return frame::Run(args, archives, jaudio::ArchiveProgramGames(args, games));
    };

    return frame::RunMain(argc, argv, run);
}

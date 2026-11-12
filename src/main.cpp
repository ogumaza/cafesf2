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
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ntdlib/cafe/snd/sequence_starts.h"
#include "ntdlib/sar/formats.h"
#include "ntdlib/sequence_export/archive_converter.h"
#include "ntdlib/sequence_export/arrangements.h"
#include "ntdlib/sequence_export/consoles.h"
#include "ntdlib/sequence_export/convert.h"
#include "ntdlib/sequence_export/frame.h"
#include "ntdlib/sequence_export/jaudio/game_input.h"
#include "ntdlib/sequence_export/performance.h"
#include "xsflib/sha1.h"

#include "profiles.h"

namespace frame = ntdlib::sequence_export;
namespace jaudio = ntdlib::sequence_export::jaudio;

using ntdlib::cafe::snd::SequenceArrangement;
using ntdlib::sar::SoundArchive;

namespace
{

// Reads the sound archive at `input`, named `name`, with `settings`, and the group files that the game keeps beside it.
// Sets `archive_sha1` to the SHA-1 of the archive file. The SHA-1 picks the starts of the sequences (profiles.h).
// Returns nothing if the file can't be read. The frame then reports the failure.
std::optional<SoundArchive> ReadArchive(const std::filesystem::path& input, const std::string& name,
                                        const frame::Settings& settings, std::string& archive_sha1)
{
    archive_sha1.clear();
    std::optional<std::vector<uint8_t>> bytes = frame::ReadFile(input);
    if (!bytes)
    {
        return std::nullopt;
    }

    archive_sha1 = xsflib::Sha1::Hex(xsflib::Sha1::Of(*bytes));
    SoundArchive archive = frame::LoadArchive<SoundArchive>(std::move(*bytes), name, settings);
    frame::AddGroupFiles(archive, input, name);

    return archive;
}

// The arrangements of each sound of `archive` for the frame. An arrangement sets the variables and the tracks that the
// game sets.
frame::ArrangementsOf ArrangementsOf(std::string_view archive_sha1, const SoundArchive& archive)
{
    return [sha1 = std::string(archive_sha1), &archive](std::size_t sound)
    {
        std::vector<frame::Arrangement> arrangements;
        for (const SequenceArrangement& arrangement :
             ntdlib::cafe::snd::SequenceArrangements(sha1, archive.Sounds()[sound].name))
        {
            arrangements.push_back({std::string(arrangement.suffix), [arrangement](frame::PerformOptions& options)
                                    { cafesf2::SetUpArrangement(arrangement, options); }});
        }

        return arrangements;
    };
}

// Converts entry `index` of the frame's list of `archive` with `perform`, `settings` and `program`, as the game starts
// it. A sequence uses the start in ntdlib's list. An arrangement uses the variables and the tracks that the arrangement
// sets.
frame::Conversion ConvertEntry(std::string_view archive_sha1, const SoundArchive& archive, std::size_t index,
                               const frame::PerformOptions& perform, const frame::Settings& settings,
                               const std::string& program)
{
    const frame::ArrangedSound at = frame::ArrangedSoundAt(index, archive.Sounds().size());
    frame::PerformOptions options = perform;
    if (!at.arrangement)
    {
        cafesf2::SetUpStart(ntdlib::cafe::snd::StartOfSequence(archive_sha1, archive.Sounds()[at.sound].name), options);
    }

    frame::SetUpArrangement(at, ArrangementsOf(archive_sha1, archive), options);
    return frame::ConvertSequence(archive, static_cast<uint32_t>(at.sound), options, settings, program);
}

// Fills in `converter`: the help and the Wii U settings.
void SetUpConverter(frame::ArchiveConverter<SoundArchive>& converter)
{
    converter.program = "cafesf2";
    converter.version_line = "cafesf2 " CAFESF2_VERSION;
    converter.description =
        "cafesf2: convert Wii U sound archive sequences (.bfsar) to MIDI, SoundFont 2 and\n"
        "SFZ. It also converts the music of Wii U games that use JAudio: give the game's\n"
        "start-up archive (.baa or .aaf), with the game's other sound files beside it as\n"
        "in the title's content folder.\n";
    converter.extension = ".bfsar";
    converter.settings = frame::CafeSettings();
}

// The frame converter of `converter`, with the cafesf2 hooks. The hooks keep the SHA-1 of the open archive in
// `archive_sha1`. `converter` and `archive_sha1` have to outlive the result.
frame::Converter<frame::PerformOptions, SoundArchive, frame::Conversion> MakeConverter(
    const frame::ArchiveConverter<SoundArchive>& converter, std::string& archive_sha1)
{
    frame::Converter<frame::PerformOptions, SoundArchive, frame::Conversion> archives =
        frame::MakeArchiveConverter(converter);
    archives.open = [&converter, &archive_sha1](const std::filesystem::path& input, const std::string& name)
    {
        return ReadArchive(input, name, converter.settings, archive_sha1);
    };
    archives.sequences = [&archive_sha1](const SoundArchive& archive)
    {
        return frame::WithArrangements(frame::ArchiveSequences(archive), archive.Sounds().size(),
                                       ArrangementsOf(archive_sha1, archive));
    };
    archives.convert =
        [&converter, &archive_sha1](SoundArchive& archive, std::size_t index, const frame::PerformOptions& perform)
    {
        return ConvertEntry(archive_sha1, archive, index, perform, converter.settings, converter.program);
    };

    return archives;
}

} // namespace

int main(int argc, char** argv)
{
    frame::ArchiveConverter<SoundArchive> converter;
    SetUpConverter(converter);
    std::string archive_sha1;
    const frame::Converter<frame::PerformOptions, SoundArchive, frame::Conversion> archives =
        MakeConverter(converter, archive_sha1);

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

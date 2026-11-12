// SPDX-License-Identifier: MIT

// Tests the starts of the sequences (src/profiles.h): how the start of a listed sequence and its arrangements set the
// variables and the tracks of the performance. The cafe_sequence_starts test in ntdlib checks the values in the lists.

#include "profiles.h"

#include <cstdint>
#include <span>
#include <string_view>

#include "ntdlib/cafe/snd/sequence_starts.h"
#include "ntdlib/sequence_export/performance.h"
#include "xsflib/testing/check.h"

using ntdlib::cafe::snd::SequenceArrangement;
using ntdlib::cafe::snd::SequenceArrangements;
using ntdlib::cafe::snd::StartOfSequence;
using ntdlib::sequence_export::PerformOptions;
using xsflib::testing::Check;

namespace
{

// The SHA-1s of the sound archives of the checks.
constexpr std::string_view kAmiiboFestival = "5784acb45299369f8388246f0ba4311610c709bc";
constexpr std::string_view kAmiiboFestivalArc1 = "724d9942fa0ea684839a3de269878e11b206a135";
constexpr std::string_view kMarioParty10 = "00bdc6c4610830576a44b7198d25894fe87f06b9";
constexpr std::string_view kNewSuperMarioBrosU = "e0da277f9eed3c9fbd6dcbce09230811932d14dc";
constexpr std::string_view kRainbowCurse = "813a15124d13b1978e7e3cb49f6648b9cb50d705";

void CheckStarts()
{
    PerformOptions humming;
    cafesf2::SetUpStart(StartOfSequence(kAmiiboFestivalArc1, "SQ_SE_ACT_HUMMING"), humming);
    Check(humming.variables.empty() && humming.track_variables.size() == 1 && humming.track_variables[0].track == 0 &&
              humming.track_variables[0].index == 15 && humming.track_variables[0].value == 1,
          "amiibo Festival's humming starts with track 0's variable 15 at 1");

    PerformOptions habitat;
    cafesf2::SetUpStart(StartOfSequence(kAmiiboFestival, "SQ_SE_HAB_ENV"), habitat);
    Check(habitat.variables.size() == 1 && habitat.variables.at(18) == 0 && habitat.track_variables.empty() &&
              habitat.tracks == 0xffff,
          "the habitat card's ambience starts with global 2 at 0");

    PerformOptions unlisted;
    cafesf2::SetUpStart(StartOfSequence(kAmiiboFestival, "SQ_BGM_LOUNGE"), unlisted);
    cafesf2::SetUpStart(StartOfSequence("", "SQ_SE_HAB_ENV"), unlisted);
    Check(unlisted.variables.empty() && unlisted.track_variables.empty() && unlisted.tracks == 0xffff,
          "an unlisted sequence and an unknown archive set nothing");
}

void CheckArrangements()
{
    const std::span<const SequenceArrangement> touch = SequenceArrangements(kMarioParty10, "SQ_SE_MG0K04_KOP_TCH_DRC");
    PerformOptions both;
    cafesf2::SetUpArrangement(touch[3], both);
    Check(both.variables.size() == 2 && both.variables.at(24) == 0 && both.variables.at(25) == 0,
          "an arrangement sets each of its variables");

    PerformOptions over;
    over.variables[0] = 7;
    cafesf2::SetUpArrangement(SequenceArrangements(kNewSuperMarioBrosU, "SE_OBJ_RC_LIFTLINE_MOVE")[2], over);
    Check(over.variables.size() == 1 && over.variables.at(0) == 200, "an arrangement's value replaces --var's");
}

// Kirby and the Rainbow Curse's stage select starts its piece with track 7 alone. The tracks of a state play. A track
// that the options already leave out stays out.
void CheckTrackStates()
{
    PerformOptions start;
    cafesf2::SetUpStart(StartOfSequence(kRainbowCurse, "BGM_SEQ_MENU_INTERACTIVE"), start);
    Check(start.tracks == 0x0080 && start.variables.empty(), "the stage select starts with track 7 alone");

    const std::span<const SequenceArrangement> states = SequenceArrangements(kRainbowCurse, "BGM_SEQ_MENU_INTERACTIVE");
    PerformOptions boss;
    boss.tracks = 0xfffe;
    cafesf2::SetUpArrangement(states[9], boss);
    Check(boss.tracks == 0x1104, "world 3's boss stage plays tracks 2, 8 and 12");
    cafesf2::SetUpArrangement(states[7], boss);
    Check(boss.tracks == 0x0100, "a track that the options leave out stays out");
}

} // namespace

int main()
{
    CheckStarts();
    CheckArrangements();
    CheckTrackStates();

    return xsflib::testing::Summary("cafesf2 profiles");
}

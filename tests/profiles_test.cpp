// SPDX-License-Identifier: MIT

// Tests the games' profiles (src/profiles.h): how a profiled sequence starts, its arrangements and their names, the
// states of a piece whose tracks the game turns down, and the archives that get no profile.

#include "profiles.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

#include "ntdlib/sequence_export/performance.h"
#include "xsflib/testing/check.h"

using cafesf2::Arrangement;
using cafesf2::Profile;
using ntdlib::sequence_export::PerformOptions;
using xsflib::testing::Check;

namespace
{

// The name that an arrangement's variables give it: "_" before each variable's number and value, and "_" between two
// variables.
std::string NameOf(const cafesf2::Variables& variables)
{
    std::string name;
    for (const cafesf2::Variable& variable : variables)
    {
        if (variable.index == cafesf2::Variable::kNone)
        {
            continue;
        }

        name += (name.empty() ? "_" : "__") + std::to_string(variable.index) + "_" + std::to_string(variable.value);
    }

    return name;
}

void CheckStarts()
{
    PerformOptions humming;
    cafesf2::SetUpSound(Profile::kAmiiboFestivalArc1, "SQ_SE_ACT_HUMMING", humming);
    Check(humming.variables.empty() && humming.track_variables.size() == 1 && humming.track_variables[0].track == 0 &&
              humming.track_variables[0].index == 15 && humming.track_variables[0].value == 1,
          "amiibo Festival's humming starts with track 0's variable 15 at 1");

    PerformOptions habitat;
    cafesf2::SetUpSound(Profile::kAmiiboFestival, "SQ_SE_HAB_ENV", habitat);
    Check(habitat.variables.size() == 1 && habitat.variables.at(18) == 0 && habitat.track_variables.empty(),
          "the habitat card's ambience starts with global 2 at 0");

    PerformOptions lift;
    cafesf2::SetUpSound(Profile::kMarioParty10, "SQ_SE_BD003_VCL_LOP", lift);
    Check(lift.variables.size() == 1 && lift.variables.at(29) == 0, "a board's vehicle starts with global 13 at 0");

    PerformOptions trap;
    cafesf2::SetUpSound(Profile::kMarioParty10, "SQ_SE_MG0307_TRP", trap);
    Check(trap.variables.size() == 1 && trap.variables.at(15) == 0,
          "minigame MG0307's trap starts with variable 15 at 0");

    PerformOptions ghost;
    cafesf2::SetUpSound(Profile::kNintendoLand, "SE_LIG_OBAKE_LIFE_DOWN", ghost);
    Check(ghost.variables.size() == 1 && ghost.variables.at(4) == 0,
          "the ghost's falling life starts with variable 4 at 0");

    PerformOptions train;
    cafesf2::SetUpSound(Profile::kNintendoLand, "SE_TTL_TRAIN_MOVE_STEAM", train);
    Check(train.variables.size() == 1 && train.variables.at(0) == -9, "the plaza's train starts at level -9");

    PerformOptions damage;
    cafesf2::SetUpSound(Profile::kWiiPartyU, "SEQ_SE_PLY_FUTTOBI_DAMAGE", damage);
    Check(damage.variables.size() == 1 && damage.variables.at(15) == 0,
          "a Mii's damage in Wii Party U starts with variable 15 at 0");

    PerformOptions step;
    cafesf2::SetUpSound(Profile::kWiiPartyU, "SEQ_SE_PLY_FOOT_WALK_DRC", step);
    Check(step.variables.size() == 1 && step.variables.at(11) == 0,
          "a Mii's footstep in Wii Party U starts with the footstep at 0");

    PerformOptions theme;
    cafesf2::SetUpSound(Profile::kWiiPartyU, "SEQ_BGM_NPG006_THEMA", theme);
    Check(theme.variables.empty(), "Wii Party U's theme starts with its level unset");

    PerformOptions unlisted;
    cafesf2::SetUpSound(Profile::kAmiiboFestival, "SQ_BGM_LOUNGE", unlisted);
    cafesf2::SetUpSound(Profile::kNone, "SQ_SE_HAB_ENV", unlisted);
    cafesf2::SetUpSound(Profile::kAmiiboFestivalArc1, "SQ_SE_HAB_ENV", unlisted);
    Check(unlisted.variables.empty() && unlisted.track_variables.empty(),
          "an unlisted sequence, an archive without a profile and the other archive of the game set nothing");
}

void CheckArrangements()
{
    const std::span<const Arrangement> touch =
        cafesf2::ArrangementsOf(Profile::kMarioParty10, "SQ_SE_MG0K04_KOP_TCH_DRC");
    Check(touch.size() == 5 && touch[3].suffix == "_24_0__25_0", "Bowser's touch has five arrangements");

    PerformOptions both;
    cafesf2::SetVariables(touch[3].variables, both);
    Check(both.variables.size() == 2 && both.variables.at(24) == 0 && both.variables.at(25) == 0,
          "an arrangement sets each of its variables");

    PerformOptions over;
    over.variables[0] = 7;
    cafesf2::SetVariables(cafesf2::ArrangementsOf(Profile::kNewSuperMarioBrosU, "SE_OBJ_RC_LIFTLINE_MOVE")[2].variables,
                          over);
    Check(over.variables.size() == 1 && over.variables.at(0) == 200, "an arrangement's value replaces --var's");

    Check(cafesf2::ArrangementsOf(Profile::kAmiiboFestival, "SQ_SE_BOARD_RLT_MAN_RLT").empty(),
          "a sequence that starts with its only value has no arrangement");
    Check(cafesf2::ArrangementsOf(Profile::kMarioParty10, "SQ_SE_MG0206_JUMP").size() == 2,
          "minigame MG0206's jump has an arrangement for each other value that the game sets");
    Check(cafesf2::ArrangementsOf(Profile::kNintendoLand, "SE_TTL_TRAIN_MOVE").size() == 2,
          "the plaza's train has an arrangement for each faster level");
    Check(cafesf2::ArrangementsOf(Profile::kSuperMario3dWorld, "SeEvRunnerSqueak").size() == 3,
          "the runner's squeak has three arrangements");
    Check(cafesf2::ArrangementsOf(Profile::kWiiPartyU, "SEQ_SE_PLY_LANDING").size() == 12,
          "a Mii's landing in Wii Party U has an arrangement for each of the 12 surfaces");
    Check(cafesf2::ArrangementsOf(Profile::kWiiPartyU, "SEQ_BGM_NPG006_THEMA").size() == 3,
          "Wii Party U's theme has an arrangement for each of its three higher levels");
    Check(cafesf2::ArrangementsOf(Profile::kWiiPartyU, "SEQ_SE_NMG425_TUBE_SLIDE_1P").size() == 3,
          "a slide in Wii Party U has arrangements at the bottom, the middle and the top of its range");
    Check(cafesf2::ArrangementsOf(Profile::kNone, "SeEvRunnerSqueak").empty(),
          "an archive without a profile has no arrangement");
}

// Every arrangement of the listed sequences is named after its variables, and no two of a sequence share a name.
void CheckNames()
{
    struct Listed
    {
        Profile profile;
        std::string_view sound;
    };

    constexpr std::array<Listed, 16> kListed = {{
        {Profile::kAmiiboFestival, "SQ_SE_ESCAPE_MFA_FLY_WING_LP"},
        {Profile::kAmiiboFestival, "SQ_SE_BOARD_LND_ONE_R_01_5"},
        {Profile::kMarioParty10, "SQ_SE_MG0K04_KOP_TCH_DRC"},
        {Profile::kMarioParty10, "SQ_SE_MG0K06_BAL_ROL_DRC"},
        {Profile::kMarioParty10, "SQ_SE_MG0206_JUMP"},
        {Profile::kNintendoLand, "SE_PKM_PLAYER_FOOTSTEP"},
        {Profile::kNintendoLand, "SE_LIG_OBAKE_LIFE_DOWN"},
        {Profile::kNintendoLand, "SE_TTL_TRAIN_MOVE"},
        {Profile::kSuperMario3dWorld, "SePvSpinAttack"},
        {Profile::kNewSuperMarioBrosU, "SE_SYS_MAP_TOUCH_SCROLL_LV"},
        {Profile::kNewSuperMarioBrosU, "SE_OBJ_RC_FENCE_ROLL"},
        {Profile::kWiiPartyU, "SEQ_SE_PLY_DAMAGE_BOUND"},
        {Profile::kWiiPartyU, "SEQ_SE_NPG009_BALL_HIT_BALL"},
        {Profile::kWiiPartyU, "SEQ_SE_PLY_FOOT_RUN"},
        {Profile::kWiiPartyU, "SEQ_BGM_NPG006_THEMA"},
        {Profile::kWiiPartyU, "SEQ_SE_NMG431_TARU_HIT_KI_RM"},
    }};
    for (const Listed& listed : kListed)
    {
        const std::span<const Arrangement> arrangements = cafesf2::ArrangementsOf(listed.profile, listed.sound);
        bool named = !arrangements.empty();
        for (std::size_t i = 0; i < arrangements.size(); i++)
        {
            named = named && arrangements[i].suffix == NameOf(arrangements[i].variables);
            for (std::size_t j = 0; j < i; j++)
            {
                named = named && arrangements[i].suffix != arrangements[j].suffix;
            }
        }
        Check(named, std::string(listed.sound) + "'s arrangements are named after their variables");
    }
}

void CheckArchives()
{
    const std::array<uint8_t, 4> bytes = {'F', 'S', 'A', 'R'};
    Check(cafesf2::ProfileOf(bytes) == Profile::kNone, "an unknown archive gets no profile");
    Check(cafesf2::ProfileOf({}) == Profile::kNone, "an empty file gets no profile");
}

// Kirby and the Rainbow Curse's stage select starts its piece with track 7 alone, the area view's track. The piece's 14
// other states follow, each named after its world, with the tracks of the game's table. A track that the options
// already leave out stays out. Another piece plays every track.
void CheckTrackStates()
{
    PerformOptions start;
    cafesf2::SetUpSound(Profile::kRainbowCurse, "BGM_SEQ_MENU_INTERACTIVE", start);
    Check(start.tracks == 0x0080 && start.variables.empty(), "the stage select starts with track 7 alone");

    const std::span<const Arrangement> states =
        cafesf2::ArrangementsOf(Profile::kRainbowCurse, "BGM_SEQ_MENU_INTERACTIVE");
    Check(states.size() == 14 && states[0].suffix == " world 1" && states[0].tracks == 0x0401 &&
              states[6].suffix == " world 7" && states[6].tracks == 0x0040 && states[7].suffix == " world 1 boss" &&
              states[7].tracks == 0x0501 && states[13].suffix == " world 7 boss" && states[13].tracks == 0x0140,
          "the stage select's states follow the game's table");

    PerformOptions boss;
    boss.tracks = 0xfffe;
    cafesf2::SetUpArrangement(states[9], boss);
    Check(boss.tracks == 0x1104, "world 3's boss stage plays tracks 2, 8 and 12");
    cafesf2::SetUpArrangement(states[7], boss);
    Check(boss.tracks == 0x0100, "a track that the options leave out stays out");

    PerformOptions other;
    cafesf2::SetUpSound(Profile::kRainbowCurse, "BGM_SEQ_TITLE", other);
    Check(other.tracks == 0xffff, "the game's other pieces play every track");
}

} // namespace

int main()
{
    CheckStarts();
    CheckArrangements();
    CheckNames();
    CheckArchives();
    CheckTrackStates();

    return xsflib::testing::Summary("cafesf2 profiles");
}

// SPDX-License-Identifier: MIT

// The profiles of games whose code sets the variables that some sequences wait for, or turns some tracks of a piece
// down. cafesf2 picks a profile by the SHA-1 of the sound archive. A profile then starts each such sequence with the
// variables and the tracks that the game's code gives it. The other values that change what the sequence plays, and
// the piece's other states, become its arrangements. Any other archive plays every sequence with its variables at -1
// and every track at full volume. nw::snd starts them that way.

#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

#include "ntdlib/sequence_export/performance.h"

namespace cafesf2
{

// The sound archives with a profile.
enum class Profile
{
    kNone,
    kAmiiboFestival,     // Animal Crossing: amiibo Festival (USA), BBQ_SOUND_ARC.bfsar
    kAmiiboFestivalArc1, // Animal Crossing: amiibo Festival (USA), BBQ_SOUND_ARC1.bfsar
    kMarioParty10,       // Mario Party 10 (USA), 10STARS_SOUND.bfsar
    kNintendoLand,       // Nintendo Land (USA), lunch.bfsar
    kSuperMario3dWorld,  // Super Mario 3D World (USA), SeData.bfsar
    kNewSuperMarioBrosU, // New Super Mario Bros. U (USA), cafe_redpro_sound.bfsar
    kWiiPartyU,          // Wii Party U (USA, Rev 1), NEXPY_SOUND_US.bfsar
    kRainbowCurse,       // Kirby and the Rainbow Curse (USA), seven.bfsar
};

// The profile of the sound archive in `archive`.
Profile ProfileOf(std::span<const uint8_t> archive);

// A value that a game's code gives one of a sequence's variables. Without a track, `index` numbers the variable as
// PerformOptions::variables does: 0 to 15 for the sequence's variables and 16 to 31 for the global ones. With a track
// from 0 to 15, `index` is the track's variable from 0 to 15, which the track's commands number 32 + `index`. An entry
// with index kNone sets nothing.
struct Variable
{
    static constexpr uint8_t kNone = 0xff;
    static constexpr int8_t kNoTrack = -1;

    int8_t track = kNoTrack;
    uint8_t index = kNone;
    int16_t value = -1;
};

using Variables = std::array<Variable, 2>;

// Another state of a sequence: other values of the variables, on which the sequence's branches play something else, or
// other tracks that play. cafesf2 converts the sequence once more for each arrangement, after the sequence itself. The
// file's name is the sequence's name followed by `suffix`. `tracks` holds the tracks that play, bit 0 for track 0, as
// PerformOptions::tracks does. The game's code turns the others down to 0 through the sound's handle.
struct Arrangement
{
    std::string_view suffix;
    Variables variables = {};
    uint16_t tracks = 0xffff;
};

// Sets the variables and the tracks that the game of `profile` gives the sequence named `sound` as it starts the sound.
void SetUpSound(Profile profile, std::string_view sound, ntdlib::sequence_export::PerformOptions& perform);

// The arrangements of the sequence named `sound` in the archive of `profile`. An unlisted sequence has none.
std::span<const Arrangement> ArrangementsOf(Profile profile, std::string_view sound);

// Sets `variables` in `perform`, over any values that it holds for the same variables.
void SetVariables(const Variables& variables, ntdlib::sequence_export::PerformOptions& perform);

// Sets the variables and the tracks of `arrangement` in `perform`. A track that `perform` leaves out stays out.
void SetUpArrangement(const Arrangement& arrangement, ntdlib::sequence_export::PerformOptions& perform);

} // namespace cafesf2

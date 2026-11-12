// SPDX-License-Identifier: MIT

// The starts of the sequences that the game code sets up as the game plays them. The list of starts in ntdlib
// (ntdlib/cafe/snd/sequence_starts.h) finds them by the SHA-1 of the sound archive. A sequence then starts with the
// variables and the tracks that the game code sets for it. The other values that change what the sequence plays, and
// the piece's other states, become its arrangements. Any other archive plays every sequence with its variables at -1
// and every track at full volume. nw::snd starts them that way.

#pragma once

#include "ntdlib/cafe/snd/sequence_starts.h"
#include "ntdlib/sequence_export/performance.h"

namespace cafesf2
{

// Sets the variables and the tracks of `start` in `perform`, over any values that it holds for the same variables. A
// track that `perform` leaves out stays out.
void SetUpStart(const ntdlib::cafe::snd::SequenceStart& start, ntdlib::sequence_export::PerformOptions& perform);

// Sets the variables and the tracks of `arrangement` in `perform`, as SetUpStart does.
void SetUpArrangement(const ntdlib::cafe::snd::SequenceArrangement& arrangement,
                      ntdlib::sequence_export::PerformOptions& perform);

} // namespace cafesf2

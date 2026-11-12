// SPDX-License-Identifier: MIT

// The starts of the sequences (see profiles.h).

#include "profiles.h"

#include <cstdint>

#include "ntdlib/cafe/snd/sequence_starts.h"
#include "ntdlib/sequence_export/performance.h"

namespace cafesf2
{

using ntdlib::cafe::snd::SequenceVariable;
using ntdlib::sequence_export::PerformOptions;
using ntdlib::sequence_export::TrackVariable;

void SetUpStart(const ntdlib::cafe::snd::SequenceStart& start, PerformOptions& perform)
{
    for (const SequenceVariable& variable : start.variables)
    {
        if (variable.index == SequenceVariable::kNone)
        {
            continue;
        }

        if (variable.track == SequenceVariable::kNoTrack)
        {
            perform.variables[variable.index] = variable.value;
        }
        else
        {
            perform.track_variables.push_back(TrackVariable{
                .track = static_cast<uint8_t>(variable.track), .index = variable.index, .value = variable.value});
        }
    }

    perform.tracks = static_cast<uint16_t>(perform.tracks & start.tracks);
}

void SetUpArrangement(const ntdlib::cafe::snd::SequenceArrangement& arrangement, PerformOptions& perform)
{
    SetUpStart({.variables = arrangement.variables, .tracks = arrangement.tracks}, perform);
}

} // namespace cafesf2

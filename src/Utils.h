/*
    Copyright 2023 Zach Vonler

    This file is part of CircuitPlaygroundChronometer.

    CircuitPlaygroundChronometer is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    CircuitPlaygroundChronometer is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
    Public License for more details.

    You should have received a copy of the GNU General Public License along with
    CircuitPlaygroundChronometer.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef utils_h
#define utils_h

#include <FastLED.h>

namespace cp_chrono {

/*---------------------------------------------------------------------------*/

inline CRGB colorFadedBy(CRGB original, int fraction)
{
    uint8_t fadedRed = original.red;
    uint8_t fadedGreen = original.green;
    uint8_t fadedBlue = original.blue;
    nscale8x3_video(fadedRed, fadedGreen, fadedBlue, fraction);
    return CRGB(fadedRed, fadedGreen, fadedBlue);
}

/*---------------------------------------------------------------------------*/

} // namespace cp_chrono

#endif

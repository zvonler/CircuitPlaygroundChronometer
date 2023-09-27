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

#include "ClockDisplay.h"
#include "Utils.h"

namespace {

/*---------------------------------------------------------------------------*/

/**
 * Iterates over the pixels that represent a specific clock numeral.
 */
class PixelIndexIterator {
    static int8_t const* PixelsForClockNumeral[];

    int8_t const* _pixelIndices;
    int _next;

public:
    PixelIndexIterator(int clockNumeralIndex)
        : _pixelIndices(PixelsForClockNumeral[clockNumeralIndex])
        , _next(0)
    { }

    bool hasNext() { return _pixelIndices[_next] != -1; }
    int next() { return hasNext() ? _pixelIndices[_next++] : -1; }
};

int8_t PIXELS_12_OCLOCK[] = { 0,  9, -1 }; // 12 o'clock uses the "top" two LEDs
int8_t PIXELS_1_OCLOCK[]  = { 9, -1     };
int8_t PIXELS_2_OCLOCK[]  = { 8, -1     };
int8_t PIXELS_3_OCLOCK[]  = { 7, -1     };
int8_t PIXELS_4_OCLOCK[]  = { 6, -1     };
int8_t PIXELS_5_OCLOCK[]  = { 5, -1     };
int8_t PIXELS_6_OCLOCK[]  = { 5,  4, -1 }; // 6 o-clock uses the "bottom" two LEDs
int8_t PIXELS_7_OCLOCK[]  = { 4, -1     };
int8_t PIXELS_8_OCLOCK[]  = { 3, -1     };
int8_t PIXELS_9_OCLOCK[]  = { 2, -1     };
int8_t PIXELS_10_OCLOCK[] = { 1, -1     };
int8_t PIXELS_11_OCLOCK[] = { 0, -1     };

int8_t const*
PixelIndexIterator::PixelsForClockNumeral[] =
{
    PIXELS_12_OCLOCK,
    PIXELS_1_OCLOCK,
    PIXELS_2_OCLOCK,
    PIXELS_3_OCLOCK,
    PIXELS_4_OCLOCK,
    PIXELS_5_OCLOCK,
    PIXELS_6_OCLOCK,
    PIXELS_7_OCLOCK,
    PIXELS_8_OCLOCK,
    PIXELS_9_OCLOCK,
    PIXELS_10_OCLOCK,
    PIXELS_11_OCLOCK,
};

/*---------------------------------------------------------------------------*/

} // anonymous namespace

namespace cp_chrono {

/*---------------------------------------------------------------------------*/

void fade_in_at_origin(ClockDisplay& clock, uint32_t elapsed, uint32_t duration, CRGB color)
{
    int offset = (duration - elapsed) / float(duration) * 255;
    auto faded = colorFadedBy(color, 255 - offset);
    clock.addToNumeral(0, faded);
}

void sweep_indicator(ClockDisplay& clock, int numeral, uint32_t elapsed, uint32_t duration, CRGB color)
{
    int steps = 12;
    int pos = min(numeral, steps - (duration - elapsed) / float(duration) * steps);
    clock.addToNumeral(pos, color);
}

void sweep_hour_indicator(ClockDisplay& clock, uint32_t elapsed, uint32_t duration)
{
    sweep_indicator(clock, clock.now_12_hour(), elapsed, duration, clock.hour_color());
}

void show_hour_indicator(ClockDisplay& clock, uint32_t elapsed, uint32_t duration)
{
    clock.addToNumeral(clock.now_12_hour(), clock.hour_color());
}

void sweep_minute_indicator(ClockDisplay& clock, uint32_t elapsed, uint32_t duration)
{
    show_hour_indicator(clock, elapsed, duration);
    sweep_indicator(clock, clock.now_5_minute(), elapsed, duration, clock.minute_color());
}

void show_hour_and_minute_indicator(ClockDisplay& clock, uint32_t elapsed, uint32_t duration)
{
    show_hour_indicator(clock, elapsed, duration);
    clock.addToNumeral(clock.now_5_minute(), clock.minute_color());
}

void sweep_second_indicator(ClockDisplay& clock, uint32_t elapsed, uint32_t duration)
{
    show_hour_and_minute_indicator(clock, elapsed, duration);
    sweep_indicator(clock, clock.now_5_second(), elapsed, duration, clock.second_color());
}

void fade_in_hour_at_origin(ClockDisplay& clock, uint32_t elapsed, uint32_t duration)
{
    fade_in_at_origin(clock, elapsed, duration, clock.hour_color());
}

void fade_in_minute_at_origin(ClockDisplay& clock, uint32_t elapsed, uint32_t duration)
{
    show_hour_indicator(clock, elapsed, duration);
    fade_in_at_origin(clock, elapsed, duration, clock.minute_color());
}

void fade_in_second_at_origin(ClockDisplay& clock, uint32_t elapsed, uint32_t duration)
{
    show_hour_and_minute_indicator(clock, elapsed, duration);
    fade_in_at_origin(clock, elapsed, duration, clock.second_color());
}

void display_time(ClockDisplay& clock, uint32_t elapsed, uint32_t duration)
{
    // Hour indicator is steady
    show_hour_indicator(clock, elapsed, duration);

    // Minute indicator slowly pulses
    clock.addToNumeral(clock.now_5_minute(), colorFadedBy(clock.minute_color(), beatsin8(12, 0, 200)));

    // Seconds indicator flashes at 1Hz
    if (elapsed % 1000 > 500)
        clock.addToNumeral(clock.now_5_second(), clock.second_color());
}

/*---------------------------------------------------------------------------*/

ClockDisplay::AnimationStage
ClockDisplay::orientation_stages[] = {
    {  750, fade_in_hour_at_origin         },
    { 1000, sweep_hour_indicator           },
    {  350, show_hour_indicator            },
    {  750, fade_in_minute_at_origin       },
    { 1000, sweep_minute_indicator         },
    {  350, show_hour_and_minute_indicator },
    {  750, fade_in_second_at_origin       },
    { 1000, sweep_second_indicator         },
    {    0, display_time                   },
};

void
ClockDisplay::update(int64_t now)
{
    _now_tm = now + _offset;
    fadeToBlackBy(_pixels, _num_pixels, 40);

    if (_ampm_indicator_pin >= 0) {
        // Turn indicator on for PM
        digitalWrite(_ampm_indicator_pin, !now_is_am());
    }

    auto anim_tm = _now_tm - _reset_tm;
    uint32_t stage_start = 0;
    int i = 0;
    while (true) {
        auto stage = &orientation_stages[i++];
        if (anim_tm < stage_start + stage->duration || stage->duration == 0)
        {
            (*stage->update)(*this, anim_tm - stage_start, stage->duration);
            break;
        }
        stage_start += stage->duration;
    }
}

void
ClockDisplay::addToNumeral(int numeral, CRGB color)
{
    auto iter = PixelIndexIterator(numeral);
    while (iter.hasNext()) {
        _pixels[iter.next()] += color;
    }
}

/*---------------------------------------------------------------------------*/

} // namespace cp_chrono

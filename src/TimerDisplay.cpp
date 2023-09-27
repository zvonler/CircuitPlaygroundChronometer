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

#include "TimerDisplay.h"
#include "Utils.h"

namespace cp_chrono {

/*---------------------------------------------------------------------------*/

bool
TimerDisplay::update(int64_t tm)
{
    if (timeout_running()) {
        if (!timeout_remaining(tm)) {
            clear_timeout();
            return true;
        }
    } else if (timer_running()) {
        if (timer_elapsed(tm) >= max_timeout()) {
            stop_timer();
            return true;
        }
    }
    return false;
}

void
TimerDisplay::show(int64_t tm)
{
    constexpr static int8_t PIXELS_CW_FROM_12_OCLOCK[] =
    {
        9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    };

    if (timeout_running()) {
        fill_solid(_pixels, _num_pixels, 0);
        if (_heartbeat_indicator_pin >= 0)
            digitalWrite(_heartbeat_indicator_pin, 0);

        int64_t remaining_ms = min(max_timeout(), timeout_remaining(tm));
        int num_lit = (remaining_ms - 1) / _ms_per_pixel;
        float frac_remaining = (remaining_ms % _ms_per_pixel) / (float)_ms_per_pixel;

        uint8_t base_hue = (tm / 40) % 256;
        constexpr static uint8_t hue_step = 10;
        for (int i = 0; i <= num_lit; ++i) {
            auto pixel_hue = (base_hue + i * hue_step) % 256;
            fill_rainbow(_pixels + PIXELS_CW_FROM_12_OCLOCK[i], 1, pixel_hue, 0);
        }

        if (frac_remaining) {
            // Fade the last pixel at a varying rate
            uint8_t fade_div = 1;
            if (frac_remaining >= 0.5)
                fade_div = 3;
            else if (frac_remaining >= 0.2)
                fade_div = 2;
            else
                fade_div = 1;
            fadeToBlackBy(_pixels + PIXELS_CW_FROM_12_OCLOCK[num_lit], 1, (remaining_ms >> fade_div) % 256);
        }
    } else if (timer_running()) {
        if (_heartbeat_indicator_pin >= 0)
            digitalWrite(_heartbeat_indicator_pin, 0);

        int64_t elapsed_ms = timer_elapsed(tm);
        int num_lit = elapsed_ms / _ms_per_pixel;

        // Completed pixels are full green, and we fade out any of
        // the blue sweeper.
        for (int i = 0; i < num_lit; ++i) {
            auto pixel_idx = PIXELS_CW_FROM_12_OCLOCK[i];
            _pixels[pixel_idx] += CRGB::Green;
            fadeUsingColor(_pixels + pixel_idx, 1, CRGB(0, 255, 10));
        }

        auto num_incomplete = _num_pixels - num_lit;
        fadeToBlackBy(_pixels + 1, num_incomplete - 1, 20);
        auto idx_range = _num_pixels * 2;
        auto lit_pixel = (elapsed_ms / 125) % idx_range;
        if (lit_pixel < _num_pixels) {
            _pixels[_num_pixels - lit_pixel - 1] += CRGB::Blue;
        } else {
            fadeToBlackBy(_pixels, 1, 8);
        }
    } else {
        fill_solid(_pixels, _num_pixels, 0);
        if (_heartbeat_indicator_pin >= 0)
            digitalWrite(_heartbeat_indicator_pin, (tm % 1024) < 512);
    }
}

/*---------------------------------------------------------------------------*/

} // namespace cp_chrono

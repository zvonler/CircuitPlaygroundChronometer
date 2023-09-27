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

#include "CPChronometer.h"

#include <Adafruit_CircuitPlayground.h>
#include <CircuitPlaygroundGestures.h>
#include <FastLED.h>

namespace cp_chrono {

using CPG = CircuitPlaygroundGestures;

/*---------------------------------------------------------------------------*/

CPChronometer::CPChronometer()
    : _clock_display(_pixels, NUM_PIXELS, LED_BUILTIN)
    , _timer_display(_pixels, NUM_PIXELS, MS_PER_PIXEL, LED_BUILTIN)
{ }

void
CPChronometer::begin()
{
    _led_controller = &FastLED.addLeds<WS2811, CPLAY_NEOPIXELPIN, GRB>(_pixels, NUM_PIXELS);
    _led_controller->setCorrection(TypicalLEDStrip);
    pinMode(LED_BUILTIN, OUTPUT);
    CircuitPlaygroundGestures::instance().begin();
}

void
CPChronometer::reset(int64_t tm)
{
    _mode = CircuitPlayground.slideSwitch() ? CLOCK : TIMER;
    _timer_display.reset();
    _clock_display.reset(tm);
}

void
CPChronometer::update(int64_t now)
{
    check_for_gesture(now);

    if (_timer_display.update(now)) {
        // Timer ran out
        CircuitPlayground.playTone(700, 150);
        CircuitPlayground.playTone(650, 50);
        CircuitPlayground.playTone(700, 100);
    }

    if (_mode == CLOCK) {
        _clock_display.update(now);
    } else if (_mode == TIMER) {
        _timer_display.show(now);
    }

    _led_controller->showLeds(BRIGHTNESS);
}

void
CPChronometer::check_for_gesture(int64_t now)
{
    auto& cpg = CircuitPlaygroundGestures::instance();

    auto gesture = cpg.update(now);

    if (gesture == CPG::SLIDE_SWITCHED_ON) {
        if (_mode != CLOCK) {
            _mode = CLOCK;
            _clock_display.reset(now);
        }
        return;
    } else if (gesture == CPG::SLIDE_SWITCHED_OFF) {
        // Switch in TIMER mode position, see if that's new
        if (_mode != TIMER) {
            _mode = TIMER;
        }
        return;
    }

    // Other gestures cause adjustments, so don't do them too often
    if (now - _last_adjustment_tm < 250)
        return;

    bool adjusted = false;
    if (_mode == CPChronometer::CLOCK) {
        if (gesture == CPG::LEFT_HELD_RIGHT_CLICKED) {
            _clock_display.increase_offset(60 * 1000);
            adjusted = true;
        } else if (gesture == CPG::RIGHT_HELD_LEFT_CLICKED) {
            _clock_display.increase_offset(-60 * 1000);
            adjusted = true;
        } else if (cpg.both_pressed() && cpg.duration(now) >= 500) {
            auto duration = cpg.duration(now);
            int64_t increase = 0;
            if (duration < 1000) {
                increase = 60 * 1000;
            } else if (duration < 5000) {
                increase = 300 * 1000;
            } else {
                increase = 3600 * 1000;
            }
            auto sign = cpg.left_duration(now) > cpg.right_duration(now) ? 1 : -1;
            _clock_display.increase_offset(sign * increase);
            adjusted = true;
        }
    } else {
        if (gesture == CPG::RIGHT_CLICKED) {
            if (!_timer_display.timer_running()) {
                _timer_display.set_timeout(now, _timer_display.timeout_remaining(now) + MS_PER_PIXEL);
                adjusted = true;
            }
        } else if (gesture == CPG::LEFT_CLICKED) {
            if (!_timer_display.timeout_running()) {
                if (!_timer_display.timer_running()) {
                    _timer_display.start_timer(now);
                    adjusted = true;
                }
            }
        } else if (gesture == CPG::BOTH_PRESSED) {
            _timer_display.reset();
            _timer_display.clear_timeout();
            adjusted = true;
        }
    }
    if (adjusted)
        _last_adjustment_tm = now;
}

/*---------------------------------------------------------------------------*/

} // namespace cp_chrono


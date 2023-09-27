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

#ifndef cp_chronometer_h
#define cp_chronometer_h

#include "ClockDisplay.h"
#include "TimerDisplay.h"

namespace cp_chrono {

/*---------------------------------------------------------------------------*/

/**
 * Implements a chronometer with clock and timer functions on the Adafruit
 * Circuit Playground.
 */
class CPChronometer {
public:
    enum Mode {
        CLOCK,
        TIMER,
    };

    /**
     * Instantiates the chronometer.
     */
    CPChronometer();

    /**
     * Initializes LEDs, should be called once from setup().
     */
    void begin();

    /**
     * Resets the chronometer as if it had just turned on at tm. Any running
     * timer is stopped, and the clock will run its sweep-in animation next
     * time it is displayed.
     */
    void reset(int64_t tm);

    /**
     * Updates the chronometer to now. This function should be called
     * frequently to process input and update the display.
     */
    void update(int64_t now);


    // Returns the current offset of the clock.
    int32_t clock_offset() const { return _clock_display.offset(); }

    // Adds the adjustment to the clock offset.
    void increase_clock_offset(int32_t adjustment) { _clock_display.increase_offset(adjustment); }

    // Returns the time the clock would display at tm (i.e. tm adjusted by the clock's offset)
    int64_t clock_display_tm(int64_t tm) const { return _clock_display.display_tm(tm); }

    // The number of NeoPixels available
    constexpr static int NUM_PIXELS = 10;

    // The number of milliseconds each pixel represents in timer mode
    constexpr static uint32_t MS_PER_PIXEL = 60 * 1000;

    // The maximum countup or countdown timer value
    constexpr static uint32_t MAX_TIMEOUT = MS_PER_PIXEL * NUM_PIXELS;

    constexpr static uint8_t BRIGHTNESS = 8;

private:
    void check_for_gesture(int64_t now);

    Mode _mode = CLOCK;
    CRGB _pixels[NUM_PIXELS];
    CLEDController* _led_controller;
    ClockDisplay _clock_display;
    TimerDisplay _timer_display;
    int64_t _last_adjustment_tm = 0;
};

/*---------------------------------------------------------------------------*/

} // namespace cp_chrono

#endif

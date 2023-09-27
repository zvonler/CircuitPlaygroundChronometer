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

#ifndef timer_display_h
#define timer_display_h

#include <FastLED.h>

namespace cp_chrono {

/*---------------------------------------------------------------------------*/

/**
 * Displays a count up or count down timer on the Adafruit Circuit Playground.
 * In this class, the countup functionality is accessed through the methods with
 * "timer" in their name, and the countdown functionality through the methods
 * with "timeout" in their name.
 */
class TimerDisplay
{
public:
    TimerDisplay(CRGB* pixels, int num_pixels, uint32_t ms_per_pixel, int heartbeat_indicator_pin = -1)
        : _pixels(pixels)
        , _num_pixels(num_pixels)
        , _ms_per_pixel(ms_per_pixel)
        , _heartbeat_indicator_pin(heartbeat_indicator_pin)
    { }

    /**
     * Updates the timer's state if it is running. Returns true if the timer
     * completed counting up or down, false otherwise.
     */
    bool update(int64_t tm);

    /**
     * Displays the timer on the NeoPixels.
     */
    void show(int64_t tm);

    /**
     * Stops the timer if it is running.
     */
    void reset()
    {
        stop_timer();
        clear_timeout();
    }

    void set_timeout(int64_t now, uint32_t duration)
    {
        _timeout_tm = now + min(duration, max_timeout());
    }

    void clear_timeout() { _timeout_tm = 0; }
    bool timeout_running() const { return _timeout_tm != 0; }
    int64_t timeout_remaining(int64_t tm) const
    {
        return _timeout_tm >= tm ? _timeout_tm - tm : 0;
    }

    void start_timer(int64_t tm) { _timer_start_tm = tm; }
    void stop_timer() { _timer_start_tm = 0; }
    bool timer_running() const { return _timer_start_tm != 0; }
    int64_t timer_elapsed(int64_t tm)
    {
        return tm > _timer_start_tm ? min(max_timeout(), tm - _timer_start_tm) : 0;
    }

private:
    uint32_t max_timeout() const { return _ms_per_pixel * _num_pixels; }

    CRGB* _pixels;
    int _num_pixels;
    int _heartbeat_indicator_pin;
    uint32_t _ms_per_pixel;
    int64_t _timer_start_tm = 0;
    int64_t _timeout_tm = 0;
};

/*---------------------------------------------------------------------------*/

} // namespace cp_chrono

#endif

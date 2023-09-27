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

#ifndef clock_display_h
#define clock_display_h

#include <FastLED.h>
#include <RTClib.h>

namespace cp_chrono {

/*---------------------------------------------------------------------------*/

/**
 * Uses the Circuit Playground built-in LEDs to display an analog clock face.
 */
class ClockDisplay
{
    CRGB* _pixels;
    int _num_pixels;
    int _ampm_indicator_pin;
    int64_t _offset = 0;
    int64_t _reset_tm = 0;
    int64_t _now_tm = 0;

public:
    ClockDisplay(CRGB* pixels, int num_pixels, int ampm_indicator_pin = -1)
        : _pixels(pixels)
        , _num_pixels(num_pixels)
        , _ampm_indicator_pin(ampm_indicator_pin)
    { }

    int64_t offset() const { return _offset; }

    void increase_offset(int64_t adjustment)
    {
        _offset += adjustment;
        if (_offset < 0)
            _offset += 86400 * 1000;
    }

    int64_t display_tm(int64_t tm) const
    {
        return tm + _offset;
    }

    void reset(int64_t reset_tm)
    {
        _reset_tm = reset_tm + _offset;
    }

    void update(int64_t now);

    void addToNumeral(int numeral, CRGB color);

    DateTime now() const { return DateTime(_now_tm / 1000); }

    // Returns the current hour from 0-11
    uint8_t now_12_hour()  const { return now().hour() % 12;  }
    // Returns the current 5-minute period from 0-11
    uint8_t now_5_minute() const { return now().minute() / 5; }
    // Returns the current 5-second period from 0-11
    uint8_t now_5_second() const { return now().second() / 5; }
    // Returns true if the current time is AM (before noon)
    bool now_is_am() const { return now().hour() < 12; }

    CRGB hour_color()   const { return CRGB::Red;   }
    CRGB minute_color() const { return CRGB::Green; }
    CRGB second_color() const { return CRGB::Blue;  }

private:
    using AnimationStageUpdate = void (*)(ClockDisplay&, uint32_t, uint32_t);

    struct AnimationStage
    {
        uint32_t duration = 0;
        AnimationStageUpdate update = nullptr;

        AnimationStage(uint32_t d, AnimationStageUpdate u)
            : duration(d)
            , update(u)
        { }
    };

    static AnimationStage orientation_stages[];
};

/*---------------------------------------------------------------------------*/

} // namespace cp_chrono

#endif

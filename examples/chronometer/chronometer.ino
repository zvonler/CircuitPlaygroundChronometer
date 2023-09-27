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
#include <FastLED.h>
#include <RTClib.h>

/*---------------------------------------------------------------------------*/

cp_chrono::CPChronometer cpc;

// The RTC and stored offset are used only if USE_RTC is set when building.
#ifdef USE_RTC
#include <FlashAsEEPROM.h>
int32_t stored_offset = 0;
RTC_PCF8523 rtc;
void init_rtc()
{
    if (!rtc.begin()) {
        halt_and_catch_fire("RTC begin failed");
        // Unreachable
    }
    if (!rtc.initialized() || rtc.lostPower()) {
        Serial.println("WARN: RTC not initialized");
    }
    rtc.start();

    if (!EEPROM.isValid()) {
        // If invalid, initialize the stored offset to 0
        for (int i = 0; i < sizeof(stored_offset); ++i) {
            EEPROM.write(i, 0);
        }
        EEPROM.commit();
    } else {
        for (int i = 0; i < sizeof(stored_offset); ++i) {
            // Little-endian
            stored_offset += (EEPROM.read(i) << i * 8);
        }
        Serial.print("Read stored clock offset ");
        Serial.println(stored_offset);
        cpc.increase_clock_offset(stored_offset);
    }
}
int64_t slew = 0;
int64_t now_callback()
{
    static int64_t last_rtc_sync_tm = 0;
    static int64_t last_millis_sync_tm = 0;

    int64_t millis_now = millis();
    if (!last_rtc_sync_tm && !last_millis_sync_tm) {
        // First time through
        last_rtc_sync_tm = ((int64_t)rtc.now().unixtime()) * 1000;
        last_millis_sync_tm = millis_now;
        slew = last_rtc_sync_tm - last_millis_sync_tm;
    } else {
        EVERY_N_SECONDS(100) {
            int64_t rtc_tm = ((int64_t)rtc.now().unixtime()) * 1000;
            int64_t rtc_diff = rtc_tm - last_rtc_sync_tm;
            int64_t millis_diff = millis_now - last_millis_sync_tm;
            slew += (rtc_tm - last_rtc_sync_tm) - (millis_now - last_millis_sync_tm);
            last_rtc_sync_tm = rtc_tm;
            last_millis_sync_tm = millis_now;
        }
    }
    return millis_now + slew;
}
#else
int64_t now_callback()
{
    auto offset = 222 * 60 * 1000; // Start the epoch at about 10:10
    return offset + millis();
}
#endif

void halt_and_catch_fire(const char* message)
{
    Serial.println(message);
    Serial.flush();
    while (true)
        delay(100);
}

/*---------------------------------------------------------------------------*/

void setup()
{
    CircuitPlayground.begin();

    Serial.begin(115200);

    cpc.begin();

#ifdef USE_RTC
    init_rtc();
#else
    Serial.println("Using software clock");
#endif

    FastLED.setBrightness(6);

    cpc.reset((*now_callback)());
}

void loop()
{
    auto now = (*now_callback)();

    // The CPChronometer handles all the button input and LED output
    cpc.update(now);

#ifdef USE_RTC
    // Once per minute, see if the user has changed the clock offset from
    // what is stored in flash and update if necessary.
    EVERY_N_SECONDS(60) {
        if (stored_offset != cpc.clock_offset()) {
            stored_offset = cpc.clock_offset();
            for (int i = 0; i < sizeof(stored_offset); ++i) {
                // Little-endian
                EEPROM.write(i, (stored_offset & (0xFF << (i * 8))) >> (i * 8));
            }
            EEPROM.commit();
        }
    }
#endif

    // Write the display time to the serial port periodically for debugging
    EVERY_N_SECONDS(5) {
        DateTime dt_now(cpc.clock_display_tm(now) / 1000);
        if (dt_now.hour() < 10)
            Serial.print('0');
        Serial.print(dt_now.hour(), DEC);
        Serial.print(':');
        if (dt_now.minute() < 10)
            Serial.print('0');
        Serial.print(dt_now.minute(), DEC);
        Serial.print(':');
        if (dt_now.second() < 10)
            Serial.print('0');
        Serial.print(dt_now.second(), DEC);
#ifdef USE_RTC
        Serial.print(" (slew: ");
        Serial.print(slew);
        Serial.print(")");
#endif
        Serial.print(" (offset: ");
        Serial.print(cpc.clock_offset());
        Serial.print(")");
        Serial.println();
    }

    FastLED.delay(1000 / 120);
}

/*---------------------------------------------------------------------------*/

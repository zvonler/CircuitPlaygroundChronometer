/*
  rtc_pcf8523

  Turns the Circuit Playground into a chronometer that uses a PCF8523 real-time
  clock to track time.

  This example code is in the public domain.
*/

#include "CPChronometer.h"
#include <Adafruit_CircuitPlayground.h>
#include <FastLED.h>
#include <FlashAsEEPROM.h>
#include <RTClib.h>

/*---------------------------------------------------------------------------*/

cp_chrono::CPChronometer cpc;

int32_t stored_offset = 0;
RTC_PCF8523 rtc;
int64_t slew = 0;

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

    init_rtc();

    FastLED.setBrightness(6);

    cpc.reset((*now_callback)());
}

void loop()
{
    auto now = (*now_callback)();

    // The CPChronometer handles all the button input and LED output
    cpc.update(now);

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
        Serial.print(" (slew: ");
        Serial.print(uint32_t(slew / 1000));
        Serial.print(".");
        Serial.print(int(slew % 1000));
        Serial.print(slew);
        Serial.print(")");
        Serial.print(" (offset: ");
        Serial.print(cpc.clock_offset());
        Serial.print(")");
        Serial.println();
    }

    FastLED.delay(1000 / 120);
}

/*---------------------------------------------------------------------------*/

/*
  software_clock

  Turns the Circuit Playground into a chronometer using the millis() system
  function to track time.

  This example code is in the public domain.
*/

#include "CPChronometer.h"
#include <Adafruit_CircuitPlayground.h>
#include <FastLED.h>
#include <RTClib.h>

/*---------------------------------------------------------------------------*/

cp_chrono::CPChronometer cpc;

int64_t now_callback()
{
    auto offset = 222 * 60 * 1000; // Start the epoch at about 10:10
    return offset + millis();
}

/*---------------------------------------------------------------------------*/

void setup()
{
    CircuitPlayground.begin();

    Serial.begin(115200);

    cpc.begin();

    FastLED.setBrightness(6);

    cpc.reset((*now_callback)());
}

void loop()
{
    auto now = (*now_callback)();

    // The CPChronometer handles all the button input and LED output
    cpc.update(now);

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
        Serial.print(" (offset: ");
        Serial.print(cpc.clock_offset());
        Serial.print(")");
        Serial.println();
    }

    FastLED.delay(1000 / 120);
}

/*---------------------------------------------------------------------------*/

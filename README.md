
# Circuit Playground Chronometer

This library implements a chronometer on the Adafruit Circuit Playground, using
the built-in buttons and slide switch for control and the built-in NeoPixels for
display.

Two example sketches are provided, one with a software clock implementation and
another that uses a PCF8523 RTC.  The software clock sketch uses `millis()` to
measure time, which is usable for the timer functions but drifts too quickly for
the clock function to be very useful.  The PCF8523 sketch uses the RTC to mark
time and also stores a user-adjustable offset value in the Circuit Playground's
flash memory so that once the clock is set to local time that setting will
persist across restarts and power loss.

### Operation

The slide switch determines whether the chronometer is in Clock mode or Timer
mode.  When a timer has been started, switching to Clock mode does not stop or
reset the timer.

#### Clock mode

In Clock mode, the NeoPixels are used to display an hour, minute, and second
indicator. The ten NeoPixels represent the twelve clock numerals by
"overloading" the top two pixels to indicate 12'o-clock and the bottom two
pixels to indicate 6'o-clock. In this description, "top two pixels" mean the
ones adjacent to the USB plug on the Circuit Playground, and "bottom two pixels"
are the ones adjacent to the battery plug.

The built-in LED mounted next to the USB plug is turned on to indicate PM times.

The color scheme of the clock display is as follows:
 - The current hour is indicated with solid red
 - The current minute is indicated with slowly pulsing green
 - The current second is indicated with flashing 
blue

Clock displaying 04:30: ![0430_or_1630](https://github.com/zvonler/CircuitPlaygroundChronometer/assets/19316003/7f7fa97e-6074-4240-9b17-d34e7a9f11d2)

Clock displaying 09:00: ![0900_or_2100](https://github.com/zvonler/CircuitPlaygroundChronometer/assets/19316003/5da281ff-2066-45cc-b471-13dd71892c09)


When one or more of the indicators overlaps another, all of colors will be shown
simultaneously on the pixel(s) that overlap.

When first powered up in Clock mode, and whenever the switch is moved to Clock mode,
the NeoPixels display a short animation to help orient the user. The animation
sweeps the indicator for the hour, then the minute, then the second with each
starting from the 12'o-clock position.

Clock starting up and displaying about 10:10:
<video src="https://github.com/zvonler/CircuitPlaygroundChronometer/assets/19316003/28cf7ee6-0e5c-4f4b-8805-2a2863df8e4f"></video>

The displayed time can be adjusted to set it to the local time using the
built-in buttons. Holding the left button and clicking the right button will add
one minute to the current time, and holding the right button and clicking the
left button will subtract one minute. Holding the left button and then long
pressing the right button will adjust the time forward in larger increments, and
holding the right button and long pressing the left button will adjust the time
backwards in larger increments.

#### Timer mode

In Timer mode, the NeoPixels display the timer's current status, and the buttons
can be used to control it. The timer is either stopped, counting up, or counting
down. To switch from counting up to counting down and vice versa, the timer must
first be stopped.

To stop the timer and reset it, press both buttons at once. The NeoPixels are
blank when the timer is stopped, and the built-in LED will pulse to show that
the chronometer is still functioning.

To start the count up timer, press the left button. The count up timer uses a
clockwise-moving sweeping blue indicator, as well as solid green pixels that
accrue in a clockwise fashion to indicate completed timer units (by default each
pixel represents one minute). The count up timer will beep when the maximum
elapsed time is reached (10 timer units).

When stopped, and when the countdown timer is already running, the right button
adds one timer unit (by default each pixel represents one minute) to the
countdown timer. Pressing the right button multiple times will add more time to
the countdown timer, up to the maximum possible (10 times the timer unit size).
The remaining countdown time is represented by red pixels that disappear in a
counter-clockwise fashion. The countdown timer beeps when it reaches zero time
remaining.

Countdown timer being set to 5 pixels (each pixel represents 10s in this video):
<video src="https://github.com/zvonler/CircuitPlaygroundChronometer/assets/19316003/df013628-75b6-4eeb-8e85-d90bae15da6a"></video>

Countup timer (each green pixel represents 10s in this video):
<video src="https://github.com/zvonler/CircuitPlaygroundChronometer/assets/19316003/90d77e64-bb30-452b-89e9-8a3e4bfd3fce"></video>


#Chronomix

## Description

Designed to control 6 digits of Signalex 7-segment controllers. If you have an RTC, it can do DateTime operations, including showing the time at different levels of granularity, intervals, countdowns, and some strings.

Check out my Hackaday.io page to get a better idea of what the library can do!

### Dependencies
- [Signalex](https://github.com/krebera/Signalex) library
- Arduino Queue library
- Adafruit MCP23017 library
- Adafruit RTCLib library

## Directions

You can declare an entire clock like so:

```
//Coil charge time in milliseconds and max number of simultaneous active coils
Chronomix myChronomix = Chronomix(75, 5);
```

In the setup function just have it:

`myChronomix.setup();`

In order to initialize.
After that, make sure in your `Update` function you are calling 

```
myChronomix.updateCoils(millis());
```

*It is important that you DO NOT use any delay() functions in your code. It is bad practice and will break the timing mechanisms of the clock*

After you have set up the clock, you are welcome to take advantage of all of the display functions present in the header file:

```
void clearClock();
void clearDigit(byte digit);
void clearDigitUnforced(byte digit);
void setDigit(byte digit, int displayNum);
void setDigit(byte digit, char displayChar);
void displayString(String toDisplay, bool leftToRight);
void displayString(char toDisplay[], bool leftToRight);
void displayLocalTime(struct tm timeInfo);
void displayLocalTime(DateTime timeInfo);
void displayMilitaryTime(DateTime timeInfo);
void displayConfiguredTime(DateTime timeinfo, bool military, bool granular);
void displayTimeSpan(TimeSpan timeInfo);
void displayTimeSpanExact(TimeSpan timeInfo, int mils);
void displayTimeSpanAdaptive(TimeSpan timeInfo);
```

## How it Works

The clock is continually managing two competing queues: One for activation of coils and one for deactivations. This allows it to continually update itself and maintain internal state pretty well. Every time you ask it to flip a segment on or off, it adds that segment onto the managed activation queue, which does what it can depending on coil charge time and max active coils. Once the time limit for activation has been reached, it is thrown onto the deactivation queue and state it updated.

This allows the clock to also not charge a segment it knows is already active. If a new activation comes up on an already activated coil, it will simply skip this step.

Clearing a digit forced will wipe the activation queue for that coil. Unforced will have it work with it.
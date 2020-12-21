/*
  Chronomix.h - Library for managing an entire signalex clock
  Using Clockboard by Keenan Rebera
  Created by Keenan Rebera April 6, 2020
*/

#ifndef Chronomix_h
#define Chronomix_h

#include "Arduino.h"
#include <Signalex.h>
#include <Queue.h>
#include "time.h"
#include <RTClib.h>

#define NUM_SEGMENTS 7
#define NUM_DIGITS 6

class Chronomix {
    public:
        Chronomix(int coilTime, int maxCoils);
        void setup();
        void updateCoils(unsigned long systemTime);
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
    private:
        struct deactivation {
            byte coil;
            unsigned long deactivationTime;
        }; typedef struct deactivation Deactivation;

        struct activation {
            byte coil;
            bool forced;
        }; typedef struct activation Activation;

        int _coilTime;
        int _maxCoils;
        Queue<Activation> actionQueue;
        Queue<Deactivation> deactivationQueue;
        bool segmentStates[NUM_DIGITS * NUM_SEGMENTS];
        Signalex digits[NUM_DIGITS];
        bool _shouldFlush = false;
        void displayHourMin(int hour, int min);
        void displayHourMinSec(int hour, int min, int sec);
        void displayMilitary(int hour, int min);
        void scheduleCoilActivation(byte coil, bool forced);
        void scheduleCoilDeactivation(byte coil);
        bool coilsFull();
        bool pendingActions();
        bool pendingDeactivations();
        void activateCoil(Activation activation);
        void deactivateCoil(byte coil);
        int segmentForCoil(byte coil);
        int digitForCoil(byte coil);
        bool coilType(byte coil);
        bool shouldMoveCoil(byte coil);
        byte coilForDigitData(byte digit, byte segment, bool val);
        byte int2byte(int num);
        byte char2byte(char thisChar);
};

#endif
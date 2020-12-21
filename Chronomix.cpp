#include "Arduino.h"
#include "Chronomix.h"

Chronomix::Chronomix(int coilTime, int maxCoils) : _coilTime(coilTime), _maxCoils(maxCoils) {}

// =============== USER FACING FUNCTIONS

void Chronomix::setup(){
      for(int i = 0; i < NUM_DIGITS; ++i) {
        digits[i].setAddress(i);
        digits[i].setup();
      }
}

void Chronomix::displayHourMin(int hour, int min) {
    setDigit(5, hour < 12 ? 'A' : 'P');
    clearDigitUnforced(4);
    if(hour > 12) hour -=12;
    if(hour == 0) hour = 12;

    setDigit(0, (hour >= 10) ? hour / 10 : 0);
    setDigit(1, hour % 10);

    setDigit(2, (min >= 10) ? min / 10 : 0);
    setDigit(3, min % 10);
}

void Chronomix::displayHourMinSec(int hour, int min, int sec) {
    setDigit(0, (hour >= 10) ? hour / 10 : 0);
    setDigit(1, hour % 10);

    setDigit(2, (min >= 10) ? min / 10 : 0);
    setDigit(3, min % 10);

    setDigit(4, (sec >= 10) ? sec / 10 : 0);
    setDigit(5, sec % 10);
}

void Chronomix::displayMilitary(int hour, int min) {
    setDigit(0, (hour >= 10) ? hour / 10 : 0);
    setDigit(1, hour % 10);

    setDigit(2, (min >= 10) ? min / 10 : 0);
    setDigit(3, min % 10);
    clearDigitUnforced(4);
    clearDigitUnforced(5);
}

void Chronomix::displayConfiguredTime(DateTime timeInfo, bool military, bool granular) {
  if(!granular) {
    if(military) displayMilitaryTime(timeInfo);
    else displayLocalTime(timeInfo);
  } else {
    int hour = timeInfo.hour();
    if(!military) {
      if(hour > 12) hour -=12;
      if(hour == 0) hour = 12;
    }
    displayHourMinSec(hour, timeInfo.minute(), timeInfo.second());
  }
}

void Chronomix::displayTimeSpanAdaptive(TimeSpan timeInfo) {
  int days = timeInfo.days();
  int hours = timeInfo.hours();
  int mins = timeInfo.minutes();
  int secs = timeInfo.seconds();

  if(days > 0) {
    setDigit(0, (days >= 10) ? days / 10 : 0);
    setDigit(1, days % 10);

    setDigit(2, (hours >= 10) ? hours / 10 : 0);
    setDigit(3, hours % 10);

    setDigit(4, (mins >= 10) ? mins / 10 : 0);
    setDigit(5, mins % 10);
  } else {
    setDigit(0, (hours >= 10) ? hours / 10 : 0);
    setDigit(1, hours % 10);

    setDigit(2, (mins >= 10) ? mins / 10 : 0);
    setDigit(3, mins % 10);

    setDigit(4, (secs >= 10) ? secs / 10 : 0);
    setDigit(5, secs % 10);
  }
}

void Chronomix::displayTimeSpan(TimeSpan timeInfo) {
    int hours = timeInfo.hours();
    int mins = timeInfo.minutes();
    int secs = timeInfo.seconds();

    setDigit(0, (hours >= 10) ? hours / 10 : 0);
    setDigit(1, hours % 10);

    setDigit(2, (mins >= 10) ? mins / 10 : 0);
    setDigit(3, mins % 10);

    setDigit(4, (secs >= 10) ? secs / 10 : 0);
    setDigit(5, secs % 10);
}

void Chronomix::displayTimeSpanExact(TimeSpan timeInfo, int mils) {
    int mins = timeInfo.minutes();
    int secs = timeInfo.seconds();

    setDigit(0, (mins >= 10) ? mins / 10 : 0);
    setDigit(1, mins % 10);

    setDigit(2, (secs >= 10) ? secs / 10 : 0);
    setDigit(3, secs % 10);

    setDigit(4, (mils >= 10) ? mils / 10 : 0);
    setDigit(5, mils % 10);
}

void Chronomix::displayLocalTime(DateTime timeInfo) {
    displayHourMin(timeInfo.hour(), timeInfo.minute());
}

void Chronomix::displayMilitaryTime(DateTime timeInfo) {
    displayMilitary(timeInfo.hour(), timeInfo.minute());
}

void Chronomix::displayLocalTime(struct tm timeInfo) {
    displayHourMin(timeInfo.tm_hour, timeInfo.tm_min);
}

void Chronomix::displayString(String toDisplay, bool leftToRight) {
    char chars[NUM_DIGITS];
    toDisplay.toCharArray(chars, NUM_DIGITS);
    displayString(chars, leftToRight);
}

void Chronomix::displayString(char toDisplay[], bool leftToRight) {
    size_t len = strlen(toDisplay);
    if(leftToRight) {
        for(int i = 0; i < NUM_DIGITS; ++i) {
            if(i > len - 1) clearDigit(i);
            setDigit(i, toDisplay[i]);
        }
    } else {
        for(int i = NUM_DIGITS - 1; i >= 0; --i) {
            if(i > len - 1) continue;
            setDigit(i, toDisplay[i]);
        }
    }
}

void Chronomix::setDigit(byte digit, int displayNum) {
  byte displayByte = int2byte(displayNum);
  //error in parsing
  if(displayByte == B0000000) return;
  for(byte i = 0; i < 7; ++i) {
    bool thisBit = bitRead(displayByte, i);
    scheduleCoilActivation(coilForDigitData(digit, i, thisBit), false);
  }
}

void Chronomix::setDigit(byte digit, char displayChar) {
  byte displayByte = char2byte(displayChar);
  //error in parsing
  if(displayByte == B0000000) return;
  for(byte i = 0; i < 7; ++i) {
    bool thisBit = bitRead(displayByte, i);
    scheduleCoilActivation(coilForDigitData(digit, i, thisBit), false);
  }
}

void Chronomix::clearDigit(byte digit) {
  for(byte i = 0; i < 7; ++i) {
    //Forced activation
    scheduleCoilActivation(coilForDigitData(digit, i, false), true);
  }
}

void Chronomix::clearDigitUnforced(byte digit) {
  for(byte i = 0; i < 7; ++i) {
    //Forced activation
    scheduleCoilActivation(coilForDigitData(digit, i, false), false);
  }
}

void Chronomix::clearClock() {
  //Clear all pending coil activations
  //Let the deactivations run their course tho
  while(pendingActions()) {
      actionQueue.pop();
  }
  _shouldFlush = true;
  //Now reset all digits
  for(byte i = 0; i < NUM_DIGITS; ++i) {
    clearDigit(i);
  }
}

//This should be called every update and should be passed millis()
void Chronomix::updateCoils(unsigned long systemTime) {
  // ===== COIL ACTIVATION
  //If we aren't at full throughput
  while(!coilsFull() && pendingActions() && !_shouldFlush){
    //Take something off the queue
    Activation thisActivation = actionQueue.pop();
    // TODO: Method for activating the coil based on lookup byte
    activateCoil(thisActivation);
  }
  
  // ===== COIL DEACTIVATION
  unsigned long currentTime = systemTime;
  Deactivation front = deactivationQueue.peek();
  //Ability to dequeue a ton of coils at once if possible
  while(front.deactivationTime <= currentTime && pendingDeactivations()) {
    deactivationQueue.pop();
    deactivateCoil(front.coil);
    front = deactivationQueue.peek();
  }
  if(!pendingDeactivations() && _shouldFlush) {
    _shouldFlush = false;
  }
}

// ================ HELPER FUNCTIONS

void Chronomix::scheduleCoilActivation(byte coil, bool forced) {
  Activation thisActivation = {coil, forced};
  actionQueue.push(thisActivation);
}

void Chronomix::scheduleCoilDeactivation(byte coil) {
  Deactivation thisDeactivation = {coil, millis() + _coilTime};
  deactivationQueue.push(thisDeactivation);
}

bool Chronomix::coilsFull() {
  return deactivationQueue.count() >= _maxCoils;
}

bool Chronomix::pendingActions() {
  return actionQueue.count() > 0;
}

bool Chronomix::pendingDeactivations() {
  return deactivationQueue.count() > 0;
}

void Chronomix::activateCoil(Activation activation) {
  //Move these back in when debug is over
  if(shouldMoveCoil(activation.coil) || activation.forced) {
    int segment = segmentForCoil(activation.coil);
    int digit = digitForCoil(activation.coil);
    bool type = coilType(activation.coil);
    digits[digit].setSegmentOn(segment, type);
    segmentStates[digit * NUM_SEGMENTS + segment] = type;
    // Push onto deactivation queue
    scheduleCoilDeactivation(activation.coil);
  }
}

void Chronomix::deactivateCoil(byte coil) {
  int segment = segmentForCoil(coil);
  int digit = digitForCoil(coil);
  bool type = coilType(coil);
  digits[digit].setSegmentOff(segment, type);
}

int Chronomix::segmentForCoil(byte coil) {
  return (coil % 14) / 2;
}

int Chronomix::digitForCoil(byte coil) {
  return coil / 14;
}

bool Chronomix::coilType(byte coil) {
  return coil % 2 == 0;
}

bool Chronomix::shouldMoveCoil(byte coil) {
  return segmentStates[digitForCoil(coil) * NUM_SEGMENTS + segmentForCoil(coil)] != coilType(coil); //If the state is different, turn the coil
}

byte Chronomix::coilForDigitData(byte digit, byte segment, bool val) {
  //I know it's convoluted but she works
  return (digit * 14) + (segment * 2) + !val;
}

//Processes chars and spits out bytes
//RETURN OF 0 MEANS ERROR OR NONE
byte Chronomix::char2byte(char thisChar) {
    switch(thisChar) {
      case '0': return B1111101;
      case '1': return B0110000;
      case '2': return B1011011;
      case '3': return B1111010;
      case '4': return B0110110;
      case '5': return B1101110;
      case '6': return B1101111;
      case '7': return B0111000;
      case '8': return B1111111;
      case '9': return B1111110;
      case 'A': return B0111111;
      case 'b': return B1100111;
      case 'c': return B1000011;
      case 'd': return B1110011;
      case 'E': return B1001111;
      case 'F': return B0001111;
      case 'H': return B0110111;
      case 'h': return B0100111;
      case 'i': return B0000101;
      case 'I': return B0000101;
      case 'L': return B1000101;
      case 'n': return B0100011;
      case 'o': return B1100011;
      case 'O': return B1111101;
      case 'P': return B0011111;
      case 'S': return B1100110;
      case 'U': return B1110101;
      case 'u': return B1100001;
      case '-': return B0000010;
      case 'Y': return B1110110; 
      default: return B0000000;
  }
}

byte Chronomix::int2byte(int num) {
  switch(num) {
    case 0: return B1111101;
    case 1: return B0110000;
    case 2: return B1011011;
    case 3: return B1111010;
    case 4: return B0110110;
    case 5: return B1101110;
    case 6: return B1101111;
    case 7: return B0111000;
    case 8: return B1111111;
    case 9: return B1111110;
    default: return B0000000;
  }
}
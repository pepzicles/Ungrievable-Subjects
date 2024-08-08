#include "CeramicHeater.h"

CeramicHeater::CeramicHeater(int enable, int error, int temp) {
  // assign Pin values
  enablePin = enable;
  tempPin = temp;
  errorPin = error;
}

void CeramicHeater::setup(int target, int delta, int off, bool debug) {
  this->debug = debug;
  pinMode(enablePin, OUTPUT);
  pinMode(errorPin, INPUT);
  pinMode(13, OUTPUT);

#ifdef ESP32
  analogReadResolution(10);
#else
  analogReference(DEFAULT);
#endif

  setTargetTemp(target);
  setDeltaTemp(delta);
  setOffTemp(off);
}

// interpolation function
uint16_t CeramicHeater::interpolate(uint16_t x_low16, uint16_t x_hi16, int32_t y_low, int32_t y_hi, uint16_t x16) {
  //cast to 32 bits unsigned to avoid overflow
  int32_t x_low = (uint32_t)x_low16;
  int32_t x_hi = (uint32_t)x_hi16;
  int32_t x = (uint32_t)x16;
  int32_t interpolated = ((x_hi - x) * y_low + (x - x_low) * y_hi) / (x_hi - x_low);
  return (int16_t)(interpolated);
}


uint16_t CeramicHeater::getLUTValue(int index) {
#ifdef ESP32
  return tempLUT[index];
#else
  return getLUTValue(tempLUT + index);
#endif
}

// binary search for analog value in temperature lookup table
int CeramicHeater::lookupTemp(uint16_t analogValue) {
  // make sure analog value is in range 0 - 125 C
  if (analogValue < getLUTValue(0)) {
    Serial.println("*** ERROR: temp below 0 C ***");
    return (OFFSET - 1) * PRECISION;
  }
  if (analogValue > getLUTValue(LUT_LENGTH - 1)) {
    Serial.println("*** ERROR: temp above 125 C ***");
    return (OFFSET + LUT_LENGTH * UNITS + 1) * PRECISION;
  }

  // pick ends of array to start binary search
  uint16_t high = LUT_LENGTH - 1;
  uint16_t low = 0;

  while (high - low > 1) {
    // get the value in the middle of the lookup table
    uint16_t mid = (high + low) >> 1;
    uint16_t midval = getLUTValue(mid);
    // if we hit the correct value then return
    if (midval == analogValue) {
      return (mid * UNITS + OFFSET) * PRECISION;
    }
    // set new hi / low for binary search
    if (midval > analogValue) {
      high = mid;
    } else if (midval < analogValue) {
      low = mid;
    }
  }

  // interpolate between values
  int32_t low_temp = (int16_t(low * UNITS + OFFSET)) * PRECISION;
  int32_t high_temp = (int16_t(high * UNITS + OFFSET)) * PRECISION;
  return interpolate(getLUTValue(low), getLUTValue(high), low_temp, high_temp, analogValue);
}

// takes ~10ms per NUMSAMPLES
int CeramicHeater::getTemp() {
  float average = 0;
  float sum = 0;
  int value = 0;

  for (int i = 0; i < NUMSAMPLES; i++) {
    value = analogRead(tempPin);
    sum += value;
    delay(10);
  }

  average = sum / NUMSAMPLES;

  // resisitance
  // int resistance = 1023.0 / average - 1.0;
  // resistance = SERIESRESISTOR / average;
  // voltage
  float voltage = average * (VOLTAGE / 1023.0);

  int temp = lookupTemp((uint16_t)average);
  currentTemp = temp;

  if (debug) {
    Serial.print("avg analog read: ");
    Serial.println(average);
    Serial.print("voltage: ");
    Serial.print(voltage);
    Serial.print("V \ncurrent temp: ");
    Serial.print(temp);
    Serial.println("C \n\n");
  }

  return temp;
}

// turn on
int CeramicHeater::turnOn() {
  // turn on enable
  heaterState = 1;
  digitalWrite(enablePin, HIGH);
  digitalWrite(13, HIGH);

  return 1;
}

// get the error state
int CeramicHeater::getError() {
  if (digitalRead(errorPin) == LOW) {
    // set state variable to error
    heaterState = -1;
    return -1;
  } else {
    // reset state variable to 0 if we were previously in an error state
    if (heaterState == -1) {
      heaterState = 0;
    }
    return heaterState;
  }
}

// turn it off!
int CeramicHeater::turnOff() {
  heaterState = 0;
  digitalWrite(enablePin, LOW);
  digitalWrite(13, LOW);
  return 0;
}

// set the target temp
void CeramicHeater::setTargetTemp(int target) {
  targetTemp = target;
}

// get the target temp
int CeramicHeater::getTargetTemp() {
  return targetTemp;
}

// set the delta temp
void CeramicHeater::setDeltaTemp(int delta) {
  deltaTemp = delta;
}

// get the delta temp
int CeramicHeater::getDeltaTemp() {
  return deltaTemp;
}

// set the "off" temp
void CeramicHeater::setOffTemp(int off) {
  offTemp = off;
}

void CeramicHeater::setCycleCount(int count) {
  cycleCount = count;
}

void CeramicHeater::incrementCycleCount() {
  cycleCount++;
}

int CeramicHeater::getCycleCount() {
  return cycleCount;
}

// get the "off" temp
int CeramicHeater::getOffTemp() {
  return offTemp;
}

// get the state
int CeramicHeater::getState() {
  return heaterState;
}

void CeramicHeater::run() {
  if (debug) {
    Serial.print("target temp: ");
    Serial.println(targetTemp);
    Serial.print("current state: ");
    Serial.println(getState());
  }

  if (getError() >= 0) {
    int currentTemp = getTemp();
    if (currentTemp >= targetTemp + deltaTemp / 2) {
      turnOff();
    } else if (currentTemp <= targetTemp - deltaTemp / 2 - deltaTemp % 2) {
      turnOn();
    }
  } else {
    Serial.println("** ERROR DETECTED **");
  }
}

// heats up to target temp
// cools off to "off temp"
// assume starting from "cool state"
bool CeramicHeater::cycle() {

  // start in the on state;
  CycleState state = TURNON;
  CycleState lastState = TURNON;

  while (true) {

    if (debug) {
      Serial.print("cycle count: ");
      Serial.println(getCycleCount());
      Serial.print("target temp: ");
      Serial.println(targetTemp);
      Serial.print("current heater state: ");
      Serial.println(getState());
    }

    if (getError() >= 0) {
      int currentTemp = getTemp();

      if (state == TURNON) {
        // if element is cool, turn on
        if (currentTemp <= offTemp) {
          turnOn();
          if (debug) {
            Serial.println("TURNON: turning on");
          }
          state = HEATING;
          lastState = TURNON;
        } else {
          state = COOLING;
          lastState = TURNON;
        }
        // lastState = TURNON;
      } else if (state == HEATING) {
        // if element is hot, turn off
        if (currentTemp >= targetTemp + deltaTemp / 2) {
          turnOff();
          state = COOLING;
          lastState = HEATING;
        } else {
          if (debug) {
            Serial.println("HEATING: heating up");
          }
          state = HEATING;
        }
        // lastState = HEATING;
      } else if (state == COOLING) {
        // if we've cooled down check whether we're done
        if (currentTemp <= offTemp) {
          // if prior state was HEATING, yes all done
          if (lastState == HEATING) {
            state = DONE;
            lastState = HEATING;
            // if prior state was TURNON then need to heat
          } else if (lastState == TURNON) {
            if (debug) {
              Serial.println("COOLING: transistioned from COOLING to TURNON");
            }
            state = TURNON;
            lastState = COOLING;
          }
          // otherwise wait around
        } else {
          if (debug) {
            Serial.println("COOLING: waiting for element to cool");
          }
          state = COOLING;
        }
      } else if (state == DONE) {
        if (debug) {
          Serial.println("DONE: cycle complete");
        }
        return true;
      }

    } else {
      Serial.println("** ERROR DETECTED **");
    }
  }

  return false;
}

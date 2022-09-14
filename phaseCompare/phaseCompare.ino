


unsigned long previousMicros = 0;        // will store last time LED was updated
float min_v = 5, max_v;
uint16_t minAnalog1 = 1023, maxAnalog1, minAnalog2 = 1023, maxAnalog2;
uint32_t prevMinMicrosTime, minMicrosTime, maxMicrosTime;
uint16_t x;
bool readFlag, periodFlag;
uint8_t upFlag, downFlag;

void setup() {
  Serial.begin(500000);
}

void loop() {
  unsigned long currentMicros = micros();
  //  x++;
  uint16_t analog1, analog2;
  uint32_t periodTime;
  if (currentMicros - previousMicros >= 1000) {
    previousMicros = currentMicros;
    readFlag = 1;
    analog1 = analogRead(A0);
    analog2 = analogRead(A1);
    //        Serial.println(analogRead(A0));
    //    Serial.println(x);
    //    x = 0;
  }

  if (readFlag) {
    readFlag = 0;

    if (analog1 < minAnalog1) {
      minAnalog1 = analog1;
      minMicrosTime = currentMicros;
      downFlag++;
      if (downFlag == 5) {
        maxAnalog1 = 0;
        if (prevMinMicrosTime) {
          periodTime = minMicrosTime - prevMinMicrosTime;
          periodFlag = 1;
        }
      }
      upFlag = 0;
    }
    else if (analog1 > maxAnalog1) {
      maxAnalog1 = analog1;
      maxMicrosTime = currentMicros;
      downFlag = 0;
      upFlag++;
      if (upFlag == 5) {
        minAnalog1 = 1023;
        prevMinMicrosTime = minMicrosTime;
      }
    }
    
//    Serial.println("v:" + (String)analog1);
//    Serial.println("down:" + (String)downFlag);
    //    Serial.println(upFlag);

    //    float analog1_v = analog1 * 5;
    //    analog1_v /= 1024;
    //    float analog2_v = analog2 * 5;
    //    analog2_v /= 1024;
    //    Serial.println(analog1_v);
    //Serial.println(analog2_v);

  }


  if (periodFlag) {
    periodFlag = 0;
        Serial.println("T=" + (String)periodTime);
        Serial.println("f=" + String(1000000 / periodTime));
  }

}

void maxMinCal() {

}

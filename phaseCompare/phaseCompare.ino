//  ตั้งค่า pin ของเข้าของสัญญาณ
#define CH1 A0 // v grid
#define CH2 A1 // v gen
#define CH3 A2 // i gen

//  ตั้งค่าอัตราส่วนแรงดันจริงต่อแรงดันที่อ่านได้(ตัวคูณ)
#define VRATIO1 350
#define VRATIO2 350
#define IRATIO2 1

uint32_t currentMillis, prevMillisTime1, prevMillisTime2, prevMillisTime3;
uint32_t centerPhaseMillis1, centerPhaseMillis2, centerPhaseMillis3;

float f1_sum, f2_sum, f3_sum, f1, f2, f3;
uint16_t f1_index, f2_index, f3_index;
bool readFlag, periodFlag1, upFlag1, periodFlag2, upFlag2, periodFlag3, upFlag3;

uint32_t analog1Sum, analog2Sum, analog3Sum, rms1Sum, rms2Sum, rms3Sum;
uint16_t analogSample, sampleInterval = 250;
uint8_t cutState1, cutState2, cutState3;

uint16_t analog1, analog2, analog3, analog_av1 = 512, analog_av2 = 512, analog_av3 = 512;
float analog1_rms, analog2_rms, analog3_rms;

int16_t phaseShift, currentPhaseShift;
float periodShift, currentPeriodShift;
int32_t periodShiftSum, currentPeriodShiftSum;
uint16_t periodShiftIndex, currentPeriodShiftIndex;

void setup()
{

  Serial.begin(115200);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  // ------------------ อินเทอร์รัปจาก timer1 ทุก 1 millisec  |  timer 1 interupt 1kHz (1ms) ------------------
  cli(); // stop interrupts
  // set timer1 interrupt at 1kHz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1 = 0;  // initialize counter value to 0;
  // set timer count for 1khz increments
  OCR1A = 1999; // = (16*10^6) / (1000*8) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei(); // allow interrupts
  // END TIMER SETUP
}

void loop()
{

  uint32_t periodTime1, periodTime2, periodTime3;

  // --------- เฉลี่ยค่าจาก ADC ---------
  if (analogSample >= sampleInterval)
  {
    // คำนวณ Phase shift
    periodShift = (float)periodShiftSum / periodShiftIndex;
    float T1 = 1000 / f1;
    int8_t timeDivider = periodShift / T1;
    // modulo periodShift
    periodShift -= timeDivider * T1;
    phaseShift = (float)periodShift * f1 * 360 / 1000;
    phaseShift = phaseShift % 360;
    if (phaseShift >= 180)
    {
      phaseShift += -360;
    }
    Serial.print("V1 V2 PhaseShift:" + String(phaseShift) + "deg");
    Serial.print("(" + String(periodShift) + "ms)");
    periodShiftSum = 0;
    periodShiftIndex = 0;

    // คำนวณ Current Phase shift
    currentPeriodShift = (float)currentPeriodShiftSum / currentPeriodShiftIndex;
    float T3 = 1000 / f3;
    int8_t currentTimeDivider = currentPeriodShift / T3;
    // modulo current periodShift
    currentPeriodShift -= currentTimeDivider * T3;
    currentPhaseShift = (float)currentPeriodShift * f3 * 360 / 1000;
    currentPhaseShift = currentPhaseShift % 360;
    if (currentPhaseShift >= 180)
    {
      currentPhaseShift += -360;
    }
    Serial.print("\tCurrent Phase Shift:" + String(currentPhaseShift) + "deg");
    Serial.println("(" + String(currentPeriodShift) + "ms)");
    currentPeriodShiftSum = 0;
    currentPeriodShiftIndex = 0;

    // เฉลี่ยค่าจาก ADC
    analog_av1 = analog1Sum / analogSample;
    analog_av2 = analog2Sum / analogSample;
    analog_av3 = analog3Sum / analogSample;

    // คำนวณ Vrms
    analog1_rms = rms1Sum / analogSample;
    analog1_rms = sqrt(analog1_rms);
    analog2_rms = rms2Sum / analogSample;
    analog2_rms = sqrt(analog2_rms);
    analog3_rms = rms3Sum / analogSample;
    analog3_rms = sqrt(analog3_rms);

    // เคลียร์ค่า
    analog1Sum = 0;
    analog2Sum = 0;
    analog3Sum = 0;
    rms1Sum = 0;
    rms2Sum = 0;
    rms3Sum = 0;
    analogSample = 0;

    // คำนวณความถี่1
    f1 = f1_sum / f1_index;
    f1_sum = 0;
    f1_index = 0;
    Serial.print("F1:" + String(f1, 1) + "Hz ");

    // คำนวณความถี่2
    f2 = f2_sum / f2_index;
    f2_sum = 0;
    f2_index = 0;
    Serial.print("\tF2:" + String(f2, 1) + "Hz ");

    // คำนวณความถี่3
    f3 = f3_sum / f3_index;
    f3_sum = 0;
    f3_index = 0;
    Serial.println("\tF3:" + String(f3, 1) + "Hz ");

    // Vrms เที่ยบกับ 5V
    float v1rms = analog1_rms * 5;
    v1rms /= 1024;
    v1rms *= float(VRATIO1);
    Serial.print("Vrms1:" + String(v1rms));
    float v2rms = analog2_rms * 5;
    v2rms /= 1024;
    v2rms *= float(VRATIO2);
    Serial.print("\tVrms2:" + String(v2rms));
    float v3rms = analog3_rms * 5;
    v3rms /= 1024;
    v3rms *= float(IRATIO2);
    Serial.println("\tIrms2:" + String(v3rms));

    // คำนวณ Power  S = P + jQ
    // Radians = Degrees × (π/180)
    // float rad = currentPhaseShift * 3.14 / 180;
    // // S = Vrms*Irms
    // float VA = v2rms * v3rms;
    // Serial.print("S:" + String(VA, 1) + "VA\t");
    // // P = Vrms*Irms*cos()
    // float W = v2rms * v3rms * cos(rad);
    // Serial.print("P:" + String(W, 1) + "W\t");
    // // Q = Vrms*Irms*sin()
    // float VAR = v2rms * v3rms * sin(rad);
    // Serial.println("Q:" + String(VAR, 1) + "VAR\n");

    if (f1 < 30 || f2 < 30) // ที่ความถี่ต่ำ ให้เพิ่ม sample
    {
      sampleInterval = 1000;
    }
    else // ที่ความถี่สูง ให้ลด sample
    {
      sampleInterval = 250;
    }
  }

  // ---------- เมื่อ ADC ได้ค่าใหม่แล้ว ให้คำนวณค่าต่างๆ --------
  if (readFlag)
  {
    readFlag = 0;

    // Serial.println("analog1:" + String(analog1) + "\tanalog2:" + String(analog2));

    // ----------- จับเวลาเมื่อสัญญาณตัดค่าเฉลี่ยทั้งขาขึ้นและขาลง ---------
    if (upFlag1 && cutState1)
    {
      if (analog1 < analog_av1)
      {
        upFlag1 = 0;
        cutState1++;
      }
    }
    else if (!upFlag1 && cutState1)
    {
      if (analog1 > analog_av1)
      {
        upFlag1 = 1;
        cutState1++;
      }
    }
    else if (!cutState1)
    {
      if (analog1 > analog_av1)
      {
        upFlag1 = 1;
        cutState1++;
      }
    }

    if (upFlag2 && cutState2)
    {
      if (analog2 < analog_av2)
      {
        upFlag2 = 0;
        cutState2++;
      }
    }
    else if (!upFlag2 && cutState2)
    {
      if (analog2 > analog_av2)
      {
        upFlag2 = 1;
        cutState2++;
      }
    }
    else if (!cutState2)
    {
      if (analog2 > analog_av2)
      {
        upFlag2 = 1;
        cutState2++;
      }
    }

    if (upFlag3 && cutState3)
    {
      if (analog3 < analog_av3)
      {
        upFlag3 = 0;
        cutState3++;
      }
    }
    else if (!upFlag3 && cutState3)
    {
      if (analog3 > analog_av3)
      {
        upFlag3 = 1;
        cutState3++;
      }
    }
    else if (!cutState3)
    {
      if (analog3 > analog_av3)
      {
        upFlag3 = 1;
        cutState3++;
      }
    }

    // ---------- สัญญาณ1 เมื่อตัดครบ6ครั้ง(0-5) ซึ่งเป็น 2 ความยาวคลื่น -> คำนวณคาบเวลา --------
    if (cutState1 == 5)
    {
      cutState1 = 1;
      if (prevMillisTime1)
      {
        periodFlag1 = 1;

        // คำนวณคาบเวลาของ 2ความยาวคลื่น
        periodTime1 = currentMillis - prevMillisTime1;
        periodTime1 /= 2;

        // คำนวณเวลาของสัญญาณ เพื่อไปคำนวณ Phase shift
        centerPhaseMillis1 = currentMillis + prevMillisTime1;
        centerPhaseMillis1 /= 2;
      }
      prevMillisTime1 = currentMillis;
    }

    // ---------- สัญญาณ2 เมื่อตัดครบ6ครั้ง(0-5) ซึ่งเป็น 2 ความยาวคลื่น -> คำนวณคาบเวลา --------
    if (cutState2 == 5)
    {
      cutState2 = 1;
      if (prevMillisTime1)
      {
        periodFlag2 = 1;

        // คำนวณคาบเวลาของ 2ความยาวคลื่น
        periodTime2 = currentMillis - prevMillisTime2;
        periodTime2 /= 2;

        // คำนวณเวลาของสัญญาณ เพื่อไปคำนวณ Phase shift
        centerPhaseMillis2 = currentMillis + prevMillisTime2;
        centerPhaseMillis2 /= 2;
      }
      prevMillisTime2 = currentMillis;
    }

    // ---------- สัญญาณ3 เมื่อตัดครบ6ครั้ง(0-5) ซึ่งเป็น 2 ความยาวคลื่น -> คำนวณคาบเวลา --------
    if (cutState3 == 5)
    {
      cutState3 = 1;
      if (prevMillisTime2)
      {
        periodFlag3 = 1;

        // คำนวณคาบเวลาของ 2ความยาวคลื่น
        periodTime3 = currentMillis - prevMillisTime3;
        periodTime3 /= 2;

        // คำนวณเวลาของสัญญาณ เพื่อไปคำนวณ Phase shift
        centerPhaseMillis3 = currentMillis + prevMillisTime3;
        centerPhaseMillis3 /= 2;
      }
      prevMillisTime3 = currentMillis;
    }
  }

  // เมื่อได้คาบของสัญญาณ1 -> บวกกันเพื่อคำนวณค่าเฉลี่ย
  if (periodFlag1)
  {
    periodFlag1 = 0;
    f1_sum += 1000 / periodTime1;
    f1_index++;
  }
  // เมื่อได้คาบของสัญญาณ2 -> บวกกันเพื่อคำนวณค่าเฉลี่ย
  if (periodFlag2)
  {
    periodFlag2 = 0;
    f2_sum += 1000 / periodTime2;
    f2_index++;
  }
  // เมื่อได้คาบของสัญญาณ3 -> บวกกันเพื่อคำนวณค่าเฉลี่ย
  if (periodFlag3)
  {
    periodFlag3 = 0;
    f3_sum += 1000 / periodTime3;
    f3_index++;
  }

  // ------ เทียบเวลาระหว่างสัญญาณ V1 V2  ------
  if (centerPhaseMillis1 && centerPhaseMillis2)
  {
    periodShiftSum += centerPhaseMillis1 - centerPhaseMillis2;
    periodShiftIndex++;
    centerPhaseMillis1 = 0;
    // centerPhaseMillis2 = 0;
  }
  // ------ เทียบเวลาระหว่างสัญญาณ V2 I2  ------
  if (centerPhaseMillis2 && centerPhaseMillis3)
  {
    currentPeriodShiftSum += centerPhaseMillis3 - centerPhaseMillis2;
    currentPeriodShiftIndex++;
    centerPhaseMillis3 = 0;
    // centerPhaseMillis2 = 0;
  }

  // ถ้าซิงค์ระหว่าง2สัญญาณใกล้เคียงกัน ให้แสดงหลอดไฟ
  if (phasecheck())
  {
    digitalWrite(13, HIGH);
  }
  else
  {
    digitalWrite(13, LOW);
  }
}

// --------- ทำงานทุก 1 ms (1kHz) เพื่ออ่านสัญญาณจาก ADC ---------
ISR(TIMER1_COMPA_vect)
{ // Interrupt at freq of 1kHz
  currentMillis++;
  analogSample++;
  readFlag = 1;

  // อ่านค่าสัญญาณ1
  analog1 = analogRead(CH1);
  analog1Sum += analog1;
  uint16_t analog1_diff = abs(analog_av1 - analog1);
  rms1Sum += analog1_diff * analog1_diff;

  // อ่านค่าสัญญาณ2
  analog2 = analogRead(CH2);
  analog2Sum += analog2;
  uint16_t analog2_diff = abs(analog_av2 - analog2);
  rms2Sum += analog2_diff * analog2_diff;

  // อ่านค่าสัญญาณ3 กระแส
  analog3 = analogRead(CH3);
  analog3Sum += analog3;
  uint16_t analog3_diff = abs(analog_av3 - analog3);
  rms3Sum += analog3_diff * analog3_diff;
}

bool phasecheck() // เงื่อนไขการซิงค์
{
  if (abs(f1 - f2) > 2) // ความถี่ต่างกัน
    return false;

  if (abs(phaseShift) > 5) // เฟสต่างกัน
    return false;

  if (abs(analog1_rms - analog2_rms) > 1) // Vrms ต่างกัน
    return false;

  return true;
}

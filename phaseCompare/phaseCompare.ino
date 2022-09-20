//  ตั้งค่า pin ของเข้าของสัญญาณ
#define CH1 A0
#define CH2 A1

uint32_t currentMillis, prevMillisTime1, prevMillisTime2;
uint32_t centerPhaseMillis1, centerPhaseMillis2;

float f1_sum, f2_sum, f1, f2;
uint16_t f1_index, f2_index;
bool readFlag, periodFlag1, upFlag1, periodFlag2, upFlag2;

uint32_t analog1Sum, analog2Sum, rms1Sum, rms2Sum;
uint16_t analogSample, sampleInterval = 250;
uint8_t cutState1, cutState2;

uint16_t analog1, analog2, analog_av1 = 512, analog_av2 = 512;
float analog1_rms, analog2_rms;

float periodShift;
int32_t periodShiftSum;
uint16_t periodShiftIndex;

void setup()
{

  Serial.begin(115200);

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

  uint32_t periodTime1, periodTime2;

  // --------- เฉลี่ยค่าจาก ADC ---------
  if (analogSample >= sampleInterval)
  {
    // คำนวณ Phase shift
    periodShift = (float)periodShiftSum / periodShiftIndex;
    uint8_t T1 = 1 / f1;
    int16_t phaseShift = (float)periodShift * f1 * 360 / 1000;
    phaseShift = phaseShift % 360;
    if (phaseShift >= 180)
    {
      phaseShift += -360;
    }
    Serial.print("PhaseShift:" + String(phaseShift) + "deg");
    Serial.print("(" + String(periodShift) + "ms)");
    periodShiftSum = 0;
    periodShiftIndex = 0;

    // เฉลี่ยค่าจาก ADC
    analog_av1 = analog1Sum / analogSample;
    analog_av2 = analog2Sum / analogSample;

    // คำนวณ Vrms
    analog1_rms = rms1Sum / analogSample;
    analog1_rms = sqrt(analog1_rms);
    analog2_rms = rms2Sum / analogSample;
    analog2_rms = sqrt(analog2_rms);

    // เคลียร์ค่า
    analog1Sum = 0;
    analog2Sum = 0;
    rms1Sum = 0;
    rms2Sum = 0;
    analogSample = 0;

    // คำนวณความถี่1
    f1 = f1_sum / f1_index;
    f1_sum = 0;
    f1_index = 0;
    Serial.print("\tF1:" + String(f1, 1));

    // คำนวณความถี่2
    f2 = f2_sum / f2_index;
    f2_sum = 0;
    f2_index = 0;
    Serial.print("\tF2:" + String(f2, 1));

    // Vrms เที่ยบกับ 5V
    float v1rms = analog1_rms * 5;
    v1rms /= 1024;
    Serial.print("\tVrms1:" + String(v1rms));
    float v2rms = analog2_rms * 5;
    v2rms /= 1024;
    Serial.println("\tVrms2:" + String(v2rms));

    if (f1 < 30 || f2 < 30)//ที่ความถี่ต่ำ ให้เพิ่ม sample
    {
      sampleInterval = 1000;
    }
    else //ที่ความถี่สูง ให้ลด sample
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
    else if (cutState1)
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
    else if (cutState2)
    {
      if (analog2 > analog_av2)
      {
        upFlag2 = 1;
        cutState2++;
      }
    }

    // ---------- สัญญาณ1 เมื่อตัดครบ6ครั้ง(0-5) ซึ่งเป็น 2 ความยาวคลื่น -> คำนวณคาบเวลา --------
    if (cutState1 == 5)
    {
      cutState1 = 0;
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
      cutState2 = 0;
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

    // เคลื่ยร์ flag เพื่อเริ่มจับค่าของสัญญาณที่ขาขึ้น reset to start at rising wave
    if (!cutState1 && !cutState2)
    {
      cutState1 = 1;
      cutState2 = 1;
      upFlag1 = 0;
      upFlag2 = 0;
    }
  }

  // เมื่อได้คาบของสัญญาณ1 -> บวกกันเพื่อคำนวณค่าเฉลี่ย
  if (periodFlag1)
  {
    periodFlag1 = 0;
    // Serial.print("T=" + String(periodTime1));
    // Serial.println("\tf=" + String(1000 / periodTime1));
    f1_sum += 1000 / periodTime1;
    f1_index++;
  }
  // เมื่อได้คาบของสัญญาณ2 -> บวกกันเพื่อคำนวณค่าเฉลี่ย
  if (periodFlag2)
  {
    periodFlag2 = 0;
    // Serial.print("T=" + String(periodTime1));
    // Serial.println("\tf=" + String(1000 / periodTime1));
    f2_sum += 1000 / periodTime2;
    f2_index++;
  }

  // ------ เทียบเวลาระหว่างสัญญาณทั้ง2  ------
  if (centerPhaseMillis1 && centerPhaseMillis2)
  {
    periodShiftSum += centerPhaseMillis1 - centerPhaseMillis2;
    periodShiftIndex++;
    centerPhaseMillis1 = 0;
    centerPhaseMillis2 = 0;
  }
}

// --------- ทำงานทุก 1 ms (1kHz) เพื่ออ่านสัญญาณจาก ADC ---------
ISR(TIMER1_COMPA_vect)
{ // Interrupt at freq of 1kHz
  currentMillis++;
  analogSample++;
  readFlag = 1;

  // อ่านค่าสัญญาณ1
  analog1 = analogRead(A0);
  analog1Sum += analog1;
  uint16_t analog1_diff = abs(analog_av1 - analog1);
  rms1Sum += analog1_diff * analog1_diff;

  // อ่านค่าสัญญาณ2
  analog2 = analogRead(A1);
  analog2Sum += analog2;
  uint16_t analog2_diff = abs(analog_av2 - analog2);
  rms2Sum += analog2_diff * analog2_diff;
}
float min_v = 5, max_v;
uint16_t maxAnalog1, maxAnalog2;
uint32_t currentMillis, prevMillisTime1, prevMillisTime2;
uint32_t peakMillis1, peakMillis2, buffPeakMillis1, buffPeakMillis2;
uint8_t upindex1, upindex2;

float f1_sum, f2_sum, f1, f2, f1_index, f2_index;
bool readFlag, periodFlag1, upFlag1, periodFlag2, upFlag2;

uint32_t analog1Sum, analog2Sum, rms1Sum, rms2Sum;
uint8_t analogSample;
uint8_t cutState1, cutState2;

uint16_t analog1, analog2, analog_av1 = 512, analog_av2 = 512;
float analog1_rms, analog2_rms;

uint16_t currentMicros;
int32_t periodDiff;

void setup()
{

  Serial.begin(115200);

  // TIMER SETUP- the timer interrupt allows preceise timed measurements of the reed switch
  // for mor info about configuration of arduino timers see http://arduino.cc/playground/Code/Timer1
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

  if (analogSample >= 200)
  {
    // Serial.print("\tTime diff:" + String(periodDiff) + "\t");
    analog_av1 = analog1Sum / analogSample;
    analog_av2 = analog2Sum / analogSample;

    analog1_rms = rms1Sum / analogSample;
    analog1_rms = sqrt(analog1_rms);

    analog2_rms = rms2Sum / analogSample;
    analog2_rms = sqrt(analog2_rms);

    analog1Sum = 0;
    analog2Sum = 0;
    rms1Sum = 0;
    rms2Sum = 0;
    analogSample = 0;

    f1 = f1_sum / f1_index;
    f1_sum = 0;
    f1_index = 0;
    Serial.print("F1:" + String(f1, 1));

    f2 = f2_sum / f2_index;
    f2_sum = 0;
    f2_index = 0;
    Serial.print("\tF2:" + String(f2, 1));

    float v1rms = analog1_rms * 5;
    v1rms /= 1024;
    Serial.print("\tVrms1:" + String(v1rms));

    float v2rms = analog2_rms * 5;
    v2rms /= 1024;
    Serial.println("\tVrms2:" + String(v2rms));
  }

  if (readFlag)
  {
    readFlag = 0;

    // Serial.println("analog1:" + String(analog1) + "\tanalog2:" + String(analog2));

    if (upFlag1)
    {
      if (analog1 < analog_av1)
      {
        upFlag1 = 0;
        cutState1++;
      }
    }
    else
    {
      if (analog1 > analog_av1)
      {
        upFlag1 = 1;
        cutState1++;
      }
    }

    if (upFlag2)
    {
      if (analog2 < analog_av2)
      {
        upFlag2 = 0;
        cutState2++;
      }
    }
    else
    {
      if (analog2 > analog_av2)
      {
        upFlag2 = 1;
        cutState2++;
      }
    }

    if (cutState1 == 5)
    {
      cutState1 = 1;
      if (prevMillisTime1)
      {
        periodFlag1 = 1;
        periodTime1 = currentMillis - prevMillisTime1;
        periodTime1 /= 2;
      }
      prevMillisTime1 = currentMillis;
    }

    if (cutState2 == 5)
    {
      cutState2 = 1;
      if (prevMillisTime1)
      {
        periodFlag2 = 1;
        periodTime2 = currentMillis - prevMillisTime2;
        periodTime2 /= 2;
      }
      prevMillisTime2 = currentMillis;
    }

    // ------ fine peak of sine wave to calculate phase diff ------
    // if (analog1 > maxAnalog1)
    // {
    //   maxAnalog1 = analog1;
    //   buffPeakMillis1 = currentMillis;
    // }
    // else if (upindex1 > 2)
    // {
    //   peakMillis1 = buffPeakMillis1;
    // }
    // else if (maxAnalog1)
    // {
    //   upindex1++;
    // }

    // if (analog2 > maxAnalog2)
    // {
    //   maxAnalog2 = analog2;
    //   buffPeakMillis2 = currentMillis;
    // }
    // else if (upindex2 > 2)
    // {
    //   peakMillis2 = buffPeakMillis2;
    // }
    // else if (maxAnalog2)
    // {
    //   upindex2++;
    // }

    // if (peakMillis1 && peakMillis2)
    // {
    //   periodDiff = peakMillis1 - peakMillis2;
    //   periodDiff = abs(periodDiff);
    //   // uint16_t degreeDiff = periodDiff * f1 * 360 / 1000;
    //   // Serial.println("mill1:" + String(peakMillis1) + "\tanalog1:" + String(maxAnalog1) + "\tmill2:" + String(peakMillis2) + "\tanalog2:" + String(maxAnalog2));
    //   // Serial.println("period 2 wave " + String(periodDiff) + " ms\tphase diff " + String(degreeDiff));

    //   // clear all variable
    //   maxAnalog1 = 0;
    //   maxAnalog2 = 0;
    //   peakMillis1 = 0;
    //   peakMillis2 = 0;
    //   upindex1 = 0;
    //   upindex2 = 0;
    // }
  }

  if (periodFlag1)
  {
    periodFlag1 = 0;
    // Serial.print("T=" + String(periodTime1));
    // Serial.println("\tf=" + String(1000 / periodTime1));
    f1_sum += 1000 / periodTime1;
    f1_index++;
  }
  if (periodFlag2)
  {
    periodFlag2 = 0;
    // Serial.print("T=" + String(periodTime1));
    // Serial.println("\tf=" + String(1000 / periodTime1));
    f2_sum += 1000 / periodTime2;
    f2_index++;
  }
}

ISR(TIMER1_COMPA_vect)
{ // Interrupt at freq of 1kHz
  currentMicros = micros() % 100000;
  currentMillis++;
  analogSample++;
  readFlag = 1;

  analog1 = analogRead(A0);
  analog1Sum += analog1;
  uint16_t analog1_diff = abs(analog_av1 - analog1);
  rms1Sum += analog1_diff * analog1_diff;

  analog2 = analogRead(A1);
  analog2Sum += analog2;
  uint16_t analog2_diff = abs(analog_av2 - analog2);
  rms2Sum += analog2_diff * analog2_diff;
}

float min_v = 5, max_v;
uint16_t minAnalog1 = 1023, maxAnalog1, minAnalog2 = 1023, maxAnalog2;
uint32_t currentMillis, prevMillisTime1, prevMillisTime2;

bool readFlag, periodFlag1, upFlag1;

uint32_t analog1Sum, analog2Sum;
uint8_t analogSample;
uint8_t cutState1, cutState2;

uint16_t analog1, analog2, analog_av1 = 512, analog_av2 = 512;

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
    analog_av1 = analog1Sum / analogSample;
    analog_av2 = analog2Sum / analogSample;
    analog1Sum = 0;
    analog2Sum = 0;
    analogSample = 0;
    // Serial.println(analog_av1);
  }

  if (readFlag)
  {
    readFlag = 0;

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
  }

  if (periodFlag1)
  {
    periodFlag1 = 0;
    Serial.print("T=" + String(periodTime1));
    Serial.println("\tf=" + String(1000 / periodTime1));
  }
}

void maxMinCal()
{
}

ISR(TIMER1_COMPA_vect)
{ // Interrupt at freq of 2kHz
  currentMillis++;
  analogSample++;
  readFlag = 1;
  analog1 = analogRead(A0);
  analog1Sum += analog1;
  analog2 = analogRead(A1);
  analog2Sum += analog2;
}

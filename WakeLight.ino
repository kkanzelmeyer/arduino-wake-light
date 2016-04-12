#include <RGBTools.h>
#include <TimerOne.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <Wire.h>

#define ALARM_PIN 2
#define TEST_PIN 3
#define ALARM_SECONDS 3600

// Variables used by interrupts
volatile uint16_t count = 0;
enum State {
  ALARM,
  SLEEP,
  CONNECTED
};
volatile State state = SLEEP;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *  LED - important to use PWM pins 
 *  controlled by timer0 / timer2
 * - - - - - - - - - - - - - - - - - - - - - - - - - */
 
RGBTools rgb(11,5,6, COMMON_CATHODE);
int red = 0;
int green = 250;
int blue = 154;

void setup() 
{
   
  // Serial
  Serial.begin(9600);
  delay(100);
  
  // Get time from RTC
  setSyncProvider(RTC.get);
  if(timeStatus() != timeSet) 
      Serial.println("Unable to sync with the RTC");
  else
      Serial.println("RTC has set the system time");  

  digitalClockDisplay();
  
  // Prepare the interrupt pin
  pinMode(ALARM_PIN, INPUT_PULLUP);
  pinMode(TEST_PIN, INPUT_PULLUP);
  
  // Timer Interrupt - every second
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(IsrTimerCounter);

}


void loop() 
{
  switch(state) 
  {
   
    case ALARM:
      
      // Reset the alarm flag
      if(RTC.alarm(ALARM_1))
        Serial.println("Alarm 1 Activated. Enabling LED");
      if(RTC.alarm(ALARM_2))
        Serial.println("Alarm 2 Activated. Enabling LED");
      
      // Display time
      digitalClockDisplay();
      
      delay(200);
      
      while(state == ALARM) 
      {
        if(digitalRead(TEST_PIN) == LOW) 
        {
          state = SLEEP;
          delay(200);
        }
      rgb.setColor(60,100,60);
      }
      Serial.println("");
    break;
    
    case SLEEP:
      sleep();
    break;
  }
}


/*
 * Alarm interrupt
 */
void IsrAlarmInterrupt() 
{
  state = ALARM;
  detachInterrupt(0);
}

/*
 *  Test pin interrupt
 */
void IsrTestInterrupt() 
{
  if(state == ALARM) {
    state = SLEEP;
  }
  if(state == SLEEP) {
    state = ALARM;
  }
  detachInterrupt(1);
}


/* 
 * Timer interrupt
 */
void IsrTimerCounter() 
{
  if(state == ALARM) 
  {
    count ++;
    
    if(count > ALARM_SECONDS) 
    {
      state = SLEEP;
      count = 0;
    }
  }
}


/*
 * Sleep function
 */
void sleep() 
{
  
  Serial.println("Getting ready for bed.");
  Serial.println("Enabling ISRs");
  attachInterrupt(digitalPinToInterrupt(ALARM_PIN), IsrAlarmInterrupt, LOW);
  delay(100);
  attachInterrupt(digitalPinToInterrupt(TEST_PIN), IsrTestInterrupt, LOW);
  delay(100);
  
  // turn off LED
  rgb.setColor(0,0,0);
  
  // Begin sleep sequence:
  Serial.println("Going to sleep now - night night.");
  Serial.println(" ");
  delay(100);
  
  // Choose our preferred sleep mode:
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  // Turn off the ADC while asleep.
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();
  
  // Set sleep enable (SE) bit:
  sleep_enable();
  
  // Enter sleep mode
  sleep_mode();
  
  // Wake up and resume normal operation
  sleep_disable();
  power_all_enable();
  delay(100);
  Serial.println("Waking up - where's the coffee?");
}

void digitalClockDisplay(void)
{
    // digital clock display of the time
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.print(' ');
    Serial.print(month());
    Serial.print('/');
    Serial.print(day());
    Serial.print('/');
    Serial.print(year()); 
    Serial.println(); 
}

void printDigits(int digits)
{
    // utility function for digital clock display: prints preceding colon and leading 0
    Serial.print(':');
    if(digits < 10)
        Serial.print('0');
    Serial.print(digits);
}

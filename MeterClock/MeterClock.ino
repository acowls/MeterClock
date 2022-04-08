#include "LedMatrixControlEx.h"
#include "RTClib.h" // https://github.com/NeiroNx/RTCLib/blob/master/src/RTClib.h
#include <NSEncoder.h>
#include <AceButton.h>
using namespace ace_button;
// fancy text from https://textfancy.com/multiline-text-art/

///// INPUTS
// Encoder with push button  switch
// Pushbutton Switch

///// OUTPUTS
// 2x Analog Meters showing Hours and Minutes
// 4x LEDs for back lighting those two Meters
// 32x8 LED matrix for display of characters and words

///// FEATUREs
// Backlighting on (for night time)
// Meter Time always on
// Toggle through timezones (no daylight saving?)
//   - Date programmed through USB
// Set Hour
// Set Minutes 

///// General Approach
// B2-LP Long Hold back button #2 to enter time / date setting mode.
//  Hour Set  B1-C > Minute Set 1C > Year Set 1C> Month Set
//  2C > cancel ... go to display 
//  2L > save new date and time 


// RTC 3231
// Defaults to:
// SCL/CLK  on A5 (pin 24)
// SCA/DATA on A4 (pin 23)
RTC_DS3231 rtc;

// NSEncoder (S1, S2, # pulses / notch)
// CLK = D3 (pin 6)
// CLK = D2 (pin 5)
NSEncoder control(2, 3, 2);

const byte MODULE_COUNT = 4;
// LedMatrixControl ( DIN, CLK, CS, # of 8x8s )
// DIN = D8 (pin 11) 
// CS  = D9 (pin 12)
// CLK = D10 (pin 13)
LedMatrixControl lc = LedMatrixControl(8, 10, 9, MODULE_COUNT);

// Diplay Meters
#define METER_HOUR   5
#define METER_MINUTE 6

// Buttons
// D4 (pin 7)
// A2 (pin 2?)
#define BUTTON1_PIN 4
#define BUTTON2_PIN A2

AceButton button1(BUTTON1_PIN);
AceButton button2(BUTTON2_PIN);

void handleEvent(AceButton*, uint8_t, uint8_t);

int currentPosInFrameBuffer = 0;
int controlPosition  = 0;

// Milliseconds between frames in animation effects
#define QUICK_STEP 15
// Maximum number of characters in the buffer
#define MAX_CHARS_IN_BUFFER 10

void setup(){
  delay(1000); // some boards reboot twice
  Serial.begin(115200);

  // Buttons use the built-in pull up register.
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  
  // set the intensity
  lc.setIntensity(3);
  lc.shutdown(0);

    if (! rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    abort();
  }

  if (rtc.lostPower()) {
    Serial.println(F("RTC lost power, let's set the time!"));
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // 15th Oct 2020 - 12:45:55
    // rtc.adjust(DateTime(2020, 10, 15, 12, 40, 55));
  }

  welcome_sequence();
}

// counter of how many times through the loop since lat mode change or input
uint16_t tick = 0;

void loop() {
  int newPos;
  
  if (control.get_Position(newPos)) {
    char posfmt[8];
    snprintf(posfmt, 8, "%d", newPos);
    Serial.println(posfmt);

    // reset the tick on input
    tick = 0;

    // call the correct handler for each event type
    if (newPos - controlPosition > 0) {
      if (handle_encoder_down(controlPosition, newPos))
        controlPosition = newPos;
      }
    else {
      if (handle_encoder_up(controlPosition, newPos))
        controlPosition = newPos;        
      }
    lc.updateDisplay();    
  }
  button1.check();
  button2.check();

  doTimer(tick);
  tick = tick + 1;
  doDisplay();
  doWakeEachHalfHour();
  delay(20);

}


/*
** The event handler for both buttons.
*/
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
    switch (eventType) {
      case AceButton::kEventClicked:
        Serial.print(F("Click on Button "));
        if (button->getPin() == BUTTON1_PIN) {
          Serial.println(F("1 encoder button")); 
          handle_button_click();
        }
        else {
          Serial.println(F("2 rear button")); 
          handle_button_rear_click();
        }
        // reset the tick on input
        tick = 0;        
        break;

      // LONG PRESS BUTTON 1  
      case AceButton::kEventLongPressed:
        Serial.print(F("Long Press on Button "));       
        
        if (button->getPin() == BUTTON1_PIN) {
          Serial.println(F("1 encoder button")); 
          handle_button_longpress();
        }
        else {
          Serial.println(F("2 rear button")); 
          handle_button_rear_longpress();
        }
        // reset the tick on input
        tick = 0;
        break;
    }
}

//      *************************
//          M O D E   B I T s
//      *************************
#define MODE_POWERUP       0x4000

#define MODE_DISPLAY       0x0100 // mask for the following :-
#define MODE_DISPLAY_HOMI  0x0101  // hours and minutes
#define MODE_DISPLAY_DATE  0x0102  // Month/Day, eg: 30/11
#define MODE_DISPLAY_YEAR  0x0104  // year,      eg: 2020
#define MODE_DISPLAY_SLEEP 0x0180  // display off

#define MODE_SET_DT        0x0200 // mask for the following :-
#define MODE_SET_DT_HOUR   0x0201
#define MODE_SET_DT_MIN    0x0202
#define MODE_SET_DT_YEAR   0x0204
#define MODE_SET_DT_MON    0x0208
#define MODE_SET_DT_DAY    0x0210

uint16_t master_mode = MODE_POWERUP;
#define ISMODE(m) (master_mode & m) == m

const TimeSpan oneDay =   TimeSpan(1, 0, 0, 0);
const TimeSpan oneHour =  TimeSpan(0, 1, 0, 0);
const TimeSpan oneMin =   TimeSpan(0, 0, 1, 0);

DateTime editableDateTime;

/*
 * set the MODE
 * use this function instead of attempting to set mode variable directly. 
 * This will reset tick counter on a mode change.
 */
void setMode(uint16_t newMode) {
  if (master_mode != newMode) {
    Serial.print(F("setMode 0x"));
    Serial.print(master_mode, HEX);
    Serial.print(F("  to  0x"));
    Serial.println(newMode, HEX);    
    master_mode = newMode;
    // reset tick counter back to zero 
    tick = 0;
  }
}

/*
 * 
 *   o-o   o-O-o  o-o  o--o  o      O  o   o 
 *   |  \    |   |     |   | |     / \  \ /  
 *   |   O   |    o-o  O--o  |    o---o  O   
 *   |  /    |       | |     |    |   |  |   
 *   o-o   o-O-o o--o  o     O---oo   o  o   
 *                                       
 * do DISPLAY
 * the correct thing based on the current mode.
 */
void doDisplay() {
  char textbuf[8];

  if (ISMODE(MODE_DISPLAY)) {
    DateTime now = rtc.now();

    setMeters(now.hour(), now.minute());
    
    if (ISMODE(MODE_DISPLAY_SLEEP)) {
      lc.clearDisplay();
      // exit without an update
      return;
    }
    else if (ISMODE(MODE_DISPLAY_HOMI))
      snprintf(textbuf, 8, "%02d:%02d", now.hour(), now.minute()); 
    else if (ISMODE(MODE_DISPLAY_DATE))
      snprintf(textbuf, 8, "%02d/%02d", now.day(), now.month()); 
    else if (ISMODE(MODE_DISPLAY_YEAR))
      snprintf(textbuf, 8, "%04d", now.year());
  }
  else if (ISMODE(MODE_SET_DT_HOUR)) {
    if (((tick / 20) % 2) == 0)
      snprintf(textbuf, 8, "%02d:%02d", editableDateTime.hour(), editableDateTime.minute());     
    else
      snprintf(textbuf, 8, "  :%02d", editableDateTime.minute());
    setMeters(editableDateTime.hour(), editableDateTime.minute());         
  }
  else if (ISMODE(MODE_SET_DT_MIN)) {
    if (((tick / 20) % 2) == 0)
      snprintf(textbuf, 8, "%02d:%02d", editableDateTime.hour(), editableDateTime.minute());     
    else
      snprintf(textbuf, 8, "%02d:  ", editableDateTime.hour());      
    setMeters(editableDateTime.hour(), editableDateTime.minute()); 
    }
  else if (ISMODE(MODE_SET_DT_DAY)) {
    if (((tick / 20) % 2) == 0)
      snprintf(textbuf, 8, "%2d/%02d", editableDateTime.day(), editableDateTime.month());     
    else
      snprintf(textbuf, 8, "  /%02d", editableDateTime.month());      
  }   
  else if (ISMODE(MODE_SET_DT_MON)) {
    if (((tick / 20) % 2) == 0)
      snprintf(textbuf, 8, "%2d/%02d", editableDateTime.day(), editableDateTime.month());     
    else
      snprintf(textbuf, 8, "%2d/  ", editableDateTime.day());      
  }
  else if (ISMODE(MODE_SET_DT_YEAR)) {
    snprintf(textbuf, 8, "Y%04d", editableDateTime.year());      
    
  }
    
  putWordInBuffer(textbuf);
  lc.updateDisplay();     
}


void doWakeEachHalfHour() {
  // wake display on the hour every hour
  if (ISMODE(MODE_DISPLAY_SLEEP)) {
    DateTime now = rtc.now();
    if (now.second() == 0 && (now.minute() == 0 || now.minute() == 30)) {
      setMode(MODE_DISPLAY_HOMI);
    }
  }
}
/**
 *   ___         __  __  
 *    |  | |\/| |_  |__) 
 *    |  | |  | |__ | \  
 *                   
 *   Actions taken based on how many times through the loop we have traversed 
 */
int doTimer(uint16_t ticks)
{
  // check if in POWERUP explicitly 
  if (ISMODE(MODE_POWERUP)) {
    if (ticks > 100)
      setMode(MODE_DISPLAY_HOMI);
  }
  // screen idle time
  else if (ISMODE(MODE_DISPLAY)) {
    if (ticks > 500)
      setMode(MODE_DISPLAY_SLEEP);
    }
  // timeout on setting the date and time
  else if (ISMODE(MODE_SET_DT)) {
    if (ticks > 500)
      setMode(MODE_DISPLAY_HOMI);
  }
}


/*
 *  __  ____ __   __      __  _  _ ___ ___  __  _  _    _     __  _  _  ___     __   __  ____  ___  ___
 * |__/ |___|__| |__/    |__] |  |  |   |  |  | |\ |    |    |  | |\ | | __    |__] |__/ |___ [__  [__ 
 * |  \ |___|  | |  \    |__] |__|  |   |  |__| | \|    |___ |__| | \| |__]    |    |  \ |___ ___] ___]
 */
void handle_button_rear_longpress() {
  char textbuf[8];
  snprintf(textbuf, 8, "hour"); 
      
  if (ISMODE(MODE_DISPLAY) || ISMODE(MODE_POWERUP)) {
    // enter date and time setting mode
    editableDateTime = rtc.now();
    setMode(MODE_SET_DT_HOUR);

    putWordInBuffer("set");
    lc.updateDisplay();
    delay(QUICK_STEP);
    putWordInBuffer("hour");
    lc.updateDisplay();
    
  } 
  else if (ISMODE(MODE_SET_DT)) {
    // end date and time setting mode. 
    rtc.adjust(editableDateTime);
    setMode(MODE_DISPLAY_HOMI);
        
    Serial.print(F("adjustment done "));
    Serial.print(editableDateTime.year());
    Serial.print(F("-"));
    Serial.print(editableDateTime.month());
    Serial.print(F("-"));
    Serial.print(editableDateTime.day());
    Serial.print(F(" "));
    Serial.print(editableDateTime.hour());
    Serial.print(F("h "));
    Serial.print(editableDateTime.minute());
    Serial.println(F("m"));
  }
  else
  {
    Serial.print(F("mode 0x"));
    Serial.print(master_mode, HEX);
    Serial.println(F(" unhandled in handle_button_rear_longpress()"));
  }
}

/*
 *     __  _  _ ___ ___  __  _  _    _     __  _  _  ___     __   __  ____  ___  ___
 *    |__] |  |  |   |  |  | |\ |    |    |  | |\ | | __    |__] |__/ |___ [__  [__ 
 *    |__] |__|  |   |  |__| | \|    |___ |__| | \| |__]    |    |  \ |___ ___] ___]
 */
void handle_button_longpress() {
  Serial.print(F("mode 0x"));
  Serial.print(master_mode, HEX);
  Serial.println(F(" unhandled in handle_button_longpress()"));
}

/*
 * handle REAR BUTTON click
 */
void handle_button_rear_click() {
  //
  handle_button_click();
  /*
    Serial.print(F("mode 0x"));
    Serial.print(master_mode, HEX);
    Serial.println(F(" unhandled in handle_button_rear_click"));
  */
}

/*
 * handle BUTTON click
 */
void handle_button_click() {
  if (ISMODE(MODE_SET_DT_HOUR)) {
    setMode(MODE_SET_DT_MIN);
  }  
  else if (ISMODE(MODE_SET_DT_MIN)) {
    setMode(MODE_SET_DT_YEAR);
  } 
  else if (ISMODE(MODE_SET_DT_YEAR)) {
    setMode(MODE_SET_DT_MON);
  } 
  else if (ISMODE(MODE_SET_DT_MON)) {
    setMode(MODE_SET_DT_DAY);
  } 
  else if (ISMODE(MODE_SET_DT_DAY)) {
    setMode(MODE_SET_DT_HOUR);
  }
  else {
    Serial.print(F("mode 0x"));
    Serial.print(master_mode, HEX);
    Serial.println(F(" unhandled in handle_button_click()"));
  }
}
                                                                       
                                                                        

/**
 *  _____               _            __ __ _____ 
 * |   __|___ ___ ___ _| |___ ___   |  |  |  _  |
 * |   __|   |  _| . | . | -_|  _|  |  |  |   __|
 * |_____|_|_|___|___|___|___|_|    |_____|__|   
 *
 * Handle ENCODER UP 
 * return 1 to allow click up
 */
bool handle_encoder_up(int current, int proposed) {
    /*char posfmt[8];
    snprintf(posfmt, 8, "%xu%d",master_mode, proposed); 
    Serial.println(posfmt);
    */

  if (ISMODE(MODE_DISPLAY_SLEEP)) {
    setMode(MODE_DISPLAY_HOMI);    
  }
  else if (ISMODE(MODE_DISPLAY_HOMI)) {
    setMode(MODE_DISPLAY_YEAR);    
  }
  else if (ISMODE(MODE_DISPLAY_DATE)) {
    setMode(MODE_DISPLAY_HOMI);    
  }
  else if (ISMODE(MODE_DISPLAY_YEAR)) {
    setMode(MODE_DISPLAY_DATE);    
  }
  else if (ISMODE(MODE_SET_DT_HOUR)) {
    editableDateTime = editableDateTime + oneHour; 
  }
  else if (ISMODE(MODE_SET_DT_MIN)) {
    editableDateTime = editableDateTime + oneMin;
  }
  else if (ISMODE(MODE_SET_DT_DAY)) {
    editableDateTime = editableDateTime + oneDay;
  }   
  else if (ISMODE(MODE_SET_DT_MON)) {
    int newMonth = editableDateTime.month() + 1;
    editableDateTime = DateTime(editableDateTime.year(), newMonth == 13 ? 0 : newMonth, editableDateTime.day(), editableDateTime.hour(), editableDateTime.minute(), 0);
  }
  else if (ISMODE(MODE_SET_DT_YEAR)) {
    int year = editableDateTime.year();
    if (year < 2150)
      editableDateTime = DateTime(year + 1, editableDateTime.month(), editableDateTime.day(), editableDateTime.hour(), editableDateTime.minute(), 0);
  }   
  else
  {
    Serial.print(F("mode 0x"));
    Serial.print(master_mode, HEX);
    Serial.println(F(" unhandled in handle_encoder_up()"));
  }
        
  return true;
}


/**
 *  _____               _            ____  _____ _ _ _ _____ 
 * |   __|___ ___ ___ _| |___ ___   |    \|     | | | |   | |
 * |   __|   |  _| . | . | -_|  _|  |  |  |  |  | | | | | | |
 * |_____|_|_|___|___|___|___|_|    |____/|_____|_____|_|___|
 *                                                           
 * Handle ENCODER DOWN
 * return 1 to allow click down
 */
bool handle_encoder_down(int current, int proposed) {
    /*char posfmt[8];
    snprintf(posfmt, 8, "%xd%d",master_mode, proposed); 
    Serial.println(posfmt);
    */
    
  if (ISMODE(MODE_SET_DT_HOUR)) {
    editableDateTime = editableDateTime - oneHour;
    }
  else if (ISMODE(MODE_SET_DT_MIN)) {
    editableDateTime = editableDateTime - oneMin;
    }
  else if (ISMODE(MODE_SET_DT_DAY)) {
    editableDateTime = editableDateTime - oneDay;
    }    
  else if (ISMODE(MODE_SET_DT_MON)) {
    int newMonth = editableDateTime.month() - 1;
    editableDateTime = DateTime(editableDateTime.year(), newMonth == 0 ? 12 : newMonth, editableDateTime.day(), editableDateTime.hour(), editableDateTime.minute(), 0);
    }
  else if (ISMODE(MODE_SET_DT_YEAR)) {
    int year = editableDateTime.year();
    if (year > 2020)
      editableDateTime = DateTime(year - 1, editableDateTime.month(), editableDateTime.day(), editableDateTime.hour(), editableDateTime.minute(), 0);
    }    
  else if (ISMODE(MODE_DISPLAY_SLEEP)) {
    setMode(MODE_DISPLAY_HOMI);    
  }
  else if (ISMODE(MODE_DISPLAY_HOMI)) {
    setMode(MODE_DISPLAY_DATE);    
  }
  else if (ISMODE(MODE_DISPLAY_DATE)) {
    setMode(MODE_DISPLAY_YEAR);    
  }
  else if (ISMODE(MODE_DISPLAY_YEAR)) {
    setMode(MODE_DISPLAY_HOMI);    
  }
  else    
  {    
    Serial.print(F("mode 0x"));
    Serial.print(master_mode, HEX);
    Serial.println(F(" unhandled in handle_encoder_down()"));
  }
              
  return true;
}


/*
 * set the meter displays
 */
void setMeters(uint8_t hour, uint8_t minute) {
  // HOUR Needle    0 = 0, 140 == 12 (full extent)
  // Minute Needle  0 = 0, 68  == 30 (half extent)
  
  // special case where we leave it at the right hand extent of the hour 
  if (hour == 12 && minute == 0) {
    analogWrite(METER_HOUR, 12 * 140 / 12);
  }
  else if (hour > 12) 
  {
    analogWrite(METER_HOUR, (hour - 12)  * 140 / 12);
  }
  else {
    analogWrite(METER_HOUR, hour * 140 / 12);
  }
  // set minute
  analogWrite(METER_MINUTE, minute * 68 / 30);  
}


/*
 * Welcome Sequence
 */
void welcome_sequence(){
  putWordInBuffer("Hello");
  lc.updateDisplay();

  DateTime now = rtc.now();
  char timefmt[8];
  snprintf(timefmt, 8, "%02d:%02d", now.hour(), now.minute());
  putWordInUpperBuffer(timefmt);
  annimateLedMatrixDown(500, QUICK_STEP, 500);  
}

/**
 * Animate characters in buffer to the left - slide effect. 
 */
void annimateLedMatrixLeft(int preDelay, int stepDelay, int postDelay) {
  delay(preDelay);
  if (currentPosInFrameBuffer > MODULE_COUNT * 8 - 5) {
    for (int loop = 0; loop < 6; loop++) {
      lc.shiftLeftFrameBuffer();            
      delay(stepDelay);
      lc.updateDisplay();
    }
    delay(postDelay);
  }
}

/**
 * Animate from Upper to Display buffer - window wipe down effect.
 */
void annimateLedMatrixDown(int preDelay, int stepDelay, int postDelay) {
  delay(preDelay);
  for (int loop = 0; loop < 8; loop++) {
    delay(stepDelay);
    lc.shuffleDownFrameBuffer();            
    lc.updateDisplay();
  }
  delay(postDelay);
}

/**
 * Animate from Upper to Display buffer - window wipe up effect.
 */
void annimateLedMatrixUp(int preDelay, int stepDelay, int postDelay) {
  delay(preDelay);
  for (int loop = 0; loop < 8; loop++) {
    delay(stepDelay);
    lc.shuffleUpFrameBuffer();            
    lc.updateDisplay();
  }
  delay(postDelay);
}

/**
 * Put a word in the Upper buffer
 */
void putWordInUpperBuffer(char *theWord)
{
  lc.clearUpperFrameBuffer();
  int len = strlen(theWord);
  if (len < 0 || len > MAX_CHARS_IN_BUFFER)
     return;
    
  for (int i = 0; i < len; i++)
  {
    lc.drawCharInUpperFrameBuffer(i * 6, theWord[i]);
  }
}

/**
 * Put a word in the Lower buffer
 */
void putWordInLowerBuffer(char *theWord)
{
  lc.clearLowerFrameBuffer();
  int len = strlen(theWord);
  if (len < 0 || len > MAX_CHARS_IN_BUFFER)
     return;
    
  for (int i = 0; i < len; i++)
  {
    lc.drawCharInLowerFrameBuffer(i * 6, theWord[i]);
  }
}

/**
 * Put a word in to the main display buffer
 */
void putWordInBuffer(const char *theWord)
{
  lc.clearFrameBuffer();
  int len = strlen(theWord);
  if (len < 0 || len > MAX_CHARS_IN_BUFFER)
     return;
    
  for (int i = 0; i < len; i++)
  {
    lc.drawCharInFrameBuffer(i * 6, theWord[i]);
  }
}

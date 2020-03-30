#include <SoftwareSerial.h>

/*
  ESP8266 connected to ESPserial
*/

const int timeZone = 1; // UTC+1
bool _startup = true;


const unsigned int BUTTON = 19; // pin of button
unsigned long _delay = 0; // Button continuous press delay
unsigned long _t = 0; // Next time jump
unsigned int _m = 45; // minute displayed
unsigned int _h = 11; // hour displayed

/**
 * In german there are two ways to express the current time.
 * In the moment you plug in the power, the state of the button controlls which way is used.
 * 
 *   (IDIOM == false, btn released) => "viertel eins", "halb eins", "dreiviertel eins", "punkt eins"
 *   (IDIOM == true, btn pressed) => "viertel nach zwölf", "halb eins", "viertel vor eins", "punkt eins"
 */
const boolean IDIOM = !digitalRead(BUTTON);

// Colors in which a LED can light up.
enum color {
  BLACK = 0,
  RED = 1,
  GREEN = 2,
  YELLOW = 3,
  BLUE = 4,
  PURPLE = 5,
  AQUA = 6,
  WHITE = 7
};


/**
 * Definition of LEDs that need be be light up in order to show
 * a specific word on the word clock.
 */
// Line 1: KURZDREIVIERTELÄ (1-16)
const unsigned int kurz[] = {1, 2, 3, 4, 0};
const unsigned int dreiviertel[] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};
const unsigned int viertel[] = {9, 10, 11, 12, 13, 14, 15, 0};
// Line 2: VORNACHPUNKTHALB (17-32)
const unsigned int vor[] = {17, 18, 19, 0};
const unsigned int nach[] = {20, 21, 22, 23, 0};
const unsigned int punkt[] = {24, 25, 26, 27, 28, 0};
const unsigned int halb[] = {29, 30, 31, 32, 0};
// Line 3: ZWÖLFÜNFZWEINSAU (33-48)
const unsigned int zwolf[] = {33, 34, 35, 36, 37, 0};
const unsigned int funf[] = {37, 38, 39, 40, 0};
const unsigned int zwei[] = {41, 42, 43, 44, 0};
const unsigned int eins[] = {43, 44, 45, 46, 0};
// Line 4: SECHSIEBENEUNELF (49-64)
const unsigned int sechs[] = {49, 50, 51, 52, 53, 0};
const unsigned int sieben[] = {53, 54, 55, 56, 57, 58, 0};
const unsigned int neun[] = {58, 59, 60, 61, 0};
const unsigned int elf[] = {62, 63, 64, 0};
// Line 5: DREIVIERACHTZEHN (65-80)
const unsigned int drei[] = {65, 66, 67, 68, 0};
const unsigned int vier[] = {69, 70, 71, 72, 0};
const unsigned int acht[] = {73, 74, 75, 76, 0};
const unsigned int zehn[] = {77, 78, 79, 80, 0};

// Loading (0..4) Right column
const unsigned int loadingLEDS[] = {80, 64, 48, 32, 16, 0};
const unsigned int loadingColors[] = {GREEN, GREEN, YELLOW, YELLOW, RED, 0};

// Init ("ZEIT")
const unsigned int initLEDS[] = {4, 7, 10, 13, 0};

/**
 * Definition of the pins that are connected to an LED.
 * For LED n the pin:
 *   (n*4)-4 => ground
 *   (n*4)-3 => red
 *   (n*4)-2 => green
 *   (n*4)-1 => blue
 */
const unsigned int LEDMAP[] = {
  2, 3, 4, 5,     //  "1" == "K"
  3, 2, 4, 5,     //  "2" == "U"
  4, 2, 3, 5,     //  "3" == "R"
  5, 2, 3, 4,     //  "4" == "Z" INIT/ZEIT
  6, 2, 3, 4,     //  "5" == "D"
  7, 2, 3, 4,     //  "6" == "R"
  8, 2, 3, 4,     //  "7" == "E" INIT/ZEIT
  9, 2, 3, 4,     //  "8" == "I"
  10, 2, 3, 4,    //  "9" == "V"
  11, 2, 3, 4,    // "10" == "I" INIT/ZEIT
  12, 2, 3, 4,    // "11" == "E"
  14, 2, 3, 4,    // "12" == "R"
  15, 2, 3, 4,    // "13" == "T" INIT/ZEIT
  16, 2, 3, 4,    // "14" == "E"
  17, 2, 3, 4,    // "15" == "L"
  18, 2, 3, 4,    // "16" == "Ä" LOAD
  2, 6, 7, 8,     // "17" == "V"
  3, 6, 7, 8,     // "18" == "O"
  4, 6, 7, 8,     // "19" == "R"
  5, 6, 7, 8,     // "20" == "N"
  6, 5, 7, 8,     // "21" == "A"
  7, 5, 6, 8,     // "22" == "C"
  8, 5, 6, 7,     // "23" == "H"
  9, 5, 6, 7,     // "24" == "P"
  10, 5, 6, 7,    // "25" == "U"
  11, 5, 6, 7,    // "26" == "N"
  12, 5, 6, 7,    // "27" == "K"
  14, 5, 6, 7,    // "28" == "T"
  15, 5, 6, 7,    // "29" == "H"
  16, 5, 6, 7,    // "30" == "A"
  17, 5, 6, 7,    // "31" == "L"
  18, 5, 6, 7,    // "32" == "B" LOAD
  2, 9, 10, 11,   // "33" == "Z"
  3, 9, 10, 11,   // "34" == "W"
  4, 9, 10, 11,   // "35" == "Ö"
  5, 9, 10, 11,   // "36" == "L"
  6, 9, 10, 11,   // "37" == "F"
  7, 9, 10, 11,   // "38" == "U"
  8, 9, 10, 11,   // "39" == "N"
  9, 8, 10, 11,   // "40" == "F"
  10, 8, 9, 11,   // "41" == "Z"
  11, 8, 9, 10,   // "42" == "W"
  12, 8, 9, 10,   // "43" == "E"
  14, 8, 9, 10,   // "44" == "I"
  15, 8, 9, 10,   // "45" == "N"
  16, 8, 9, 10,   // "46" == "S"
  17, 8, 9, 10,   // "47" == "A"
  18, 8, 9, 10,   // "48" == "U" LOAD
  2, 12, 14, 15,  // "49" == "S"
  3, 12, 14, 15,  // "50" == "E"
  4, 12, 14, 15,  // "51" == "C"
  5, 12, 14, 15,  // "52" == "H"
  6, 12, 14, 15,  // "53" == "S"
  7, 12, 14, 15,  // "54" == "I"
  8, 12, 14, 15,  // "55" == "E"
  9, 12, 14, 15,  // "56" == "B"
  10, 12, 14, 15, // "57" == "E"
  11, 12, 14, 15, // "58" == "N"
  12, 11, 14, 15, // "59" == "E"
  14, 11, 12, 15, // "60" == "U"
  15, 11, 12, 14, // "61" == "N"
  16, 11, 12, 14, // "62" == "E"
  17, 11, 12, 14, // "63" == "L"
  18, 11, 12, 14, // "64" == "F" LOAD
  2, 16, 17, 18,  // "65" == "D"
  3, 16, 17, 18,  // "66" == "R"
  4, 16, 17, 18,  // "67" == "E"
  5, 16, 17, 18,  // "68" == "I"
  6, 16, 17, 18,  // "69" == "V"
  7, 16, 17, 18,  // "70" == "I"
  8, 16, 17, 18,  // "71" == "E"
  9, 16, 17, 18,  // "72" == "R"
  10, 16, 17, 18, // "73" == "A"
  11, 16, 17, 18, // "74" == "C"
  12, 16, 17, 18, // "75" == "H"
  14, 16, 17, 18, // "76" == "T"
  15, 16, 17, 18, // "77" == "Z"
  16, 15, 17, 18, // "78" == "E"
  17, 15, 16, 18, // "79" == "H"
  18, 15, 16, 17  // "80" == "N" LOAD
};

/** 
 * This array represents the current status of the LED screen
 * 0 means the LED is nurned of, 1-7 corresponds to a color.
 */
unsigned int _screen[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0,
  2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct date {
  uint32_t year    = 0;
  uint8_t  month   = 0; // 1..12
  uint8_t  day     = 0; // 1..31
  uint8_t  hour    = 12; // 0..23
  uint8_t  minute  = 30; // 0..60
  uint8_t  second  = 0; // 0..60
  uint8_t  weekday = 0; // 0..6
  uint8_t  summer  = 0; // 0..1
};

void serialDelay(unsigned long delay_time);
void serialFlush(unsigned long delay_time);
void serialWaitAvail(unsigned int delay_time);
bool getDate(date &dateStruct);
bool sendCommand(String cmd, int check = 10);
bool setTimezone(int timeZone);
uint8_t str2weekDay(const char* const weekDayString);
uint8_t str2month(const char* const monthString);
uint8_t str2twoDigitNumber(const char* const numberString);
uint16_t str2fourDigitNumber(const char* const numberString);
uint8_t summertime_EU(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, int8_t tzHours);
void adjustSummertime(date &currentDate);
void setNewTime(uint8_t hour, uint8_t minute);
void drawTime(void);

void drawInitPermanent(void);
void drawLoadingPermanent(int level);
void clearPermanent(void);
void drawError(void);


SoftwareSerial ESPserial(19, 13); // RX, TX
date currentDate;




/**
 * \brief Initially set all used pins of the Arduino to the desired state
 */
void setup() {
  clearPermanent();
  pinMode(BUTTON, INPUT_PULLUP);

  // initialize both serial ports:
  Serial.begin(115200);

  ESPserial.begin(9600);

  drawInitPermanent();

  serialDelay(4000);

  Serial.print("DisSLP");
  sendCommand("AT+SLEEP=0");
  
  Serial.print("ConfHN");
  sendCommand("AT+CWHOSTNAME=\"Word Clock\"");

  char state = 0;
  for (byte j = 5; j > 0 and (state != '2' and state != '3' and state != '4'); --j)
  {
    if (state != 0)
    {
      Serial.print("failed! s = ");
      Serial.println(state);
    }

    Serial.print("QrySTAT ... ");
    if (sendCommand("AT+CIPSTATUS", -100))
    {
      if (ESPserial.find("STATUS:"))
      {
        state = ESPserial.read();
      }
      else
      {
        Serial.println("NoSTAT");
      }
      // flush
      serialFlush(100);
    }
    else
      Serial.println("NoQRY");
  }
  if (state == '2' or state == '3' or state == '4')
    Serial.println("succ");

  setTimezone(timeZone);
}

/**
 * \brief Turnes on and off a LED in the defined color.
 * 
 *   This function is called by the runloop once each loop for every LED that should light up.
 *   Through the speed of the loop and switching it fr the human eye it seems as if all LEDs glow.
 *   
 *   LEDMAP is used to determine which pin of the Arduino a LED is connected to.
 * 
 * \param aLed LED that should light up.
 * \param aColor Color in which the LED should light up.
 */
void lightUp(const unsigned int aLed, const enum color aColor) {
  const unsigned int lGnd = LEDMAP[aLed * 4 - 4];
  const unsigned int lRed = LEDMAP[aLed * 4 - 3];
  const unsigned int lGreen = LEDMAP[aLed * 4 - 2];
  const unsigned int lBlue = LEDMAP[aLed * 4 - 1];
  
  pinMode(lGnd, OUTPUT);
  digitalWrite(lGnd, 0);

  pinMode(lRed, OUTPUT);
  digitalWrite(lRed, aColor & RED);
  pinMode(lRed, INPUT);

  pinMode(lGreen, OUTPUT);
  digitalWrite(lGreen, aColor & GREEN);
  pinMode(lGreen, INPUT);

  pinMode(lBlue, OUTPUT);
  digitalWrite(lBlue, aColor & BLUE);
  pinMode(lBlue, INPUT);

  pinMode(lGnd, INPUT);
}

void lightUpPermanent(const unsigned int aLed, const enum color aColor) {
  const unsigned int lGnd = LEDMAP[aLed * 4 - 4];
  const unsigned int lRed = LEDMAP[aLed * 4 - 3];
  const unsigned int lGreen = LEDMAP[aLed * 4 - 2];
  const unsigned int lBlue = LEDMAP[aLed * 4 - 1];

  pinMode(lGnd, OUTPUT);
  digitalWrite(lGnd, 0);

  if (aColor & RED)
  {
    pinMode(lRed, OUTPUT);
    digitalWrite(lRed, 1);
  }

  if (aColor & GREEN)
  {
    pinMode(lGreen, OUTPUT);
    digitalWrite(lGreen, 1);
  }

  if (aColor & BLUE)
  {
    pinMode(lBlue, OUTPUT);
    digitalWrite(lBlue, 1);
  }
}

void clearPermanent(void)
{
  for (int i = 2; i < 19; i++) {
    if (i != 13) {
      pinMode(i, INPUT);
    }
  }
}

/**
 * \brief This function modifies the state of the screen and turnd the specified LEDs to the specified color.
 * 
 *   In case the time that should be displayed has changed this function is used by nextTime() to activate the
 *   new words on the screen.
 * 
 * \param aLeds LEDs to turn to color aColor on the screen.
 * \param aColor Color to which the LED should be set.
 */
void draw(const unsigned int aLeds[], const color aColor) {
  for (int i = 0; aLeds[i] != 0; i++) {
    _screen[aLeds[i] - 1] = aColor;
  }
}

/**
 * \brief Switches the clock to the next quarter of the hour
 * 
 *   Moves the clock 1min foreward in time and draws the new time on the screen.
 *   Called either when counter _t is passed or button is pressed for adjustment.
 */
void nextTime() {
  // Push time forward by 1min
  _m = _m + 1;

  while (_m >= 60)
  {
    _h = _h + 1;
    _m = _m - 60;
  }

  _h = _h % 24;

  drawTime();
}

void setNewTime(uint8_t hour, uint8_t minute) {
  _m = minute-1; // will be incremented by nextTime()
  _h = hour;

  nextTime();
}

void drawTime(void) {
  // Erase screen
  memset(_screen, 0, sizeof _screen);

  // Randomly choose color of the words
  const unsigned int lC = random(1, 7);
  uint8_t hour = _h;

  // Draw new words
  if ( IDIOM == false ) {
    if      (_m < 15)  draw(punkt,        lC);
    else if (_m < 30) {draw(viertel,      lC); hour = _h + 1;}
    else if (_m < 45) {draw(halb,         lC); hour = _h + 1;}
    else              {draw(dreiviertel,  lC); hour = _h + 1;}
  } else {
    if      (_m < 15)  draw(punkt,    lC);
    else if (_m < 30) {draw(viertel,  lC); draw(nach, lC);}
    else if (_m < 45) {draw(halb,     lC);                hour = _h + 1;}
    else              {draw(viertel,  lC); draw(vor, lC); hour = _h + 1;}
  }

  hour = hour % 12;

  switch (hour) {
    case  1: draw(eins,   lC); break;
    case  2: draw(zwei,   lC); break;
    case  3: draw(drei,   lC); break;
    case  4: draw(vier,   lC); break;
    case  5: draw(funf,   lC); break;
    case  6: draw(sechs,  lC); break;
    case  7: draw(sieben, lC); break;
    case  8: draw(acht,   lC); break;
    case  9: draw(neun,   lC); break;
    case 10: draw(zehn,   lC); break;
    case 11: draw(elf,    lC); break;
    case  0: draw(zwolf,  lC); break;
  }
}

void drawInitPermanent(void)
{
  int i = 0;
  while (initLEDS[i] != 0)
  {
    lightUpPermanent(initLEDS[i], AQUA);
    ++i;
  }
}

void drawLoadingPermanent(int level)
{
  int i = 0;
  // clear loading leds
  while (loadingLEDS[i] != 0)
  {
    lightUp(loadingLEDS[i], BLACK);
    ++i;
  }
  // set appropriate ones according to selected level
  while (level >= 0)
  {
    lightUpPermanent(loadingLEDS[level], loadingColors[level]);
    --level;
  }
}

void drawError(void)
{
  int i;
  for (int times = 0; times < 5; ++times)
  {
    // clear loading leds
    i = 0;
    while (loadingLEDS[i] != 0)
    {
      lightUp(loadingLEDS[i], BLACK);
      ++i;
    }
    delay(100);
    // set them to red
    i = 0;
    while (loadingLEDS[i] != 0)
    {
      lightUpPermanent(loadingLEDS[i], RED);
      ++i;
    }
    delay(200);
  }
  clearPermanent();
}


/**
 * \brief Runloop of the clock
 * 
 *   After the setup() function completed, this function is called over and over again.
 *   It allows reacting on the current state of the clock and allows us to light up the LEDs.
 */
void loop() {
  // Adjust clock time if button is pressed
  if (digitalRead(BUTTON) == false && _delay < millis()) {
    _m = _m + 4;
    nextTime();
    _t = millis() + 60000;
    _delay = millis() + 500;
  }

  // Move to the next quarter of the hour if the conter _t has been passed
  if (millis() > _t) {
    bool autoTime = false;
    if (_startup or (_h == 4 and _m < 2))
    {
      _startup = false;
      clearPermanent();
      drawLoadingPermanent(0);
      autoTime = getDate(currentDate);
      for (byte i = 4; i > 0 and currentDate.year == 1970; --i)
      {
        drawLoadingPermanent(5-i);
        autoTime = getDate(currentDate);
      }
      clearPermanent();
      if (currentDate.year == 1970)
      {
        drawError();
      }
    }
    if (autoTime)
    {

      adjustSummertime(currentDate);

      uint8_t hour = currentDate.hour;
      uint8_t minute = currentDate.minute;
      uint8_t second = currentDate.second;

      Serial.print("Time: ");
      Serial.print(hour);
      Serial.print(":");
      Serial.print(minute);
      Serial.print(":");
      Serial.println(second);
      
      // forward 7 minutes, so displayed time is closer to the actual one
      minute += 7;
      if (minute >= 60)
      {
        hour += 1;
        minute -= 60;
      }
      setNewTime(hour, minute);
    }
    else
    {
      nextTime();
    }
    _t = _t + 60000;
  }

  // Lightup all LEDs of the screen
  for (int led = 1; led < 81 ; led++) {
    if (_screen[led - 1] > 0 ) {
      lightUp(led, _screen[led - 1]);
    }
  }
}


void serialFlush(unsigned long delay_time)
{
  for (; delay_time > 0; --delay_time)
  {
    while (ESPserial.available())
    {
      ESPserial.read();
    }
    delay(1);
  }
  while (ESPserial.available())
  {
    ESPserial.read();
  }
}

void serialDelay(unsigned long delay_time)
{
  for (; delay_time > 0; --delay_time)
  {
    while (ESPserial.available())
    {
      Serial.write(ESPserial.read());
    }
    delay(1);
  }
  while (ESPserial.available())
  {
    Serial.write(ESPserial.read());
  }
}

bool getDate(date &dateStruct)
{
  Serial.print("Gd ... ");
  if (sendCommand("AT+CIPSNTPTIME?", -500))
  {
    delay(900);
    if (ESPserial.find("+CIPSNTPTIME:"))
    {
      // 'Mon Jan 06 13:16:09 2020'
      char dateString[24];
      for (byte i = 0; i < 24; ++i)
      {
        dateString[i] = ESPserial.read();
      }
      // flush
      serialFlush(100);

      // decode 
      // format: 'Mon Jan 06 13:16:09 2020'
      dateStruct.weekday = str2weekDay(         &dateString[0]);
      dateStruct.month   = str2month(           &dateString[4]);
      dateStruct.day     = str2twoDigitNumber(  &dateString[8]);
      dateStruct.hour    = str2twoDigitNumber(  &dateString[11]);
      dateStruct.minute  = str2twoDigitNumber(  &dateString[14]);
      dateStruct.second  = str2twoDigitNumber(  &dateString[17])+1;
      dateStruct.year    = str2fourDigitNumber( &dateString[20]);

      if (dateStruct.year == 1970)
      {
        Serial.println("1970, retry");
        return false;
      }
      else
      {
        Serial.println("succ");
        return true;
      }
    }
    else
    {
      Serial.println("NoANS");
      return false;
    }
  }
  Serial.println("fail");
  return false;
}

bool sendCommand(String cmd, int check)
{
  if (check >= 0)
    Serial.print(" ... ");

  ESPserial.println(cmd);
  if (check >= 0) {
    serialWaitAvail(check);
    if (ESPserial.find("OK"))
    {
      Serial.println("succ");
      return true;
    }
    else
    {
      Serial.println("fail");
      return false;
    }
  }
  else
  {
    serialWaitAvail(-check);
    // wait for transmission to mostly finish
    delay(100);
    return true;
  }
}

bool setTimezone(int timeZone)
{
  char buf[24];
  sprintf(buf, "AT+CIPSNTPCFG=1,%i", timeZone);
  
  Serial.print("SetTZ UTC");
  if (timeZone < 0)
    Serial.print(timeZone);
  else
  {
    Serial.print("+");
    Serial.print(timeZone);
  }
  return sendCommand(buf);
}

void serialWaitAvail(unsigned int delay_time)
{
  while ((delay_time-- > 0) and !ESPserial.available())
    {
      delay(1);
    }
}

uint8_t str2weekDay(const char* const weekDayString)
{
  if (strncmp(weekDayString, "Tue", 3) == 0) return 1;
  if (strncmp(weekDayString, "Wed", 3) == 0) return 2;
  if (strncmp(weekDayString, "Thu", 3) == 0) return 3;
  if (strncmp(weekDayString, "Fri", 3) == 0) return 4;
  if (strncmp(weekDayString, "Sat", 3) == 0) return 5;
  if (strncmp(weekDayString, "Sun", 3) == 0) return 6;
  return 0;
}

uint8_t str2month(const char* const monthString)
{
  if (strncmp(monthString, "Jan", 3) == 0) return 1;
  if (strncmp(monthString, "Feb", 3) == 0) return 2;
  if (strncmp(monthString, "Mar", 3) == 0) return 3;
  if (strncmp(monthString, "Apr", 3) == 0) return 4;
  if (strncmp(monthString, "May", 3) == 0) return 5;
  if (strncmp(monthString, "Jun", 3) == 0) return 6;
  if (strncmp(monthString, "Jul", 3) == 0) return 7;
  if (strncmp(monthString, "Aug", 3) == 0) return 8;
  if (strncmp(monthString, "Sep", 3) == 0) return 9;
  if (strncmp(monthString, "Oct", 3) == 0) return 10;
  if (strncmp(monthString, "Nov", 3) == 0) return 11;
  if (strncmp(monthString, "Dec", 3) == 0) return 12;
  return 0;
}

uint8_t str2twoDigitNumber(const char* const numberString)
{
  return (10 * (numberString[0]-'0')) + (numberString[1]-'0');
}

uint16_t str2fourDigitNumber(const char* const numberString)
{
  return (1000 * (numberString[0]-'0')) + (100 * (numberString[1]-'0'))
          + (10 * (numberString[2]-'0')) + (numberString[3]-'0');
}

uint8_t summertime_EU(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, int8_t tzHours)
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
{
  if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
  if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if (month==3 && (hour + 24 * day)>=(1 + tzHours + 24*(31 - (5 * year /4 + 4) % 7)) || month==10 && (hour + 24 * day)<(1 + tzHours + 24*(31 - (5 * year /4 + 1) % 7)))
    return 1;
  else
    return 0;
}

void adjustSummertime(date &currentDate)
{
  int summer = summertime_EU(currentDate.year, currentDate.month, currentDate.day, currentDate.hour, timeZone);
  if (summer != currentDate.summer)
  {
    if (summer)
    {
      Serial.println("Chng Win -> Sum");
      ++currentDate.hour;
      setTimezone(timeZone+summer);
    }
    else
    {
      Serial.println("Chng Sum -> Win");
      --currentDate.hour;
      setTimezone(timeZone);
    }
    currentDate.summer = summer;
  }
}

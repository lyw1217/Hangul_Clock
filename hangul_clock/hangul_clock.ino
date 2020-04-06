#include <RTClib.h>
#include <Adafruit_NeoPixel.h>

#define TEST

#define LED_PIN         6
#define LED_COUNT       36
#define BRIGHTNESS      80

#define BUTTON_1        2
#define BUTTON_2        3

#define BIRTH_MON       5
#define BIRTH_DAY       28
#define PERIOD          15

RTC_DS3231  rtc;
DateTime    old_now;
DateTime    now;

// 시(17, 18), 분(31, 32)은 항상 켜져있으므로, LED 2개 배치, 절반씩 할당
int hours[12][3] = { {0  ,11 , 18}, /* 열 두 시 */
                     {1  , -1, 17}, /* 한 시 */ 
                     {11 , -1, 17}, /* 두 시 */
                     {3  , -1, 17}, /* 세 시 */
                     {4  , -1, 17}, /* 네 시 */
                     {2  , 9 , 17}, /* 다 섯 시 */
                     {10 , 9 , 17}, /* 여 섯 시 */
                     {8  , 7 , 18}, /* 일 곱 시 */
                     {5  , 6 , 18}, /* 여 덟 시 */
                     {15 ,16 , 18}, /* 아 홉 시 */
                     {0  , -1, 18}, /* 열 시 */
                     {0  , 1 , 18}, /* 열 한 시 */ };

int oclock[2][3] = { {24 , -1, 25}, /* 자 정 */ 
                     {25 , -1, 26}  /* 정 오 */ };
                     
int dec_minutes[6][2]  = { { -1, -1 }, { -1, 19 }, { 23, 19 }, { 22, 19 }, { 21, 19 }, { 20, 19 } }; /* -1-1, -1십, 이십, 삼십, 사십, 오십 */
int uni_minutes[10][2] = { { -1, -1 }, { 36, 31 }, { 27, 31 }, { 28, 31 }, { 29, 31 }, { 38, 31 }, { 30, 32 }, { 35, 32 }, { 34, 32 }, { 33, 32 } }; /* -1-1, 일분, 이분, 삼분, 사분, 오분, 육분, 칠분, 팔분, 구분 */

Adafruit_NeoPixel strip = Adafruit_NeoPixel( LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800 );

int state_hour;
int state_min;

/* function declaration */
void WelcomeLED();
void SerialPrintTime();
void rainbowFade(int wait, int rainbowLoops, int start_pixel, int end_pixel);
void HappyBirthDay( DateTime _now );
void ClockLED( DateTime _now );
int HourButtonState();
int MinButtonState();

/* setup function */
void setup()
{
  Serial.begin(9600);

  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  if ( !rtc.begin() )
  {
    Serial.println( "Couldn't find RTC" );
    while (1);
  }
  
  if ( rtc.lostPower() ) 
  {
    Serial.println( "RTC lost power, lets set the time!" );
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    
    rtc.adjust( DateTime( F(__DATE__), F(__TIME__) ) );
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  old_now = rtc.now();

  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.clear();

  // welcome led shining
  rainbowFade(3, 3, 0, LED_COUNT);
}

/* loop */
void loop()
{
  now = rtc.now();
    
  state_hour = digitalRead(BUTTON_1);
  state_min  = digitalRead(BUTTON_2);
  
  if( HourButtonState() == 1 )
  {
    rtc.adjust(now+TimeSpan(0,1,0,0));
  }
  
  if( MinButtonState() == 1 )
  {
    rtc.adjust(now+TimeSpan(0,0,1,0));
  }
  
  // 매 초마다 실행
  if( old_now != now )
  {
    ClockLED( now );

    #ifdef TEST
    SerialPrintTime();
    #endif
    
    old_now = now;
  }
}


/* function definition */
void SerialPrintTime()
{
  Serial.print(now.year(), DEC);
  Serial.print('.');
  Serial.print(now.month(), DEC);
  Serial.print('.');
  Serial.print(now.day(), DEC);
  Serial.print(". ");

  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
}


// 생일 날 생일 LED가 반짝!
void HappyBirthDay()
{
  // 반짝반짝 (12-14번 LED만)
  rainbowFade(3, 10, 12, 14);
}


// 매개변수는 1초마다 바뀌는 rtc의 시간 값
void ClockLED( DateTime _now )
{
  // 현재 시간과 분을 이용해서
  // 분이 바뀔 때마다 키고 끄고 반복
  int now_hour, old_hour;
  int now_minute, old_minute;

  if( old_now.hour() != _now.hour()){
    now_hour = _now.hour();
    old_hour = old_now.hour();
    
    // 이전 시간 OFF
    strip.setPixelColor( ( hours[old_hour / 12][0] ), strip.Color( 0, 0, 0 ) );
    strip.setPixelColor( ( hours[old_hour / 12][1] ), strip.Color( 0, 0, 0 ) );
    strip.setPixelColor( ( hours[old_hour / 12][2] ), strip.Color( 0, 0, 0 ) );
    
    // 현재 시간 ON
    strip.setPixelColor( ( hours[now_hour / 12][0] ), strip.Color( 255, 255, 255 ) );
    strip.setPixelColor( ( hours[now_hour / 12][1] ), strip.Color( 255, 255, 255 ) );
    strip.setPixelColor( ( hours[now_hour / 12][2] ), strip.Color( 255, 255, 255 ) );
                     
    // 자정, 정오
    if( (now_hour == 0) || (now_hour == 24) )
    {
      strip.setPixelColor( (oclock[0][0] ), strip.Color( 255, 255, 255 ) );
      strip.setPixelColor( (oclock[0][1] ), strip.Color( 255, 255, 255 ) );
      strip.setPixelColor( (oclock[0][2] ), strip.Color( 255, 255, 255 ) );
    }
    else if( now_hour == 12 )
    {
      strip.setPixelColor( (oclock[1][0] ), strip.Color( 255, 255, 255 ) );
      strip.setPixelColor( (oclock[1][1] ), strip.Color( 255, 255, 255 ) );
      strip.setPixelColor( (oclock[1][2] ), strip.Color( 255, 255, 255 ) );
    }
    else
    {
      for(int i = 0; i < 6; i++){
        strip.setPixelColor( (oclock[i % 2][i % 3] ), strip.Color( 0, 0, 0 ) );
      }
    }

    strip.show();

    #ifdef TEST
    Serial.print("Hour Change ");
    Serial.print(hours[old_hour / 12][0]);
    Serial.print(' ');
    Serial.print(hours[old_hour / 12][1]);
    Serial.print(' ');
    Serial.print(hours[old_hour / 12][2]);
    Serial.print(" to ");
    Serial.print(hours[now_hour / 12][0]);
    Serial.print(' ');
    Serial.print(hours[now_hour / 12][1]);
    Serial.print(' ');
    Serial.println(hours[now_hour / 12][2]);
    #endif
  }

  if( old_now.minute() != _now.minute()){
    now_minute = _now.minute();
    old_minute = old_now.minute();
    
    // 이전 분 OFF
    strip.setPixelColor( ( dec_minutes[old_minute / 10][0] ), strip.Color( 0, 0, 0 ) );
    strip.setPixelColor( ( dec_minutes[old_minute / 10][1] ), strip.Color( 0, 0, 0 ) );
    strip.setPixelColor( ( uni_minutes[old_minute % 10][0] ), strip.Color( 0, 0, 0 ) );
    strip.setPixelColor( ( uni_minutes[old_minute % 10][1] ), strip.Color( 0, 0, 0 ) );
    
    // 현재 분 ON
    strip.setPixelColor( ( dec_minutes[now_minute / 10][0] ), strip.Color( 255, 255, 255 ) );
    strip.setPixelColor( ( dec_minutes[now_minute / 10][1] ), strip.Color( 255, 255, 255 ) );
    strip.setPixelColor( ( uni_minutes[now_minute % 10][0] ), strip.Color( 255, 255, 255 ) );
    strip.setPixelColor( ( uni_minutes[now_minute % 10][1] ), strip.Color( 255, 255, 255 ) );
    
    strip.show();

    #ifdef TEST
    Serial.print("Minute Change ");
    Serial.print(dec_minutes[old_minute / 10][0]);
    Serial.print(' ');
    Serial.print(dec_minutes[old_minute / 10][1]);
    Serial.print(' ');
    Serial.print(uni_minutes[old_minute % 10][0]);
    Serial.print(' ');
    Serial.print(uni_minutes[old_minute % 10][1]);
    Serial.print(" to ");
    Serial.print(dec_minutes[now_minute / 10][0]);
    Serial.print(' ');
    Serial.print(dec_minutes[now_minute / 10][1]);
    Serial.print(' ');
    Serial.print(uni_minutes[now_minute % 10][0]);
    Serial.print(' ');
    Serial.println(uni_minutes[now_minute % 10][1]);
    #endif

    // 생일 날 매 30분마다 생일축하
    if( (_now.month() == BIRTH_MON) && (_now.day() == BIRTH_DAY) && (_now.minute() / PERIOD == 0))
    {
      HappyBirthDay();
    }
  }
}


// https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/RGBWstrandtest/RGBWstrandtest.ino
// fade on = 1 , off = 0
void rainbowFade(int wait, int rainbowLoops, int start_pixel, int end_pixel) {
  int fadeVal=0, fadeMax=100;

  // Hue of first pixel runs 'rainbowLoops' complete loops through the color
  // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to rainbowLoops*65536, using steps of 256 so we
  // advance around the wheel at a decent clip.
  for(uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops*65536;
    firstPixelHue += 256) {

    for(int i=start_pixel; i<end_pixel; i++) { // For each pixel in strip...

      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      uint32_t pixelHue = firstPixelHue + (i * 65536L / 10);

      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the three-argument variant, though the
      // second value (saturation) is a constant 255.
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255,
        255 * fadeVal / fadeMax)));
    }

    strip.show();
    delay(wait);

    if(firstPixelHue < 65536) {                              // First loop,
      if(fadeVal < fadeMax) fadeVal++;                       // fade in
    } else if(firstPixelHue >= ((rainbowLoops-1) * 65536)) { // Last loop,
      if(fadeVal > 0) fadeVal--;                             // fade out
    } else {
      fadeVal = fadeMax; // Interim loop, make sure fade is at max
    }
  }
}

int HourButtonState()
{
  static int prevstate_hour = 1;
  delay(10);

  if( (state_hour == 0) && (prevstate_hour == 1))
  {
    prevstate_hour = 0;
    return 0;
  }
  else if( (state_hour == 1) && (prevstate_hour == 0) )
  {
    prevstate_hour = 1;
    return 1;
  }
  return 0;
}

int MinButtonState()
{
  static int prevstate_min = 1;
  delay(10);

  if( (state_min == 0) && (prevstate_min == 1))
  {
    prevstate_min = 0;
    return 0;
  }
  else if( (state_min == 1) && (prevstate_min == 0) )
  {
    prevstate_min = 1;
    return 1;
  }
  return 0;
}

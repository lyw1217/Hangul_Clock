#include <RTClib.h>
#include <Adafruit_NeoPixel.h>

//#define DEBUG
// #define INTERRUPT

#define LED_PIN           10
#define LED_COUNT         36
#define BRIGHTNESS        50

#define BUTTON_HOUR       2
#define BUTTON_MIN        3
#define SHORT_PUSH        300
#define LONG_PUSH         4000

#define PERIOD            15

RTC_DS3231   rtc;

DateTime     old_now;
DateTime     now;

volatile int h_state       = LOW;
volatile int m_state       = LOW;

volatile int h_short_state = LOW;
volatile int m_short_state = LOW;

volatile int h_long_state  = LOW;
volatile int m_long_state  = LOW;

volatile unsigned long h_current_high;
volatile unsigned long h_current_low;
volatile unsigned long m_current_high;
volatile unsigned long m_current_low;

int sunriseH, sunrisem, sunsetH, sunsetm;
int brightness ;

int hours[12][3] = { {0  ,11 , 17}, /* 열 두 시 */
                     {1  ,-1 , 17}, /* 한 시 */ 
                     {11 ,-1 , 17}, /* 두 시 */
                     {3  ,-1 , 17}, /* 세 시 */
                     {4  ,-1 , 17}, /* 네 시 */
                     {2  , 9 , 17}, /* 다 섯 시 */
                     {10 , 9 , 17}, /* 여 섯 시 */
                     {8  , 7 , 17}, /* 일 곱 시 */
                     {5  , 6 , 17}, /* 여 덟 시 */
                     {15 ,16 , 17}, /* 아 홉 시 */
                     {0  ,-1 , 17}, /* 열 시 */
                     {0  , 1 , 17}, /* 열 한 시 */ };

int oclock[2][3] = { {23 , -1, 24}, /* 자 정 */ 
                     {24 , -1, 25}  /* 정 오 */ };
                     
int dec_minutes[6][2]  = { { -1, -1 }, { -1, 18 }, { 22, 18 }, { 21, 18 }, { 20, 18 }, { 19, 18 } }; /* -1-1, -1십, 이십, 삼십, 사십, 오십 */
int uni_minutes[10][2] = { { -1, -1 }, { 34, 30 }, { 26, 30 }, { 27, 30 }, { 28, 30 }, { 35, 30 }, { 29, 30 }, { 33, 30 }, { 32, 30 }, { 31, 30 } }; /* -1-1, 일분, 이분, 삼분, 사분, 오분, 육분, 칠분, 팔분, 구분 */

// 음력 계산이 어려우니 하드코딩으로..
int fa_birthday[11][3] = { { 2021, 9, 22 },  { 2022, 9, 11 },  { 2023, 9, 30},  { 2024, 9, 18 },   { 2025, 10, 7 },  { 2026, 9, 26},   { 2027, 9, 16 },  { 2028, 10, 4 },  { 2029, 9, 23 },  { 2030, 9, 13 },  { 2031, 10,2 } };
int mo_birthday[11][3] = { { 2021, 1, 27 },  { 2022, 1, 17 },  { 2023, 1, 6 },  { 2024, 1, 25 },   { 2025, 1, 14 },  { 2026, 2, 2 },   { 2027, 1, 22 },  { 2028, 1, 11 },  { 2029, 1, 29 },  { 2030, 1, 18 },  { 2031, 1, 8 } };
// 양력
int other_birth[3][2]  = { { 2, 13 }, { 3, 30 }, { 12, 17 } };

Adafruit_NeoPixel strip = Adafruit_NeoPixel( LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800 );

/* function declaration */
/*
void WelcomeLED       ( );
void SerialPrintTime  ( );
void rainbowFade      ( int wait, int rainbowLoops, int start_pixel, int end_pixel );
void HappyBirthDay    ( DateTime _now );
void ClockLED         ( DateTime _now );
void HourButtonState  ( );
void MinButtonState   ( );
void ResetSecond      ( );
*/
  
/* setup function */
void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif
  
  pinMode(BUTTON_HOUR, INPUT_PULLUP);
  pinMode(BUTTON_MIN , INPUT_PULLUP);

#ifdef INTERRUPT
  attachInterrupt( digitalPinToInterrupt(BUTTON_HOUR), HourButtonState, FALLING );
  attachInterrupt( digitalPinToInterrupt(BUTTON_MIN ),  MinButtonState, FALLING );
#endif

  if ( !rtc.begin() )
  {
    Serial.println( "Couldn't find RTC" );
    while (1);
  }

  #ifdef DEBUG
  Serial.println( " RTC RESET " );
  rtc.adjust( DateTime( F(__DATE__), F(__TIME__) ) );
  #endif
  
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
  now     = rtc.now();

  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.clear();

  // welcome led shining
  rainbowFade(3, 3, 0, LED_COUNT);

  ClockLED(now);
}

/* loop */
void loop()
{
  now = rtc.now();

  int y = now.year(),
      m = now.month(),
      d = now.day(),
      h = now.hour(),
      mi= now.minute(),
      s = now.second();
  
#ifdef INTERRUPT
  // 짧게 눌렀을 때
  if( h_short_state == HIGH )
  {
    rtc.adjust( now + TimeSpan(0,1,0,0) );
    ResetSecond();
    h_short_state = LOW;
  }
  if( m_short_state == HIGH )
  {
    rtc.adjust( now + TimeSpan(0,0,1,0) );
    ResetSecond();
    m_short_state = LOW;
  }

  // 길게 눌렀을 때
  if( h_long_state == HIGH )
  {
    rtc.adjust( now + TimeSpan(0,1,0,0) );
    ResetSecond();
    h_long_state = LOW;
  }
  if( m_long_state == HIGH )
  {
    rtc.adjust( now + TimeSpan(0,0,1,0) );
    ResetSecond();
    m_long_state = LOW;
  }
#endif
  
  // 매 초마다 실행
  if( old_now != now )
  {
    sunrise_sunset(y, m, d, 127+((9/60)+(39/3600)), 37+((29/60)+(11/3600))); // 127도 9분 39초 , 37도 29분 11초

    ClockLED( now );
    
    #ifdef DEBUG
    SerialPrintTime();
    #endif
    
    old_now = now;
  }

  delay(10);
}


/* function definition */
void SerialPrintTime()
{
  Serial.print( now.year()    , DEC );
  Serial.print('.');
  Serial.print( now.month()   , DEC );
  Serial.print('.');
  Serial.print( now.day()     , DEC );
  Serial.print(". ");

  Serial.print( now.hour()    , DEC );
  Serial.print(':');
  Serial.print( now.minute()  , DEC );
  Serial.print(':');
  Serial.println( now.second(), DEC );

  Serial.print("sunrise : ");
  Serial.print( sunriseH      , DEC );
  Serial.print(":");
  Serial.println( sunrisem    , DEC );

  Serial.print("sunset : ");
  Serial.print( sunsetH       , DEC );
  Serial.print(":");
  Serial.println( sunsetm     , DEC );
}


// 생일 날 생일 LED가 반짝!
void HappyBirthDay()
{
  // 반짝반짝 (12-14번 LED만)
  rainbowFade(3, 10, 12, 15);
}


void ClockLED( DateTime _now )
{
  int now_hour  , old_hour;
  int now_minute, old_minute;
  int now_year  , now_month, now_day; // birthday

  /* hour change */
  now_hour =    _now.hour();
  old_hour = old_now.hour();
   /* minute change */
  now_minute = _now.minute();
  old_minute = old_now.minute();

  // 일출 밝기 조절
  if ( ( sunsetH >= now_hour   && now_hour   >= sunriseH )
    && ( sunsetm <  now_minute && now_minute <  sunrisem ) ) {
      brightness = 255;
  }
  else {
    // 일몰 밝기 조절
    brightness = 100;
  }
  
  // 이전 시간 OFF
  strip.setPixelColor( ( hours[old_hour % 12][0] ), strip.Color(   0,   0,   0 ) );
  strip.setPixelColor( ( hours[old_hour % 12][1] ), strip.Color(   0,   0,   0 ) );
  strip.setPixelColor( ( hours[old_hour % 12][2] ), strip.Color(   0,   0,   0 ) );
  
  // 현재 시간 ON
  strip.setPixelColor( ( hours[now_hour % 12][0] ), strip.Color( brightness, brightness, brightness ) );
  strip.setPixelColor( ( hours[now_hour % 12][1] ), strip.Color( brightness, brightness, brightness ) );
  strip.setPixelColor( ( hours[now_hour % 12][2] ), strip.Color( brightness, brightness, brightness ) );
                   
  // 자정
  if( ( (now_hour == 0) || (now_hour == 24) ) && (now_minute == 0) )
  {
    strip.setPixelColor( (oclock[0][0] ), strip.Color( brightness, brightness, brightness ) );
    strip.setPixelColor( (oclock[0][1] ), strip.Color( brightness, brightness, brightness ) );
    strip.setPixelColor( (oclock[0][2] ), strip.Color( brightness, brightness, brightness ) );
  }
  // 정오
  else if( (now_hour == 12) && (now_minute == 0) )
  {
    strip.setPixelColor( (oclock[1][0] ), strip.Color( brightness, brightness, brightness ) );
    strip.setPixelColor( (oclock[1][1] ), strip.Color( brightness, brightness, brightness ) );
    strip.setPixelColor( (oclock[1][2] ), strip.Color( brightness, brightness, brightness ) );
  }
  else
  {
    for(int i = 0; i < 6; i++){
      strip.setPixelColor( (oclock[i % 2][i % 3] ), strip.Color( 0, 0, 0 ) );
    }
  }

  strip.show();

  #ifdef DEBUG
  Serial.print("Hour Change ");
  Serial.print(hours[old_hour % 12][0]);
  Serial.print(' ');
  Serial.print(hours[old_hour % 12][1]);
  Serial.print(' ');
  Serial.print(hours[old_hour % 12][2]);
  Serial.print(" to ");
  Serial.print(hours[now_hour % 12][0]);
  Serial.print(' ');
  Serial.print(hours[now_hour % 12][1]);
  Serial.print(' ');
  Serial.println(hours[now_hour % 12][2]);
  #endif



  // 이전 분 OFF
  strip.setPixelColor( ( dec_minutes[old_minute / 10][0] ), strip.Color(   0,   0,   0 ) );
  strip.setPixelColor( ( dec_minutes[old_minute / 10][1] ), strip.Color(   0,   0,   0 ) );
  strip.setPixelColor( ( uni_minutes[old_minute % 10][0] ), strip.Color(   0,   0,   0 ) );
  strip.setPixelColor( ( uni_minutes[old_minute % 10][1] ), strip.Color(   0,   0,   0 ) );
  
  // 현재 분 ON
  strip.setPixelColor( ( dec_minutes[now_minute / 10][0] ), strip.Color( brightness, brightness, brightness ) );
  strip.setPixelColor( ( dec_minutes[now_minute / 10][1] ), strip.Color( brightness, brightness, brightness ) );
  strip.setPixelColor( ( uni_minutes[now_minute % 10][0] ), strip.Color( brightness, brightness, brightness ) );
  strip.setPixelColor( ( uni_minutes[now_minute % 10][1] ), strip.Color( brightness, brightness, brightness ) );

  if( ( now_minute != 0 ) && ( now_minute % 10 == 0 ) )
  {
    strip.setPixelColor( 30, strip.Color( brightness, brightness, brightness ) );
  }
  else if( ( now_minute == 0 ) )
  {
    strip.setPixelColor( 30, strip.Color(   0,   0,   0 ) );
  }
  
  strip.show();

  #ifdef DEBUG
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

  now_year  = _now.year();
  now_month = _now.month();
  now_day   = _now.day();

  // 가족들의 생일 날, PERIOD 분마다 생일축하
  for ( int i = 0 ; i < 11 ; i++ ) {
    #ifdef DEBUG
    Serial.print("fa_birthday[");
    Serial.print(i);
    Serial.print("][0] = ");
    Serial.println(fa_birthday[i][0]);
    Serial.print("fa_birthday[");
    Serial.print(i);
    Serial.print("][1] = ");
    Serial.println(fa_birthday[i][1]);
    Serial.print("fa_birthday[");
    Serial.print(i);
    Serial.print("][2] = ");
    Serial.println(fa_birthday[i][2]);
    Serial.print("other_birth[");
    Serial.print(i%3);
    Serial.print("][0] = ");
    Serial.println(other_birth[i%3][0]);
    Serial.print("other_birth[");
    Serial.print(i%3);
    Serial.print("][1] = ");
    Serial.println(other_birth[i%3][1]);
    #endif
    if( (now_year == fa_birthday[i][0] || now_year == mo_birthday[i][0]) ) {
      if( now_month == fa_birthday[i][1] || now_month == mo_birthday[i][1] || now_month == other_birth[i%3][0] ) {
        if ( now_day == fa_birthday[i][2] || now_day == mo_birthday[i][2] || now_day == other_birth[i%3][1] ) {
          if ( now_minute % PERIOD == 0 )
            HappyBirthDay();
        }
      }
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
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, brightness,
        brightness * fadeVal / fadeMax)));
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

  delay(100);
}

void HourButtonState()
{
  if( digitalRead(BUTTON_HOUR) == LOW )
  {
    h_current_high = millis();
    h_state        = HIGH;
    //return ;
  }

  if( ( digitalRead(BUTTON_HOUR) == HIGH ) && ( h_state == HIGH ) )
  {
    Serial.println("HOUR");
    h_current_low = millis();

    if( ( h_current_low - h_current_high ) > 1 && ( h_current_low - h_current_high ) < SHORT_PUSH )
    {
      h_short_state = HIGH;
      h_state       = LOW;
    }
    else if( ( h_current_low - h_current_high ) >= SHORT_PUSH && ( h_current_low - h_current_high ) < LONG_PUSH )
    {
      h_long_state  = HIGH;
      h_state       = LOW;
    }
  }
}

void MinButtonState()
{
  if( digitalRead(BUTTON_MIN) == LOW )
  {
    m_current_high = millis();
    m_state        = HIGH;
    //return ;
  }

  if( ( digitalRead(BUTTON_MIN) == HIGH ) && ( m_state == HIGH ) )
  {
    m_current_low = millis();

    if( ( m_current_low - m_current_high ) > 1 && ( m_current_low - m_current_high ) < SHORT_PUSH )
    {
      Serial.println("MIN");
      m_short_state = HIGH;
      m_state       = LOW;
    }
    else if( ( m_current_low - m_current_high ) >= SHORT_PUSH && ( m_current_low - m_current_high ) < LONG_PUSH )
    {
      m_long_state  = HIGH;
      m_state       = LOW;
    }
  }
}

void ResetSecond()
{
  while( now.second() != 0 )
  {
    rtc.adjust( now - TimeSpan(0,0,0,1) );
    now = rtc.now();
  }  
}


// 일출 일몰 계산
void sunrise_sunset(int y, int m, int d, double longitude, double lati) {

  double latitude  = deg2rad(lati);
  double zenith    = deg2rad(90.8333);  // 일출일몰 계산, 시민박명:96.0, 항해박명:102.0, 천문박명:108.0
  double timezone  = 9.0;               // KST +9H
  
  // 1. first calculate the day of the year
  int N = floor(275 * m / 9) - (floor((m + 9) / 12) * (1 + floor((y - 4 * floor(y / 4) + 2) / 3))) + d - 30;

  // 2. convert the longitude to hour value and calculate an approximate time
  double lhour = longitude / 15;
  
  double tr = N + ((6 - lhour) / 24.0); // sunrise
  double ts = N + ((18 - lhour) / 24.0); // sunrise

  // 3. calculate the Sun's mean anomaly
  double Mr = (0.9856 * tr) - 3.289;
  double Ms = (0.9856 * ts) - 3.289;
  
  // 4. calculate the Sun's true longitude
  // to be adjusted into the range [0,360) by adding/subtracting 360
  double Lr = Mr + (1.916*sin(deg2rad(Mr))) + (0.020*sin(deg2rad(2*Mr))) + 282.634;
  double Ls = Ms + (1.916*sin(deg2rad(Ms))) + (0.020*sin(deg2rad(2*Ms))) + 282.634;
  Lr = (Lr>=0) ? fmod(Lr, 360) : fmod(Lr, 360) + 360.0;
  Ls = (Ls>=0) ? fmod(Ls, 360) : fmod(Ls, 360) + 360.0;
  double lr = deg2rad(Lr);
  double ls = deg2rad(Ls);

  // 5a. calculate the Sun's right ascension
  // to be adjusted into the range [0,360) by adding/subtracting 360
  double RAr = rad2deg(atan(0.91764 * tan(lr)));
  double RAs = rad2deg(atan(0.91764 * tan(ls)));
  RAr = (RAr >= 0) ? fmod(RAr, 360) : fmod(RAr, 360) + 360.0;
  RAs = (RAs >= 0) ? fmod(RAs, 360) : fmod(RAs, 360) + 360.0;

  // 5b. right ascension value needs to be in the same quadrant as L
  RAr += (floor(Lr / 90.0) * 90.0) - (floor(RAr / 90.0) * 90.0);
  RAs += (floor(Ls / 90.0) * 90.0) - (floor(RAs / 90.0) * 90.0);

  // 5c. right ascension value needs to be converted into hours
  RAr /= 15;
  RAs /= 15;

  // 6. calculate the Sun's declination
  double sindecr = 0.39782 * sin(lr);
  double sindecs = 0.39782 * sin(ls);
  double cosdecr = cos(asin(sindecr));
  double cosdecs = cos(asin(sindecs));

  // 7a. calculate the Sun's local hour angle
  // (cosH> 1) the sun never rises on this location (on the specified date)
  // (cosH<-1) the sun never sets on this location (on the specified date)
  double cosHr = (cos(zenith) - (sindecr * sin(latitude))) / (cosdecr * cos(latitude));
  double cosHs = (cos(zenith) - (sindecs * sin(latitude))) / (cosdecs * cos(latitude));

  // 7b. finish calculating H and convert into hours
  double Hr = 360.0 - rad2deg(acos(cosHr));
  double Hs = rad2deg(acos(cosHs));
  Hr /= 15;
  Hs /= 15;

  // 8. calculate local mean time of rising/setting
  double Tr = Hr + RAr - (0.06571 * tr) - 6.622;
  double Ts = Hs + RAs - (0.06571 * ts) - 6.622;

  // 9. adjust back to UTC
  // to be adjusted into the range [0,24) by adding/subtracting 24
  double UTr = Tr - lhour;
  double UTs = Ts - lhour;
  UTr = (UTr >= 0) ? fmod(UTr, 24.0) : fmod(UTr, 24.0) + 24.0;
  UTs = (UTs >= 0) ? fmod(UTs, 24.0) : fmod(UTs, 24.0) + 24.0;

  // 10. convert UT value to local time zone of latitude/longitude
  double localTr = fmod(UTr + timezone, 24.0);
  double localTs = fmod(UTs + timezone, 24.0);

  // last convert localT to human time
  sunriseH = floor(localTr);
  sunrisem = (int)((localTr - sunriseH) * 60);
  sunsetH = floor(localTs);
  sunsetm = (int)((localTs - sunsetH) * 60);
}

double deg2rad(double degree) {
  return degree * PI / 180;
}

double rad2deg(double radian) {
  return radian * 180 / PI;
}

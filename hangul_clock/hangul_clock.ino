#include <RTClib.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN    6
#define NUM_LED    6

RTC_DS3231 rtc;
DateTime temp_now;

Adafruit_NeoPixel strip = Adafruit_NeoPixel( NUM_LED, LED_PIN, NEO_GRB + NEO_KHZ800 );

void setup() {
  Serial.begin(9600);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);  // 나중에 led로 알려주는 방법으로 바꾸기
  }
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    // 나중에 LED로 알려주는 방법으로 바꾸기(배터리가 부족하니 B모양을 출력한다던지..)
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  temp_now = rtc.now();

  strip.setBrightness(80);
  strip.begin();
  strip.clear();
}

int i = 0, state = 0;

void loop() {
  DateTime now = rtc.now();  
  if(temp_now != now){
    temp_now = now;
    
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
    
    if(!state){
      strip.setPixelColor(i, strip.Color(255,255,255));
      strip.show();
    }else{
      strip.setPixelColor(i, strip.Color(0,0,0));
      strip.show();
    }
    i++;
    
    if(i>=NUM_LED){
      i=0;
      state = !state;
    }
  }
  
  /*
  for(int i = 0; i<strip.numPixels(); i++){
    strip.setPixelColor(i, strip.Color(255, 255, 255));
    strip.show();
    delay(200);
  }
  
  for(int i = 0; i < NUM_LED; i++){
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
    delay(200);
  }
  */
}

/*This program puts the servo values into an array,
 reagrdless of channel number, polarity, ppm frame length, etc...
 You can even change these while scanning!*/
#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//#define SSD1306_LCDHEIGHT 64


#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
/*static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };
*/

#define PPM_LOST_SENSITIVITY 20
#define PPM_Pin 3  //this must be 2 or 3
#define multiplier (F_CPU/8000000)  //leave this alone
int ppm[16];  //array for storing up to 16 servo signals
int lost_frames = 0;
byte servo[] = {1,4,5,6,7,8,9,10};  //pin number of servo output
//#define servoOut  //comment this if you don't want servo output
#define DEBUG

void setup()
{
  Serial.begin(115200);
  Serial.println("ready");

  #if defined(servoOut)
  for(byte i=0; sizeof(servo)-1; i++) pinMode(servo[i], OUTPUT);
  #endif
 
  pinMode(PPM_Pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(PPM_Pin), read_ppm, CHANGE);

  TCCR1A = 0;  //reset timer1
  TCCR1B = 0;
  TCCR1B |= (1 << CS11);  //set timer1 to increment every 0,5 us

    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

}

void loop()
{
  lost_frames++;
  if(lost_frames < PPM_LOST_SENSITIVITY ){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Channel 1: "); display.println(ppm[0]);
    display.print("Channel 2: "); display.println(ppm[1]);
    display.print("Channel 3: "); display.println(ppm[2]);
    display.print("Channel 4: "); display.println(ppm[3]);
    display.display();
    
  }else{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("NO PPM SIGNAL");
    display.display();
  }
  delay(50);

  //You can delete everithing inside loop() and put your own code here
/*  int count;

  while(ppm[count] != 0){  //print out the servo values
    Serial.print(ppm[count]);
    Serial.print("  ");
    count++;
  }
  Serial.println("");
  delay(100);  //you can even use delays!!!

*/
}



void read_ppm(){  //leave this alone
  static unsigned int pulse;
  static unsigned long counter;
  static byte channel;
  static unsigned long last_micros;

  counter = TCNT1;
  TCNT1 = 0;

  if(counter < 610*multiplier){  //must be a pulse if less than 710us
    pulse = counter;
    #if defined(servoOut)
    if(sizeof(servo) > channel) digitalWrite(servo[channel], HIGH);
    if(sizeof(servo) >= channel && channel != 0) digitalWrite(servo[channel-1], LOW);
    #endif
  }
  else if(counter > 1910*multiplier){  //sync pulses over 1910us
    channel = 0;
    #if defined(DEBUG)
    Serial.print("PPM Frame Len: ");
    Serial.println(micros() - last_micros);
    last_micros = micros();
    #endif
  }
  else{  //servo values between 710us and 2420us will end up here
    ppm[channel] = (counter + pulse)/multiplier;
    #if defined(DEBUG)
    Serial.print(ppm[channel]);
    Serial.print("  ");
    #endif    
    channel++;
    lost_frames=0;
  }
}



/* Program for Controller Arduino
    Student Codes: 25, 1
    Last edited: 12/8/21
*/

// include libraries for RF Transceiver
#include <RH_RF69.h>
#include <SPI.h>

// include libraries for OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// define pin numbers

// RF Transceiver
int G0_RF = 2;
int SC_RF = 13;
int MISO_RF = 12;
int MOSI_RF = 11;
int CS_RF = 5;
int RST_RF = 6;


// OLED Screen
int SDA_OLED = A4;
int SCK_OLED = A5;

// Joystick
int button = 10;
int X_pot = A2;
int Y_pot = A3;
int Xpot_val;
int Ypot_val;


// initialize RF Transceiver object
RH_RF69 RfController(CS_RF, G0_RF);

// initialize OLED Screen object
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(OLED_RESET);


// enumerate states
enum States {initialize, manual, autonomous};
States myState = initialize;

void setup() {
  // begin serial communication
  Serial.begin(9600);

  // set up joystick controls
  pinMode(button, INPUT_PULLUP);
  pinMode(A2, INPUT); // X_pot , value between 0 and 5V
  pinMode(A3, INPUT); //Y-pot, value between 0 and 5V


  // set-up OLED screen
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED set-up failed");
  }
  else {
    Serial.println("OLED init successful");
  }


delay(1000);

  // reset RF Transceiver
  pinMode(RST_RF, OUTPUT);
  digitalWrite(RST_RF, LOW);
  digitalWrite(RST_RF, HIGH);
  delay(10);
  digitalWrite(RST_RF, LOW);
  delay(10);

  //initialize RF Transceiver, display if successful
  if (!RfController.init()) {
    Serial.println("failed RF init");
  }
  else {
    Serial.println("RF init successful");
  }

delay(1000);

  // set frequency and power for RF Transceiver
  // Boat B = 910 MHz
  RfController.setFrequency(910.0);
  RfController.setTxPower(16, true);


}


void loop() {
  switch(myState) {
    case initialize:
      // clear contents from display buffer
      display.clearDisplay();
      // now format our message
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      // now print the message
      display.println("init complete");
      // show the message on the OLED display
      display.display();
      delay(1000);
      
      Serial.println(myState);
      myState = manual;
      Serial.println(myState);
      break;
      
    case manual:
      // display current state
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("manual");
      display.display();
      // control motor with joystick
      Xpot_val = analogRead(A2);
      Ypot_val = analogRead(A3);
      Xpot_val = int(constrain(float(Xpot_val) * 255.0 / 1023.0, 0, 254));
      Ypot_val = int(constrain((1 - (float(Ypot_val) / 1023.0)) * 255 , 0, 254));

      /*
      Serial.print("X value: ");
      Serial.print(Xpot_val);
      Serial.print("  Y value: ");
      Serial.println(Ypot_val);

      */

      // delay(100);

      // send data to boat
      int radiopacket[] = {Ypot_val,Xpot_val};

      while(!RfController.send((uint8_t *)radiopacket, sizeof(radiopacket))) {
          RfController.waitPacketSent();
      }
       //Serial.println("Message Transmitted!");

      // change to autonomous if button pressed
      if (digitalRead(button) == LOW) {
        // transmit state change to boat
        int packet[] = {255,255};
        while (!RfController.send((uint8_t *)packet, sizeof(packet))) {
          RfController.waitPacketSent();
          Serial.println("hi");
        }
        delay(1000);
        // display change
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("switching to autonomous");
        display.display();
          
        Serial.println(myState);

        delay(500);
        myState = autonomous;
        
      }
      Serial.println("hi");
      break;
  
    case autonomous:
      // display current state
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("autonomous");
      display.display();
      
      Serial.println("hi");

      // change to manual if button pressed
      if (digitalRead(button) == LOW) {

        // transmit state change to boat
        int radiopacket[] = {127,127,1};
        while (!RfController.send((uint8_t *)radiopacket, sizeof(radiopacket))) {
          RfController.waitPacketSent();
        }

          // display state change
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("switching to manual");
          display.display();
          
        
        delay(500);
        myState = manual;
      }
      break;
  }

}

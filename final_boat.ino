#include <TinyGPS.h>

// ARDUINO SKETCH FOR BOAT

// gps libraries
// #include <TinyGPS++.h>
#include <SoftwareSerial.h>
// servos
#include <Servo.h>
// transceiver
#include <RH_RF69.h>
#include <SPI.h>



const int GPS_BAUD = 9600;

// RF transceiver PINS ----------------------------------
#define CS 5
#define RST 6
#define G0 2
#define SCK 13
#define MISO 12
#define MOSI 11

// GPS PINS ---------------------------------------------
#define Tx 3 //arduino Tx
#define Rx 4 // arduino Rx

// IMU PINS ---------------------------------------------
#define SDA A4
#define SCL A5

// Servo PINS -------------------------------------------
#define RUDDER 9
#define PROPELLER 10


// object instances -------------------------------------
//Servo
Servo rud_Servo; // rudder servo
Servo prop_Servo; // propeller servo

// GPS
TinyGPS gps;
SoftwareSerial ss(Rx, Tx);

// transceiver
RH_RF69 rf69(CS, G0);

// VARIABLES -------------------------------------
enum myStates {initialize, autonomous, rc};
myStates boatState = initialize;
uint8_t message_r[3]; // message received.array len 4: throttle, direction, state
uint8_t message_t[3]; // message transmitted
uint8_t len_message;


// HELPER METHODS -------------------------------------


void SetupServo() {
  rud_Servo.attach(RUDDER);
  prop_Servo.attach(PROPELLER);
  //rud_Servo.write(90); // 90 degrees, i.e. straight (range 0 - 180)
  
  // Electronic Speed Control (ESC) unit needs to be initialized.
  /*for (int throttle = 0; throttle < 180; throttle++) {
    prop_Servo.write(throttle);
    delay(5);
  }
  for (int throttle = 180; throttle > 0; throttle--) {
    prop_Servo.write(throttle);
    delay(5);
  }*/
  for (int direc = 0; direc < 180; direc+=5) {
    rud_Servo.write(direc);
    delay(100);
  }
}

void SetupTransceiver() {
  pinMode(RST, OUTPUT);
  digitalWrite(RST, HIGH);
  delay(10);
  digitalWrite(RST, LOW);
  delay(10);

  if(!rf69.init()) {
    Serial.println("transceiver intialization failed");
  }
  if(!rf69.setFrequency(910.0)) {
    Serial.println("set frequency failed");
  }
  rf69.setTxPower(16, true); // anywhere from 14 - 20

}

void setup() {
  Serial.begin(9600);

  // SET UP GPS ---------------------------------------------
  ss.begin(GPS_BAUD);
  // wait one second to acquire signal, then read
  delay(1000);
  /*
  while (1 < 2) {
    if (ss.available()) { // wait for first reading
      if (gps.encode(ss.read())) {
        //gps.f_get_position(&lat_origin, &lon_origin, &age_origin);
      }
    }
  }*/
  
  // SetupPins();
  SetupTransceiver();
  SetupServo();
}


void loop() {
  switch(boatState) {
    case initialize:
      boatState = rc;  
      break;
    case autonomous:
      Serial.print("autonomous");
      break;
    case rc:
      if(rf69.available()) {
        len_message = sizeof(message_r);
        if(rf69.recv(message_r, &len_message)) {
          //Serial.println(message_r[0]);
          double throttle = (double)(message_r[0] / 255.0 * 180.0);
          throttle = constrain(throttle, 0, 180);
          //Serial.println(throttle);
          double direc = (double)(message_r[2] / 255.0 * 180.0);
          
          direc = constrain(direc, 0, 180);
          for(int i = 0; i < sizeof(message_r); i++) {
            Serial.print(i);
            Serial.print(" is ");
            Serial.println(message_r[i]);
          }
          prop_Servo.write((int)throttle);
          rud_Servo.write((int)direc);
          //delay(5);
          if (message_r[0] == 255) {
            boatState = autonomous;
          }
        }
      }
      break;
  }

  
  
}

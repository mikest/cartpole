#include <Arduino.h>
#include <SPI.h>
#include <Estee_TMC5130.h>
#include <limits.h>

uint8_t TMC_CS = PIN_A5;  // Chip select, Active LOW. next to SCK/MOSI/MISO on the Adafruit Feather M0
uint8_t TMC_EN = 10;      // Drive enable pin. Active LOW. Optional, you can also just tie to ground
uint8_t BLE_CS = 8;       // Hidden pin #8 which controls the SS line for the Bluetooth module SPI comms

Estee_TMC5130 tmc = Estee_TMC5130(TMC_CS);

void setup()
{
  // init serial coms
  Serial.begin(9600);
  SPI.begin();

  // encoder
  enc_setup();

  // chip select & drive enable
  pinMode(TMC_EN, OUTPUT);
  digitalWrite(TMC_EN, HIGH); // disabled for start
  pinMode(TMC_CS, OUTPUT);
  digitalWrite(TMC_EN, HIGH);
  
  // disable BLE talking on SPI bus
#if defined(ARDUINO_SAMD_ZERO)
  pinMode(BLE_CS, OUTPUT);
  digitalWrite(BLE_CS, HIGH);
#endif

  // This sets the motor currents for HOLD & RUN, as well as the natural motor direction for positive moves
  tmc.begin(0x4, 0x4, NORMAL_MOTOR_DIRECTION);

  // drive *MUST* be disabled when testing frequency scaling
//  digitalWrite(TMC_EN, HIGH);
//  float freq = tmc.updateFrequencyScaling();
//  Serial.print("frequency scaling: ");
//  Serial.println(freq);
//  delay(5000);

  #define ACCEL 15000 * 200
  #define VELOCITY 5000 * 200

  // ramp definition
  uint32_t onerev = 200*256;
  tmc.writeRegister(XACTUAL, 0);
  tmc.writeRegister(VSTART, 0x0);
  tmc.writeRegister(A_1,  ACCEL);
  tmc.writeRegister(V_1,  VELOCITY/2);
  tmc.writeRegister(AMAX, ACCEL/2);
  tmc.writeRegister(VMAX, VELOCITY);
  tmc.writeRegister(DMAX, ACCEL/2);
  //tmc.writeRegister(V_1,  VELOCITY);
  tmc.writeRegister(D_1,  ACCEL);
  tmc.writeRegister(VSTOP, 10);
  tmc.writeRegister(TZEROWAIT, 0);
  tmc.writeRegister(RAMPMODE, 0);

  Serial.println("starting up");

  // enable the drive
  digitalWrite(TMC_EN, LOW);
}

// periodic delay
uint32_t t_echo = millis();
uint32_t t_dirchange = millis();
bool dir = false;

void loop()
{
  uint32_t now = millis();

  float enc = enc_read();

  tmc.writeRegister(XTARGET, 200*enc*4);

  // print out current position
  if( now - t_echo > 100 )
  {
    t_echo = now;

    // get the current target position
    int32_t xactual = 0, vactual = 0;
    xactual = tmc.readRegister(XACTUAL);
    vactual = tmc.readRegister(VACTUAL);

    Serial.print("xpos,v:");
    Serial.print(xactual);
    Serial.print(", ");
    Serial.print(vactual);
    Serial.print(" enc:");
    Serial.println(enc);
  }

}

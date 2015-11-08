#include <Wire.h>

#define MAG_ADDR 0x0E  // address for magnetometer

void setup(){
  Wire.begin();
  Serial.begin(115200);  // default is 9600
  startMag();
}

void startMag(){
  // Initializes magnetometer by setting some defaults on the breakout board
  // Datasheet at https://cdn.sparkfun.com/datasheets/Sensors/Magneto/MAG3110_v9.2.pdf
  
  // Enable automatic magnetic sensor reset, to guarantee mag readings are fresh
  // CTRL_REG2 uses bits 7, 5, 4, but we only want 7 turned on.
  Wire.beginTransmission(MAG_ADDR);
  Wire.send(0x11);    // CTRL_REG2
  Wire.send(0x80):    // AUTO_MRST_EN, 1000 0000 binary
  Wire.endTransmission();
  
  delay(15); // I2C requires time between transmissions
  
  // Set operating mode to ACTIVE.
  // CTRL_REG1 uses 8 bits, least sig bit is AC. Defaults to STANDBY.
  Wire.beginTransmission(MAG_ADDR);
  Wire.send(0x10);    // CTRL_REG1
  Wire.send(0x01);    // Operating mode = ACTIVE, 0000 0001 binary
  Wire.endTransmission();

  /* The magnetometer should probably also calbrate itself on startup,
  by setting offset registers.
  OUT_X_MSB is the most significant byte of a 2 byte X reading.
  OUT_X_LSB is the least significant byte.
  If CTRL_REG1's Fast Read (FR) bit is set, then burst-read will only return
    the most significant byte for each axis.
  */
}




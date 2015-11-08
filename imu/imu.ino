#include <Wire.h>

#define MAG_ADDR 0x0E  // address for magnetometer

void setup(){
  Wire.begin();
  Serial.begin(115200);  // default is 9600
  startMag();
}

void loop(){
  Serial.println( readMag() );  
}

void startMag(){
  // Initializes magnetometer by setting some defaults on the breakout board
  // Datasheet at https://cdn.sparkfun.com/datasheets/Sensors/Magneto/MAG3110_v9.2.pdf
  
  // Enable automatic magnetic sensor reset, to guarantee mag readings are fresh
  // CTRL_REG2 uses bits 7, 5, 4, but we only want 7 turned on.
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x11);    // CTRL_REG2
  Wire.write(0x80);    // AUTO_MRST_EN, 1000 0000 binary
  Wire.endTransmission();
  
  delay(15); // I2C requires time between transmissions
  
  // Set operating mode to ACTIVE.
  // CTRL_REG1 uses 8 bits, least sig bit is AC. Defaults to STANDBY.
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x10);    // CTRL_REG1
  Wire.write(0x01);    // Operating mode = ACTIVE, 0000 0001 binary
  Wire.endTransmission();

  /*
  The magnetometer should probably also calbrate itself on startup,
  by setting offset registers.  See 5.3.1 on datasheet.
    OFF_X_MSB (0x09), OFF_X_LSB (0x0A)
    OFF_Y_MSB (0x0B), OFF_Y_LSB (0x0C)
    OFF_Z_MSB (0x0D), OFF_Z_LSB (0x0E)
  
  A note about collection precision and rates.
    The magnetometer has a precision of two bytes per axis, but fast-read mode
    returns only 1 byte per axis.
      OUT_X_MSB is the most significant byte of a 2 byte X reading.
      OUT_X_LSB is the least significant byte.
  */
}

String readMag(){
  int x,y,z;
  
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x01);                  // Tells mag to prepare new data
  Wire.endTransmission();
  
  Wire.requestFrom(MAG_ADDR, 6);    // 2 bytes per x,y,z axis
  // Receieve the 6 bytes and sort them into the x,y,z axis vars
  if(6<=Wire.available()){
    x = Wire.read()<<8;             // First byte is MSB_x
    x |= Wire.read();                // Second byte is LSB_x
    y = Wire.read()<<8;
    y |= Wire.read();
    z = Wire.read()<<8;
    z |= Wire.read();
  }
  String result = String('X'+x+', '+'Y'+y+', '+'Z'+z);
  return result;
}

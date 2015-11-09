#include <Wire.h>

#define MAG_ADDR 0x0E   // address for magnetometer
#define MPU_ADDR 0x68   // address for MPU_6050

void setup(){
  Wire.begin();
  Serial.begin(115200);  // default is 9600
  scanI2C();
  startMag();
  start_6050();
}

void loop(){
  Serial.println( readMag() );
  Serial.println( readMPU() );
  Serial.println();
  delay(1000);
}

void start_6050(){
  // Initializes MPU-6050 accelerometer/gyro
  // Datasheet at https://cdn.sparkfun.com/datasheets/Components/General%20IC/PS-MPU-6000A.pdf

  // Set defaults for the MPU-6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);     // Enables PWR_MGMT_1 register
  Wire.write(0);        // Wakes up the MPU-6050
  Wire.endTransmission();
}

String readMPU(){
  // Returns a human readable string of the MPU-6050 data.
  int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, temp;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);   // Starts with ACCEL_XOUT_H register
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 14, true); // 14 registers total
  AcX = Wire.read()<<8 | Wire.read();   // 0x3B ACCEL_XOUT_H and 0x3C ACCEL_XOUT_L
  AcY = Wire.read()<<8 | Wire.read();   // 2 bytes for AcY
  AcZ = Wire.read()<<8 | Wire.read();   // 2 bytes for AcZ
  temp = Wire.read()<<8 | Wire.read();  // 2 bytes for temperature
  GyX = Wire.read()<<8 | Wire.read();   // 2 bytes for GyX
  GyY = Wire.read()<<8 | Wire.read();   // 2 bytes for GyY
  GyZ = Wire.read()<<8 | Wire.read();   // 2 bytes for GyZ
  String result = String("AcX: ")+AcX+String(", AcY: ")+AcY+String(", AcZ: ")+
    AcZ+String(", GyX: ")+GyX+String(", GyY: ")+GyY+String(", GyZ: ")+GyZ+String(", Temp: ")+temp;
  return result;
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
  // Returns a string with the x,y,z readings formatted for human eyes.
  int16_t x,y,z;
  
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x01);                  // Tells mag to prepare new data
  Wire.endTransmission();
  
  Wire.requestFrom(MAG_ADDR, 6);    // 2 bytes per x,y,z axis
  // Receieve the 6 bytes and sort them into the x,y,z axis vars
  if(6<=Wire.available()){
    x = Wire.read()<<8 | Wire.read();   // First byte is MSB_x, second is LSB_x
    y = Wire.read()<<8 | Wire.read();
    z = Wire.read()<<8 | Wire.read();
  }
  String result = String("MagX: ")+x+String(", MagY: ")+y+String(", MagZ: ")+z;
  return result;
}

void scanI2C(){
  // Displays information about connected I2C devices.
  byte error, address;
  int numDevices = 0;
  
  Serial.println("Scanning for I2C devices..");
  for(address=0; address<127; address++){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if(error == 0){
      // Device found at this address.
      Serial.print("Device found at address: 0x");
      if(address<16) Serial.print('0');
      Serial.println(address, HEX);
      numDevices++;
    }else if(error == 4){
      // Exception at this address.
      Serial.print("Unknown error at address: 0x");
      if(address<16) Serial.print('0');
      Serial.println(address, HEX);
      numDevices++;
    }
    delay(10);
  }
  if(numDevices == 0){
    Serial.println("No I2C devices found.");
  }else{
    Serial.println("Scan complete.");
  }
}

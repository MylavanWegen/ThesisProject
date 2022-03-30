void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
//--------------------------Accelerometer initialization --------------------------- //
//
//void initAccelerometer()
//{
//  busSelect(vib_bus_accelerometer);
//  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens
//
//  Serial.println("LIS3DH test!");
//
//  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
//    Serial.println("Couldnt start");
//    while (1) yield();
//  }
//  Serial.println("LIS3DH found!");
//
//  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!
//
//  Serial.print("Range = "); Serial.print(2 << lis.getRange());
//  Serial.println("G");
//
//  lis.setDataRate(LIS3DH_DATARATE_100_HZ);
//  Serial.print("Data rate set to: ");
//
//
//}
//
//
//void handleAccelerometer()
//{
//  //Get all parameters
//  busSelect(vib_bus_accelerometer);
//  lis.read();
//  dataX = lis.x;
//  dataY = lis.y;
//  dataZ = lis.z;
//
//
//}
//
//
//void visualizeData(float dataX, float dataY, float dataZ, Adafruit_LIS3DH &lis)
//{
//  Serial.print("X:  "); Serial.print(dataX);
//  Serial.print("  \tY:  "); Serial.print(dataY);
//  Serial.print("  \tZ:  "); Serial.print(dataZ);
//
//  /* Display the results (acceleration is measured in m/s^2) */
//  sensors_event_t event;
//  lis.getEvent(&event);
//
//  Serial.print("\t\tX: "); Serial.print(event.acceleration.x);
//  Serial.print(" \tY: "); Serial.print(event.acceleration.y);
//  Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
//  Serial.println(" m/s^2 ");
//
//  //Serial.println(signalStart);
//
//  delay(200);
//}

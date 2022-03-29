<<<<<<< HEAD
#include <Wire.h>
#include "DRV2605L.h"
//#include <Adafruit_LIS3DH.h>
//#include <Adafruit_Sensor.h>
//#include "SPI.h"
// hello 

// ------------------------Vibrator and Accelerometer parameters------------------------------ //
const int amnt_vibrators = 1;

DRV2605L vib[amnt_vibrators];
//Adafruit_LIS3DH lis = Adafruit_LIS3DH();                  //Default constructor is I2C, addr 0x19.

boolean     autoCal = true;                     //Autocalibration on or off
int         voltage_rms = 1.2;                  //rms voltage
int         voltage_peak = 1.9;                 //peak voltage
int         frequency = 240;                    //rated resonance frequency
char        vibMode = DRV2605_MODE_REALTIME;    //mode selection

float dataX;
float dataY;
float dataZ;

// ------------------------Serial Communication Commands-------------------- //
// Moving //
const char VIB_GO           = 'g';
const char VIB_STOP         = 's';

// Locations Vibrators ///
const char VIB_LCTN_THUMB   = 't';
const char VIB_LCTN_INDEX   = 'i';
const char VIB_LCTN_MIDDLE  = 'm';
const char VIB_LCTN_RING    = 'r';
const char VIB_LCTN_PINKY   = 'p';
const char VIB_LCTN_HANDPALM   = 'h';


// ------------------------Multiplexer--------------------------------------- //
const int vib_bus_thumb   = 0;
const int vib_bus_index   = 1;
const int vib_bus_middle  = 2;
const int vib_bus_ring    = 3;
const int vib_bus_pinky   = 4;
const int vib_bus_handpalm   = 5;

const int vib_bus_accelerometer = 7;

void busSelect(uint8_t bus) {      // Select Bus for multiplexer communication
  if (bus > 7) return;
  Wire.beginTransmission(0x70);    // TCA9548A address is 0x70
  Wire.write(1 << bus);            // send byte to select bus
  Wire.endTransmission();
}


//---------------------------Serial communication---------------------------------//
const byte  numChars = 32;
char        receivedChars[numChars];
char        tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data
char        vibLocation[numChars] = {0};
char        vibAction = 0;
int         vibEffectInt = 0;

boolean     parsed = false;
boolean     newData = false;

String      vibActionString;
String      vibLocationString;
String      vibEffectString;

int         vibLocationInt = 0;
int         vibActionInt = 0;


void recvWithStartEndMarkers() {        // Receive incoming serial input from start to end marker
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void parseData() {                         // split the incomming data into parts and convert to strings and integers
  strcpy(tempChars, receivedChars);
  char * strtokIndx;                       // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");     // get the first part - the string
  strcpy(vibLocation, strtokIndx);         // copy it to messageFromPC

  strtokIndx = strtok(NULL, ",");          // this continues where the previous call left off
  vibAction = *strtokIndx;                 // convert this part to a char

  strtokIndx = strtok(NULL, ",");
  vibEffectInt = atoi(strtokIndx);         // convert this part to an integer

  parsed = true;                           // parsing has been done

  if (vibLocation == VIB_LCTN_THUMB) {    // Check Location Input
    vibLocationString = "Thumb";
    vibLocationInt = vib_bus_thumb;
    //Serial.println("This is correct");
  }
  else if (*vibLocation == VIB_LCTN_INDEX) {
    vibLocationString = "Index";
    vibLocationInt = vib_bus_index;
  }
  else if (*vibLocation == VIB_LCTN_MIDDLE) {
    vibLocationString = "Middle";
    vibLocationInt = vib_bus_middle;
  }
  else if (*vibLocation == VIB_LCTN_RING) {
    vibLocationString = "Ring";
    vibLocationInt = vib_bus_ring;
  }
  else if (*vibLocation == VIB_LCTN_PINKY) {
    vibLocationString = "Pinky";
    vibLocationInt = vib_bus_pinky;
  }
  else if (*vibLocation == VIB_LCTN_HANDPALM) {
    vibLocationString = "Hand Palm";
    vibLocationInt = vib_bus_handpalm;
  }
  else {
    Serial.println("You have chosen an invalid vibrator");
    vibLocationString = "Nothing";
    parsed = false;       // wrong input
    // create code to change to correct bus
  }

  if (vibAction == VIB_GO) {               // Check Action Input
    vibActionString = "Go";
    vibActionInt = 1;
    //Serial.println("This is correct");
  }
  else if (vibAction == VIB_STOP) {
    vibActionString = "Stop";
    vibActionInt = 0;
  }
  else  {
    Serial.println("You have chosen an invalid action");
    vibActionString = "Nothing";
    parsed = false;       // wrong input
  }

  if (vibEffectInt > 200) {                // Check Effect Input
    Serial.println("You have chosen an invalid value");
    parsed = false;
  }
  else {
    vibEffectInt = vibEffectInt;
  }

}

void showParsedData()
{ // Show the incoming data
  Serial.print("Location ");
  Serial.println(vibLocationString);
  Serial.print("Action");
  Serial.println(vibActionString);
  Serial.print("Effect");
  Serial.println(vibEffectInt);

}

void handleParseData()
{ // Use the data to perform the necessary actions

  busSelect(vibLocationInt);
  switch (vibActionInt) {

    case 0:                                         // Stop vibrating

      if (vibMode == DRV2605_MODE_REALTIME) {
        Serial.println("Stop");
        vib[vibLocationInt].setRealtimeValue(0x00);
        //newData = false;
      }
      else if (vibMode == DRV2605_MODE_INTTRIG) {

        vib[vibLocationInt].stop();
        //newData = false;
      }

      else {
        Serial.println("Choose another mode for now...");
        //newData = false;
      }
      break;

    case 1:                                          // Start Vibrating
     
      busSelect(vibLocationInt);
      if (vibMode == DRV2605_MODE_REALTIME) {
        Serial.println("Start Vibrating Realtime");
        Serial.println(vibLocationInt);
        Serial.println(vibEffectInt);
       // vib[vibLocationInt].go();
        vib[vibLocationInt].setRealtimeValue(vibEffectInt);
        delay(500);
        
        //newData = false;
      }
      
      else if (vibMode == DRV2605_MODE_INTTRIG) {
        Serial.println("Start Vibrating Internal");
        vib[vibLocationInt].setWaveform(0, vibEffectInt);
        vib[vibLocationInt].setWaveform(1, 0);
        vib[vibLocationInt].go();
        //newData = false;

      }
      else {
        Serial.println("Choose another mode for now...");
        //newData = false;
      }
      break;

    default:
      break;
  }
  //newData = false;
}

//--------------------------Vibrator initialization --------------------------- //
void initVibrator(char mode)
{
  Serial.println("Begin Initialization, set mode is:");
  Serial.println((char)mode);
  for (int i = 0; i < amnt_vibrators; i++)
  {
    busSelect(i);                     //  select bus
    vib[i].begin();                   //  set-up the vibrators to standard configuration

    if (vib[i].begin()) {
      Serial.print( "Vibrator: "); Serial.print(i + 1); Serial.print( "on bus: "); Serial.println(i);
      vib[i].useLRA();                 //  set to LRA mode
      vib[i].selectLibrary(6);         //  select LRA library
      vib[i].setMode(DRV2605_MODE_REALTIME);                    // set to selected mode
      if (autoCal == true) {
        vib[i].autoCalibrationLRA(voltage_rms, voltage_peak, frequency);           // start auto calibration process


        if (vib[i].autoCalibrationLRA(voltage_rms, voltage_peak, frequency)) {
          Serial.println((String) "Calibration of vibrator " + i + " Complete");                   // check if auto calibration worked
        }
        else {
          Serial.println((String) "Calibration Failed, is tried again");
          vib[i].autoCalibrationLRA(voltage_rms, voltage_peak, frequency);           // start auto calibration process
          if (vib[i].autoCalibrationLRA(voltage_rms, voltage_peak, frequency)) {
            Serial.println((String) "Calibration of vibrator " + i + " Complete");                   // check if auto calibration worked
          }
          else {
            Serial.println((String) "Calibration Failed again");


          }
        }

      }

      else {
        Serial.println("Ooops, no actuator detected ... Check your wiring!"); // check if all vibrators are properly connected
      }
    }
    else {
      Serial.println("No Actuator attached");
    }
  }
}


// --------------------------- Set-up ---------------------------------------//
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  delay(250);     //allow time for communication start

  initVibrator(vibMode);
  //initAccelerometer();
  Serial.println("Setup finished");

  // receive input //
  Serial.println("This demo expects 3 pieces of data - which actuator, 's' for stop and 'g' for go, and an effect value");
  Serial.println("Enter data in this style  <t, g, 13> ");
  Serial.println();
}


//============================ LOOOP ========================================//

void loop()
{
  recvWithStartEndMarkers();
  if (newData == true) {
    //strcpy(tempChars, receivedChars);
    parseData();
    showParsedData();
    handleParseData();
    newData= false;
    
  //}
  //handleAccelerometer();
  //visualizeData(dataX, dataY, dataZ, lis);
  }
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
=======
#include <Wire.h>
#include "DRV2605L.h"
//#include <Adafruit_LIS3DH.h>
//#include <Adafruit_Sensor.h>
//#include "SPI.h"

// ------------------------Vibrator and Accelerometer parameters------------------------------ //
const int amnt_vibrators = 1;

DRV2605L vib[amnt_vibrators];
//Adafruit_LIS3DH lis = Adafruit_LIS3DH();                  //Default constructor is I2C, addr 0x19.

boolean     autoCal = true;                     //Autocalibration on or off
int         voltage_rms = 1.2;                  //rms voltage
int         voltage_peak = 1.9;                 //peak voltage
int         frequency = 240;                    //rated resonance frequency
char        vibMode = DRV2605_MODE_REALTIME;    //mode selection

float dataX;
float dataY;
float dataZ;

// ------------------------Serial Communication Commands-------------------- //
// Moving //
const char VIB_GO           = 'g';
const char VIB_STOP         = 's';

// Locations Vibrators ///
const char VIB_LCTN_THUMB   = 't';
const char VIB_LCTN_INDEX   = 'i';
const char VIB_LCTN_MIDDLE  = 'm';
const char VIB_LCTN_RING    = 'r';
const char VIB_LCTN_PINKY   = 'p';
const char VIB_LCTN_HANDPALM   = 'h';


// ------------------------Multiplexer--------------------------------------- //
const int vib_bus_thumb   = 0;
const int vib_bus_index   = 1;
const int vib_bus_middle  = 2;
const int vib_bus_ring    = 3;
const int vib_bus_pinky   = 4;
const int vib_bus_handpalm   = 5;

const int vib_bus_accelerometer = 7;

void busSelect(uint8_t bus) {      // Select Bus for multiplexer communication
  if (bus > 7) return;
  Wire.beginTransmission(0x70);    // TCA9548A address is 0x70
  Wire.write(1 << bus);            // send byte to select bus
  Wire.endTransmission();
}


//---------------------------Serial communication---------------------------------//
const byte  numChars = 32;
char        receivedChars[numChars];
char        tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data
char        vibLocation[numChars] = {0};
char        vibAction = 0;
int         vibEffectInt = 0;

boolean     parsed = false;
boolean     newData = false;

String      vibActionString;
String      vibLocationString;
String      vibEffectString;

int         vibLocationInt = 0;
int         vibActionInt = 0;


void recvWithStartEndMarkers() {        // Receive incoming serial input from start to end marker
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void parseData() {                         // split the incomming data into parts and convert to strings and integers
  strcpy(tempChars, receivedChars);
  char * strtokIndx;                       // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");     // get the first part - the string
  strcpy(vibLocation, strtokIndx);         // copy it to messageFromPC

  strtokIndx = strtok(NULL, ",");          // this continues where the previous call left off
  vibAction = *strtokIndx;                 // convert this part to a char

  strtokIndx = strtok(NULL, ",");
  vibEffectInt = atoi(strtokIndx);         // convert this part to an integer

  parsed = true;                           // parsing has been done

  if (vibLocation == VIB_LCTN_THUMB) {    // Check Location Input
    vibLocationString = "Thumb";
    vibLocationInt = vib_bus_thumb;
    //Serial.println("This is correct");
  }
  else if (*vibLocation == VIB_LCTN_INDEX) {
    vibLocationString = "Index";
    vibLocationInt = vib_bus_index;
  }
  else if (*vibLocation == VIB_LCTN_MIDDLE) {
    vibLocationString = "Middle";
    vibLocationInt = vib_bus_middle;
  }
  else if (*vibLocation == VIB_LCTN_RING) {
    vibLocationString = "Ring";
    vibLocationInt = vib_bus_ring;
  }
  else if (*vibLocation == VIB_LCTN_PINKY) {
    vibLocationString = "Pinky";
    vibLocationInt = vib_bus_pinky;
  }
  else if (*vibLocation == VIB_LCTN_HANDPALM) {
    vibLocationString = "Hand Palm";
    vibLocationInt = vib_bus_handpalm;
  }
  else {
    Serial.println("You have chosen an invalid vibrator");
    vibLocationString = "Nothing";
    parsed = false;       // wrong input
    // create code to change to correct bus
  }

  if (vibAction == VIB_GO) {               // Check Action Input
    vibActionString = "Go";
    vibActionInt = 1;
    //Serial.println("This is correct");
  }
  else if (vibAction == VIB_STOP) {
    vibActionString = "Stop";
    vibActionInt = 0;
  }
  else  {
    Serial.println("You have chosen an invalid action");
    vibActionString = "Nothing";
    parsed = false;       // wrong input
  }

  if (vibEffectInt > 200) {                // Check Effect Input
    Serial.println("You have chosen an invalid value");
    parsed = false;
  }
  else {
    vibEffectInt = vibEffectInt;
  }

}

void showParsedData()
{ // Show the incoming data
  Serial.print("Location ");
  Serial.println(vibLocationString);
  Serial.print("Action");
  Serial.println(vibActionString);
  Serial.print("Effect");
  Serial.println(vibEffectInt);

}

void handleParseData()
{ // Use the data to perform the necessary actions

  busSelect(vibLocationInt);
  switch (vibActionInt) {

    case 0:                                         // Stop vibrating

      if (vibMode == DRV2605_MODE_REALTIME) {
        Serial.println("Stop");
        vib[vibLocationInt].setRealtimeValue(0x00);
        //newData = false;
      }
      else if (vibMode == DRV2605_MODE_INTTRIG) {

        vib[vibLocationInt].stop();
        //newData = false;
      }

      else {
        Serial.println("Choose another mode for now...");
        //newData = false;
      }
      break;

    case 1:                                          // Start Vibrating
     
      busSelect(vibLocationInt);
      if (vibMode == DRV2605_MODE_REALTIME) {
        Serial.println("Start Vibrating Realtime");
        Serial.println(vibLocationInt);
        Serial.println(vibEffectInt);
       // vib[vibLocationInt].go();
        vib[vibLocationInt].setRealtimeValue(vibEffectInt);
        delay(500);
        
        //newData = false;
      }
      
      else if (vibMode == DRV2605_MODE_INTTRIG) {
        Serial.println("Start Vibrating Internal");
        vib[vibLocationInt].setWaveform(0, vibEffectInt);
        vib[vibLocationInt].setWaveform(1, 0);
        vib[vibLocationInt].go();
        //newData = false;

      }
      else {
        Serial.println("Choose another mode for now...");
        //newData = false;
      }
      break;

    default:
      break;
  }
  //newData = false;
}

//--------------------------Vibrator initialization --------------------------- //
void initVibrator(char mode)
{
  Serial.println("Begin Initialization, set mode is:");
  Serial.println((char)mode);
  for (int i = 0; i < amnt_vibrators; i++)
  {
    busSelect(i);                     //  select bus
    vib[i].begin();                   //  set-up the vibrators to standard configuration

    if (vib[i].begin()) {
      Serial.print( "Vibrator: "); Serial.print(i + 1); Serial.print( "on bus: "); Serial.println(i);
      vib[i].useLRA();                 //  set to LRA mode
      vib[i].selectLibrary(6);         //  select LRA library
      vib[i].setMode(DRV2605_MODE_REALTIME);                    // set to selected mode
      if (autoCal == true) {
        vib[i].autoCalibrationLRA(voltage_rms, voltage_peak, frequency);           // start auto calibration process


        if (vib[i].autoCalibrationLRA(voltage_rms, voltage_peak, frequency)) {
          Serial.println((String) "Calibration of vibrator " + i + " Complete");                   // check if auto calibration worked
        }
        else {
          Serial.println((String) "Calibration Failed, is tried again");
          vib[i].autoCalibrationLRA(voltage_rms, voltage_peak, frequency);           // start auto calibration process
          if (vib[i].autoCalibrationLRA(voltage_rms, voltage_peak, frequency)) {
            Serial.println((String) "Calibration of vibrator " + i + " Complete");                   // check if auto calibration worked
          }
          else {
            Serial.println((String) "Calibration Failed again");


          }
        }

      }

      else {
        Serial.println("Ooops, no actuator detected ... Check your wiring!"); // check if all vibrators are properly connected
      }
    }
    else {
      Serial.println("No Actuator attached");
    }
  }
}


// --------------------------- Set-up ---------------------------------------//
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  delay(250);     //allow time for communication start

  initVibrator(vibMode);
  //initAccelerometer();
  Serial.println("Setup finished");

  // receive input //
  Serial.println("This demo expects 3 pieces of data - which actuator, 's' for stop and 'g' for go, and an effect value");
  Serial.println("Enter data in this style  <t, g, 13> ");
  Serial.println();
}


//============================ LOOOP ========================================//

void loop()
{
  recvWithStartEndMarkers();
  if (newData == true) {
    //strcpy(tempChars, receivedChars);
    parseData();
    showParsedData();
    handleParseData();
    newData= false;
    
  //}
  //handleAccelerometer();
  //visualizeData(dataX, dataY, dataZ, lis);
  }
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
>>>>>>> 7b12795a3a087e0b47b85019f5c96b6db084289a

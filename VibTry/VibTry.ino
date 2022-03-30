
#include <Wire.h>
#include "DRV2605L.h"

// ------------------------Vibrator and Accelerometer parameters------------------------------ //
const int amnt_vibrators = 1;

DRV2605L vib[amnt_vibrators];

boolean     autoCal = true;                     //Autocalibration on or off
int         voltage_rms = 1.2;                  //rms voltage
int         voltage_peak = 1.9;                 //peak voltage
int         frequency = 240;                    //rated resonance frequency
uint8_t     vibMode = DRV2605_MODE_REALTIME;    //mode selection



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

  Serial.print("Location ");
  Serial.println(String(vibLocation));
  Serial.print("Action");
  Serial.println(String(vibAction));
  Serial.print("Effect");
  Serial.println(vibEffectInt);

  if (*vibLocation == VIB_LCTN_THUMB) {    // Check Location Input
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

  if (vibEffectInt > 255) {                // Check Effect Input
    Serial.println("You have chosen an invalid value");
    parsed = false;
  }
  else {
    vibEffectInt = map(vibEffectInt, 0,9,0,255);
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

      }
      else if (vibMode == DRV2605_MODE_INTTRIG) {

        vib[vibLocationInt].stop();

      }

      else {
        Serial.println("Choose another mode for now...");

      }
      break;

    case 1:                                          // Start Vibrating

      busSelect(vibLocationInt);
      if (vibMode == DRV2605_MODE_REALTIME) {
        Serial.println("Start Vibrating Realtime");
        Serial.println(vibLocationInt);
        Serial.println(vibEffectInt);
        vib[vibLocationInt].setRealtimeValue(vibEffectInt);
        delay(500);

      }

      else if (vibMode == DRV2605_MODE_INTTRIG) {
        Serial.println("Start Vibrating Internal");
        vib[vibLocationInt].setWaveform(0, vibEffectInt);
        vib[vibLocationInt].setWaveform(1, 0);
        vib[vibLocationInt].go();


      }
      else {
        Serial.println("Choose another mode for now...");

      }
      break;

    default:
      break;
  }
}

//--------------------------Vibrator initialization --------------------------- //
void initVibrator(uint8_t vibmode)
{
  Serial.println("Begin Initialization, set mode is:");
  String  modeString  = String(vibmode);
  Serial.println(modeString);

  for (int i = 0; i < amnt_vibrators; i++)
  {
    busSelect(i);                     //  select bus
    vib[i].begin();                   //  set-up the vibrators to standard configuration

    if (vib[i].begin()) {
      Serial.print( "Vibrator: "); Serial.print(i); Serial.print( "on bus: "); Serial.println(i);
      
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
        Serial.println("No autocalibration"); // check if all vibrators are properly connected
      }
      vib[i].useLRA();                 //  set to LRA mode
      vib[i].selectLibrary(6);         //  select LRA library
      vib[i].setMode(vibmode);                    // set to selected mode
      //Serial.println(vib[i].readRegister8(DRV2605_REG_MODE));
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
  delay(500);     //allow time for communication start
  initVibrator(vibMode);
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
    Serial.println("New data!");
    parseData();
    showParsedData();
    handleParseData();
    newData = false;
  }
}

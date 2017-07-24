/*
 * Copywrite Chris Blust 2017
 * IntelHacks 2017 Judges granted any rights desired for judging
 */
#include<CurieBLE.h>
#include<GPS.h>
#include<Adafruit_PWMServoDriver.h>
//Pulse lengths for rear motor (throttle)
//1050 to 1080 for use in grass, 1120 for use on pavement
#define THROTTLE_SPEED 1120
//The pulse length, in microseconds, that if sent to the Electronic Speed Control, produces a stopped rover
#define THROTTLE_NEUTRAL 1140
//Range of degrees a vehicle must come within from 90 degrees of this vehicle to trigger a stop
#define DANGER_RANGE 15
//LED indicators for debugging and demonstration
//Indicates the vehicle detected a potential collision
#define SAFE_STOP_LED 11
//Indicates the vehicle did not detect a potential collision
#define NO_STOP_LED 12
//UUID for the BLE service and track angle characteristic
#define BLE_UUID "7890"
//Max length for BLE track angle characteristic
#define BLE_MAX_LENGTH 16
//The name the rover is discoverable as
#define BLE_NAME "Rover"
//The frequency at which the PWM driver operates at
#define PWM_DRIVER_FREQUENCY 50
//The channel of the PWM driver the Electric Speed Control is connected to that controls the rear motor
#define THROTTLE_CHANNEL 15
//The channel of the PWM driver the steering servo motor is connected to
#define STEERING_SERVO_CHANNEL 0
//The arguement entered into the PWM driver setPWM call (always 0)
#define PWM_ON 0
//The time the rover waits to take a GPS reading after beginning moving in milliseconds
#define GPS_CALIBRATION_TIME 2000
//The time the vehicle waits for the other to pass in milliseconds
#define SAFE_STOP_WAIT_TIME 3000
//The time the vehicle continues moving foreward after a stop or no stop has been detected in milliseconds
#define CONCLUSION_DELAY 4000

//PWM Driver reference, controls the front steering servo and the rear motor throught the Electric Speed Controller
Adafruit_PWMServoDriver driver = Adafruit_PWMServoDriver(0x40);

//Bluetooth stuff
BLEPeripheral per;
BLEService roverService(BLE_UUID);
BLECharacteristic trackAngleCharacteristic(BLE_UUID, BLERead | BLEWrite, BLE_MAX_LENGTH);

//Reference to the Adafruit Ultimate Breakout GPS Board
GPS gps;

//Variable that allows the program to only run through once
bool go = true;

void setup(){
  //Setup LEDs
  pinMode(SAFE_STOP_LED, OUTPUT);
  pinMode(NO_STOP_LED, OUTPUT);
  
  //Start with LEDs off
  digitalWrite(SAFE_STOP_LED, LOW);
  digitalWrite(NO_STOP_LED, LOW);

  //Enable reading from the GPS
  gps.setupSerial(0,1);
  
  //Setup BLE stuff
  per.setLocalName(BLE_NAME);
  per.setDeviceName(BLE_NAME);
  per.setAppearance(true);
  roverService.addCharacteristic(trackAngleCharacteristic);
  per.addAttribute(roverService);
  per.begin();
  
  //Setup PWM driver
  driver.begin();
  driver.setPWMFreq(PWM_DRIVER_FREQUENCY);

  //Start rover stopped and pointing straight
  setThrottle(THROTTLE_NEUTRAL);
  setSteering(305);

  //Blink LEDs three times to indicate the rover is awaiting a BLE connection
  for(int x = 0; x < 3; x++){
    digitalWrite(SAFE_STOP_LED, HIGH);
    digitalWrite(NO_STOP_LED, HIGH);
    delay(500);
    digitalWrite(SAFE_STOP_LED, LOW);
    digitalWrite(NO_STOP_LED, LOW);
    delay(500);
  }
}
//double myTrack = 0.0;
//char piString[32];
//int piTrack = 0;

void loop(){
  BLECentral central = per.central();
  //if rover is connected to another Bluetooth device, start rear motor
  if(central && go){
      setThrottle(THROTTLE_SPEED);
      //wait to ensure gps reading is taken while moving
      delay(GPS_CALIBRATION_TIME);
      
      //Update the gps readings in the Arduino's RAM
      gps.update();

      //BLE.poll();
      
      //Get a reference to the current GPS angle
      int myTrackAngle = gps.angle;

      //Read the other vehicle's transmitted track angle from the BLE characteristic
      int otherTrackAngle = readTrackAngleCharacteristic();

      //Calculate the angles 90 degrees from this vehicle's track angle
      int dangerAngleOne = getOffset(myTrackAngle, 90);
      int dangerAngleTwo = getOffset(myTrackAngle, 270);

      //If the other vehicle is approaching within range of the danger angles, stop and let it pass
      if(inRange(otherTrackAngle, dangerAngleOne, DANGER_RANGE) || inRange(otherTrackAngle, dangerAngleTwo, DANGER_RANGE)){
          //Turn on the Green LED
          digitalWrite(SAFE_STOP_LED, HIGH);
          setThrottle(THROTTLE_NEUTRAL);
          delay(SAFE_STOP_WAIT_TIME);
          setThrottle(THROTTLE_SPEED);
          delay(CONCLUSION_DELAY);
          setThrottle(THROTTLE_NEUTRAL);
      }else{
          //Continue driving past the car without stopping, then eventually stop
          delay(CONCLUSION_DELAY);
          //Turn on the Red LED
          digitalWrite(NO_STOP_LED, HIGH);
          setThrottle(THROTTLE_NEUTRAL);
      }
  //Make sure the program does not loop again
  go = false;
}
  
}

//Reads the bytes of the track angle characteristic and converts them to a character array, then returns the int value of those characters
int readTrackAngleCharacteristic(){
    char otherString[32];
    strncpy(otherString,(char*)trackAngleCharacteristic.value(), trackAngleCharacteristic.valueLength());
    return atoi(otherString);
}

//Returns the degree value of i offset by offset, make negative to subtract
int getOffset(int i, int offset){
  return getDegree(i + offset);
}

//Interprets i as an angle in degrees and returns its coterminal angle in range [0, 360)
int getDegree(int i){
    if(i < 360 && i >= 0){
        return i;
    //Keep subtracting or adding 360 until the value is within the range
    }else if(i >= 360){
        return getDegree(i - 360);  
    }else{
        return getDegree(i + 360);  
    }
}

//Returns if check is in range of i (check and i are degrees in range [0, 360)
bool inRange(int check, int i, int range){
    if(i + range < 360 && i - range >= 0){
      if(check > i - range && check < i + range){
        return true;  
      }
    }
    for(int x = i; x > i - range; x--){
      if(getDegree(x) == check){
        return true;  
      }
    }
    for(int x = i; x < i + range; x++){
      if(getDegree(x) == check){
        return true;  
      }  
    }
    return false;
}

//Tells the PWM driver to output a pulse length of pulseLength out of the channel with the Electric Speed Controller (rear motor) connected
void setThrottle(int pulseLength){
  driver.setPWM(THROTTLE_CHANNEL, PWM_ON, pulseLength);
}

//Tells the PWM driver to output a pulse lenght of pulseLength out of the channel with the steering servo connected
void setSteering(int pulseLength){
 driver.setPWM(STEERING_SERVO_CHANNEL, PWM_ON, pulseLength);  
}

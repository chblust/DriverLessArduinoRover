/*
 * Copywrite Chris Blust 2017
 * IntelHacks 2017 Judges granted any rights required for judging
 */
#include <NewPing.h>
#include <Adafruit_PWMServoDriver.h>
#include <CurieBLE.h>

//The max distance, in cm, the sensors are allowed to read
#define MAX_DIST 400
//The frequency, in Hz, the Pulse Width Modulation Driver runs at
#define PWM_FREQ 50
//The channel of the PWM driver that the throttle is attached to
#define THROTTLE_CHANNEL 15
//The channel of the PWM driver that the steering servo is attached to
#define STEERING_CHANNEL 0
//The distance, in cm, that if read from an ultrasonic sensor, triggers a collision avoidance reaction
#define DANGER_DIST 250
//The pulse length, in microseconds, that if sent to the Electronic Speed Control, produces the desired rover speed
//1050 to 1080 for use in grass, 1120 for use on pavement
#define SPEED 1000
//The pulse length, in microseconds, that if sent to the Electronic Speed Control, produces a stopped rover
#define THROTTLE_NEUTRAL 1140
//The pulse length, in microseconds, that if sent to the steering servo, points the rover straight
#define STEERING_NEUTRAL 305
//The pulse length, in microseconds, that if sent to the steering servo, points the rover to the right
#define STEERING_RIGHT 375
//The pulse length, in microseconds, that if sent to the steering servo, points the rover to the left
#define STEERING_LEFT 205
//The pins of the arduino that initiate sonar pings from the ultrasonic sensors and read the responses from them
#define LEFT_TRIG 2
#define LEFT_ECHO 3
#define FRONT_TRIG 4
#define FRONT_ECHO 5
#define RIGHT_TRIG 7
#define RIGHT_ECHO 8
//The Universal Unique Identifier for the Bluetooth service and characteristic
#define BLE_UUID "19B10010-E8F2-537E-4F6C-D104768A1214"
//The name the rover BLE peripheral is discoverable as
#define BLE_NAME "Rover"
//This is used to keep track of what position the servo is in without having to read from something
enum ServoState{
  left,
  straight,
  right
};
ServoState steeringState = straight;

//The PWM Driver
Adafruit_PWMServoDriver driver = Adafruit_PWMServoDriver();
//The ultrasonic sensors
NewPing leftSensor(LEFT_TRIG, LEFT_ECHO, MAX_DIST);
NewPing frontSensor(FRONT_TRIG, FRONT_ECHO, MAX_DIST);
NewPing rightSensor(RIGHT_TRIG, RIGHT_ECHO, MAX_DIST);

//Bluetooth Low Energy Setup
BLEService roverService(BLE_UUID);
BLECharCharacteristic goChar(BLE_UUID, BLEWrite | BLERead);

//Variables that store the current sensor readings in cm
int leftReading;
int frontReading;
int rightReading;

//Variable that holds the state of the switch on the iPad application, telling the rover to stay stopped or continue moving
int go;

//Tells the rover if the switch state is changed so it doesn't write to the driver 20 times a second
bool modeChanged = false;

//Used to help determine modeChanged
int current;

void setup(){
    //Start and setup the PWM driver
    driver.begin();
    driver.setPWMFreq(PWM_FREQ);

    //Start with the rear motor off and the steering pointed straight
    setThrottle(THROTTLE_NEUTRAL);
    setSteering(STEERING_NEUTRAL);

    //Start and setup BLE
    BLE.begin();
    BLE.setLocalName(BLE_NAME);
    BLE.setAdvertisedService(roverService);
    roverService.addCharacteristic(goChar);
    BLE.addService(roverService);
    //Rover starts not moving
    goChar.setValue(0);
    BLE.advertise();
}

void loop(){
    BLE.poll();
    //Detects if the iPad application has written a new value to the characteristic
    current = goChar.value();
    if(go != current){
        go = current;
        modeChanged = true;
    }
    //if the mode has changed, start the rear motor and collision avoidance
    if(go == 1){
        if(modeChanged){
            setThrottle(SPEED); 
        }

        //Get readings from all the sensors
        frontReading = frontSensor.ping_cm();
        leftReading = leftSensor.ping_cm();
        rightReading = rightSensor.ping_cm();

        //Logic that makes collision avoidance ignore values of 0 (sensors often return false readings of 0)
        if(frontReading == 0)
        {frontReading = 401;}
        if(leftReading == 0)
        {leftReading = 401;}
        if(rightReading == 0)
        {rightReading = 401;}
      
        //If one of the readings is less than the distance considered dangerous, avoid collision, or straigten out if all directions are safe
        if (frontReading < DANGER_DIST || leftReading < DANGER_DIST || rightReading < DANGER_DIST){
            dodge();
        }else{
            goStraight();  
        }
    }else{//If the mode changed to 0, stop the rear motor, straighten out, and stop avoiding collisions
        if (modeChanged){
            setSteering(STEERING_NEUTRAL);
            setThrottle(THROTTLE_NEUTRAL);
        }
    }
  
    //Reset this variable so the driver isn't constantly written to
    modeChanged = false;
}

void dodge(){
    //If both right and left read that something is close, dodge in the direction where the obstacle is further away
    if(leftReading < DANGER_DIST && rightReading < DANGER_DIST){
        if(rightReading > leftReading){
            dodgeRight();
        }else{
            dodgeLeft();  
        }
    //If the left reading is not dangerous, dodge to the left
    }else if (leftReading > DANGER_DIST){
        dodgeLeft();  
    //If the right reading is not dangerous, dodge to the right
    }else{
        dodgeRight(); 
    }
}

void goStraight(){
    if (steeringState != straight){ //Checks if the steering is already straight
        setSteering(STEERING_NEUTRAL); //Sets it straight if not
        steeringState = straight; //Records that steering is currently straight
    }
}

void dodgeLeft(){
    if(steeringState != left){ //Checks if the steering is already to the left
        setSteering(STEERING_LEFT); //Sets it to the left if not
        steeringState = left; //Records that steering is currently to the left
    }
}

void dodgeRight(){
    if (steeringState != right){ //Checks if the steering is already to the right
        setSteering(STEERING_RIGHT); //Sets it to the right if not
        steeringState = right; //Records that steering is currently to the right
    }
}

//Sets the throttle PWM channel to pulseLength
void setThrottle(int pulseLength){
    driver.setPWM(THROTTLE_CHANNEL, 0, pulseLength);
}

//Sets the steering servo PWM channel to pulseLength
void setSteering(int pulseLength){
    driver.setPWM(STEERING_CHANNEL, 0, pulseLength);
}

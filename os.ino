#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Servo.h>

// Define stepper motor parameters
const int stepsPerRevolutionX = 200;  // Steps per revolution for X axis motor
const int stepsPerRevolutionY = 200;  // Steps per revolution for Y axis motor
const int xSize = 2300;               // X-axis size
const int ySize = 700;                // Y-axis size

// Define servo motor parameters
const int activationAngle = 180;      // Servo activation angle
const int restAngle = 90;             // Servo rest angle
int xPos = 0;                         // Current X position
int yPos = 0;                         // Current Y position

// Define stepper motor pins
const int enablePin = 13;
const int xStep = 32;
const int xDir = 33;
const int yStep = 22;
const int yDir = 23;

// Initialize stepper motors
AccelStepper stepperX(AccelStepper::DRIVER, xStep, xDir);  // X-axis stepper
AccelStepper stepperY(AccelStepper::DRIVER, yStep, yDir);  // Y-axis stepper

// Initialize servo motor
Servo servo;
const int servoPin = 6;               // Servo motor pin

// Tolerance parameters for nail placement
const int pre_tol = 20;
const int post_tol = 10;

// Function to convert steps to millimeters
float step2mm(int steps){
    float mm = 13 * 3.14 * steps / 200;
    return mm;
}

// Function to convert millimeters to steps
int mm2step(float mm){
    float steps = 400 * mm / 40;
    return int(round(steps));
}

// Speed parameters
float speedX = 8000.0;  // Speed for X-axis stepper
float speedY = 8000.0;  // Speed for Y-axis stepper
float servoSpeed = 8.0; // Speed for servo motor

// Timing parameters
unsigned long startTime = 0;
unsigned long interval = 100;

void processGCode(String gcode) {
  // Process G-code commands and execute corresponding actions

  // Handle M220 command: Set maximum speed and acceleration for stepper motors
  if (gcode.startsWith("M220")) {
    int xIndex = gcode.indexOf('X');
    int yIndex = gcode.indexOf('Y');
    int sIndex = gcode.indexOf('S');

    // Process X-axis speed and acceleration
    if (xIndex != -1) {
      float xValue = gcode.substring(xIndex + 1, yIndex).toFloat();
      stepperX.setMaxSpeed(xValue); 
      stepperX.setAcceleration(xValue / 1.3);
      speedX = xValue;
    }

    // Process Y-axis speed and acceleration
    if (yIndex != -1) {
      float yValue = gcode.substring(yIndex + 1, sIndex).toFloat();
      stepperY.setMaxSpeed(yValue);
      stepperY.setAcceleration(yValue / 1.3);
      speedY = yValue;
    }

    // Process servo speed
    if (sIndex != -1) {
      float sValue = gcode.substring(sIndex + 1).toFloat();
      servoSpeed = sValue;
    }
  }

  // Handle G92 command: Set current position
  if (gcode.startsWith("G92")) {
    pinMode(enablePin, HIGH);
    int xIndex = gcode.indexOf('X');
    int yIndex = gcode.indexOf('Y');

    // Set current X position
    if (xIndex != -1) {
      float xValue = gcode.substring(xIndex + 1, yIndex).toFloat();
      xPos = xValue;
    }

    // Set current Y position
    if (yIndex != -1) {
      float yValue = gcode.substring(yIndex + 1).toFloat();
      yPos = yValue;
    }
  }

  // Handle G01 command: Linear move
  if (gcode.startsWith("G01")) {
    Serial.println("Moving");
    int xIndex = gcode.indexOf('X');
    int yIndex = gcode.indexOf('Y');

    // Move X-axis to specified position
    if (xIndex != -1) {
      float xValue = gcode.substring(xIndex + 1, yIndex).toFloat();
      stepperX.moveTo(mm2step(-xValue));
    }

    // Move Y-axis to specified position
    if (yIndex != -1) {
      float yValue = gcode.substring(yIndex + 1).toFloat();
      stepperY.moveTo(mm2step(-yValue));
    }

    // Execute the movement
    stepperX.runToPosition();
    stepperY.runToPosition();
  }

  // Handle G02 command: Linear move and shoot nails based on steps
  if (gcode.startsWith("G02")) {
    Serial.println("Moving");
    int xIndex = gcode.indexOf('X');
    int yIndex = gcode.indexOf('Y');
    int fIndex = gcode.indexOf('F');
    int nIndex = gcode.indexOf('N');
    float xValue;
    float yValue;
    float no_nails;
    float nail_pos[] = {};
    Serial.println(mm2step(gcode.substring(xIndex+1,yIndex).toFloat()));
    if (xIndex != -1) {
      xValue = gcode.substring(xIndex + 1, yIndex).toFloat();
      stepperX.moveTo(mm2step(xValue));
      //stepperX.moveTo(xValue);
    }
    if (yIndex != -1) {
      yValue = gcode.substring(yIndex + 1,nIndex).toFloat();
      stepperY.moveTo(mm2step(-yValue));
    }
    if (nIndex != -1) {
      no_nails = gcode.substring(nIndex+1).toFloat();
      float nail_dist = mm2step((xValue - stepperX.currentPosition())/(no_nails + 1));
      for (int i = 0; i < no_nails - 1; i++) {
        nail_pos[i] = mm2step(stepperX.currentPosition()) + nail_dist * i;
      }
    }
    while (stepperX.currentPosition() != xValue) {
      stepperX.run();
      int i = 0;
      if (mm2step(stepperX.currentPosition()) == nail_pos[i] - pre_tol) {
        servo.write(activationAngle);
      }
      if (mm2step(stepperX.currentPosition()) == nail_pos[i] + post_tol) {
        servo.write(restAngle);
        i += 1;
      }
    }
  }

  // Handle G03 command: Linear move and shoot nails based on time
  if (gcode.startsWith("G03")) {
    int xIndex = gcode.indexOf('X');
    int yIndex = gcode.indexOf('Y');
    float xValue;
    float yValue;
    float no_nails;
    float nail_pos[] = {};
    Serial.println(mm2step(gcode.substring(xIndex+1,yIndex).toFloat()));
    if (xIndex != -1) {
      xValue = gcode.substring(xIndex + 1, yIndex).toFloat();
      stepperX.moveTo(mm2step(-xValue));
    }
    if (yIndex != -1) {
      yValue = gcode.substring(yIndex + 1).toFloat();
      stepperY.moveTo(mm2step(-yValue));
    }
    startTime = millis();
    int i = 1;
    int servoDelay = servoSpeed;
    int pos = restAngle;
    while(stepperX.run()) {
      stepperX.run();
      if (millis() - startTime >= servoDelay) {
        servo.write(pos + i);
        startTime = millis();
        if (i >= activationAngle - restAngle) {
          i = 0;
        }
        i += 1;
      }
    }
    servo.write(restAngle);
  }

  // Handle M03 command: Activate servo motor
  if (gcode.startsWith("M03")) {
    Serial.write("Pulling trigger");
    servo.write(restAngle);
    delay(50);
    servo.write(activationAngle);
    delay(300);
    servo.write(restAngle);
  }

  // Handle M30 command: Return to home position and detach servo
  if (gcode.startsWith("M30")) {
    stepperX.moveTo(0);
    stepperY.moveTo(0);
    stepperX.runToPosition();
    stepperY.runToPosition();
    servo.detach();
  }

  // Additional G-code commands can be added here as needed
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("Initiated");

  // Set up stepper motors
  stepperX.setMaxSpeed(12000); 
  stepperX.setAcceleration(8000);
  stepperY.setMaxSpeed(8000);
  stepperY.setAcceleration(6000);

  // Attach servo to its pin
  servo.attach(servoPin);
}

void loop() {
  // Check if serial data is available
  if (Serial.available() > 0) {
    // Read the G-code command from serial
    String gcode = Serial.readStringUntil('\n');

    // Check if the command starts with 'G' or 'M'
    if (gcode.startsWith("G") || gcode.startsWith("M")) {
      Serial.println(gcode);
      processGCode(gcode); // Process the G-code command
    }
  }
}
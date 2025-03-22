#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // initial servo position
int servoPin = 5; // Pin connected to the servo's control wire

void setup() {
  Serial.begin(115200);
  myservo.attach(servoPin);  // attaches the servo on pin D1 to the servo object
  myservo.write(pos); //set to initial position
}

void loop() {
  if (Serial.available() > 0) {
    int val = Serial.parseInt(); // read the incoming byte:

    if (val == 1) {
      pos = 0; //Move clockwise to 180
      myservo.write(pos);
      Serial.println("Moving clockwise to 180 degrees");
    } else if (val == -1) {
      pos = 180; //Move anti-clockwise to 0
      myservo.write(pos);
      Serial.println("Moving anti-clockwise to 0 degrees");
    } else if (val == 0) {
      pos = 90; //Return to rest position
      myservo.write(pos);
      Serial.println("Returning to rest position (90 degrees)");
    } else {
      Serial.println("Invalid input. Enter 1, -1, or 0.");
    }
    Serial.readStringUntil('\n'); //Clear the serial buffer.
  }
  delay(15); // Delay to allow servo to reach position
}
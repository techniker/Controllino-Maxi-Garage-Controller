// GarageController
// <tec ( att ) sixtopia.net>
#include <Arduino.h>
#include <Controllino.h>
#include <Bounce2.h> // Include the Bounce2 library
#include <Ethernet.h>
#include <SPI.h>

// Garage door controller pins
const int buttonPin = CONTROLLINO_A9;        // Pin for the button
const int doorPowerPin = CONTROLLINO_RELAY_08;    // Pin to control the door power
const int doorDirectionPin = CONTROLLINO_RELAY_09; // Pin to control the door direction (up/down)
const int statusLightPin = CONTROLLINO_D11;   // Pin for the status light
const int statusLightPinLocked = CONTROLLINO_D10;   // Pin for status light1
const int endSwitchPin = CONTROLLINO_A8; // Pin for the end switch
const int endSwitchPowerOffDelay = 100; // Delay before turning off power pin when end switch is triggered (milliseconds)

const int doorMovementTime = 30000; // 60 seconds in milliseconds
const int blinkInterval = 300;      // Status light blink interval in milliseconds
const int debounceInterval = 50;   // Debounce interval in milliseconds
const int doorChangeDelay = 1000;  // 1-second delay before changing door state (milliseconds)

// Create a Bounce2 object for button debouncing
Bounce debouncer = Bounce();

int buttonState = LOW;
int lastButtonState = LOW;
int doorState = 0; // 0 for idle, 1 for opening, 2 for closing

unsigned long doorStartTime = 0;
boolean doorMoving = false;
boolean statusLightOn = false;
unsigned long lastBlinkTime = 0;

int endSwitchState = LOW; // The end switch is initially in the fully closed state
int lastEndSwitchState = HIGH;
unsigned long endSwitchTriggerTime = 0;
unsigned long lastDoorStateChangeTime = 0; // Track the time when the door state was last changed

// Network settings for Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip('10.22.5.30');
EthernetServer server(80);

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(doorPowerPin, OUTPUT);
  pinMode(doorDirectionPin, OUTPUT);
  pinMode(statusLightPin, OUTPUT);
  pinMode(statusLightPinLocked, OUTPUT);
  pinMode(endSwitchPin, INPUT); // Use INPUT_PULLUP to enable internal pull-up resistor
  pinMode(CONTROLLINO_D9, OUTPUT); // Set CONTROLLINO_D9 as an output pin for fully closed state

  // Attach the debouncer to the buttonPin and set debounce interval
  debouncer.attach(buttonPin);
  debouncer.interval(debounceInterval);

  Serial.begin(115200);
  Serial.print("GarageControl v1.0");
  Serial.print(endSwitchState);

  // Start the Ethernet connection and the server
  Ethernet.begin(mac, ip);
  server.begin();
}

void controlDoor(const String& action) {
  if (action == "open") {
    // Logic to open the door
    // ...
  } else if (action == "close") {
    // Logic to close the door
    // ...
  } else if (action == "stop") {
    // Logic to stop the door
    // ...
  }
}

void loop() {
  // Update the button state through debouncer
  debouncer.update();
  buttonState = debouncer.read();

  // Check if the button has been pressed
  if (buttonState == LOW && lastButtonState == HIGH) {
    // Check if the 1-second delay has passed since the last door state change
    unsigned long currentTime = millis();
    if (currentTime - lastDoorStateChangeTime >= doorChangeDelay) {
      // Change door state only if the delay has passed
      doorState = (doorState + 1) % 3;

      // Start or stop door movement
      if (doorState == 1 || doorState == 2) {
        doorStartTime = millis();
        doorMoving = true;
        statusLightOn = true;

        // Set direction output
        if (doorState == 1) {
          // Opening
          digitalWrite(doorDirectionPin, HIGH); // Set direction to up
        } else if (doorState == 2) {
          // Closing
          digitalWrite(doorDirectionPin, LOW); // Set direction to down
        }

        // Wait for 300ms before enabling the power pin
        delay(300);
        digitalWrite(doorPowerPin, HIGH); // Turn on door power
      } else {
        doorMoving = false;
        statusLightOn = false;
        digitalWrite(doorPowerPin, LOW); // Turn off door power
      }

      // Update the time of the last door state change
      lastDoorStateChangeTime = currentTime;
    }
  }

  // Control door movement
  if (doorMoving) {
    unsigned long currentTime = millis();
    if (currentTime - doorStartTime < doorMovementTime) {
      // Continue with door movement
      Serial.print("endSwitchState:");
      Serial.print(endSwitchState);
    } else {
      // Time to stop door movement
      doorMoving = false;
      statusLightOn = false;
      digitalWrite(doorPowerPin, LOW); // Turn off door power
    }
  }

  // Control status light blinking
  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentMillis;
    if (statusLightOn) {
      digitalWrite(statusLightPin, !digitalRead(statusLightPin)); // Toggle status light
    } else {
      digitalWrite(statusLightPin, HIGH); // Turn on status light constantly while idle
    }
  }

  // Check if the door is moving down and the end switch state has changed
  if (doorState == 2) {
    if (endSwitchState != lastEndSwitchState) {
      if (endSwitchState == HIGH) {
        // End switch has gone high, delay turning off power pin
        digitalWrite(statusLightPinLocked, HIGH);
        endSwitchTriggerTime = millis() + endSwitchPowerOffDelay; // Set the trigger time with a delay
      }
    }
  }

  // Check if the end switch delay has passed and turn off the power pin
  if (endSwitchTriggerTime > 0 && (millis() - endSwitchTriggerTime >= endSwitchPowerOffDelay)) {
    doorMoving = false;
    statusLightOn = false;
    digitalWrite(doorPowerPin, LOW); // Turn off door power
    endSwitchTriggerTime = 0; // Reset the trigger time
  }

  // Update the last end switch state
  lastEndSwitchState = endSwitchState;

// Web server handling
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    String request = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE html><html>");
          client.println("<head><title>Garage Door Controller</title></head>");
          client.println("<body><h1>Garage Door Controller</h1>");
          client.println("<button onclick=\"location.href='/open'\">Open</button>");
          client.println("<button onclick=\"location.href='/close'\">Close</button>");
          client.println("<button onclick=\"location.href='/stop'\">Stop</button>");
          client.println("</body></html>");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    client.stop();

    if (request.indexOf("/open") > 0) {
      controlDoor("open");
    } else if (request.indexOf("/close") > 0) {
      controlDoor("close");
    } else if (request.indexOf("/stop") > 0) {
      controlDoor("stop");
    }
  }
  // Update the last button state
  lastButtonState = buttonState;
}
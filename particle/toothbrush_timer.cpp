/*
Simple project to get my kids to brush their teeth; they each got a button and an associated blinking
LED that would blink for 30 seconds; the last 5 seconds would blink more rapidly to let them know 
they were nearly done. Over time, I gradually increased the timer, pushing the code remotely to the
Particle Photon. Once brushing was done, they'd use the same timer for swishing flouride.

*/

// This #include statement was automatically added by the Particle IDE.
// #include "SparkFunMicroOLED/SparkFunMicroOLED.h"
#include "Particle.h"

// MicroOLED oled;
int PAUSE = 100; // milliseconds to pause

int GreenButtonPressed = 0;
int GreenButton = D0;
int GreenLED = A0;
long GreenStartTime;
long GreenCountdownTime = 0;

int BlueButtonPressed = 0;
int BlueButton = D1;
int BlueLED = A1;
long BlueStartTime;
long BlueCountdownTime = 0;

long lastTime = 0;

//long countdown = 658;
long countdown= 180;

void setup() {
    Serial.begin(9600);
    // oled.begin();
    // oled.clear(ALL);
    // oled.display();

    pinMode(GreenButton, INPUT_PULLUP);
    pinMode(GreenLED, OUTPUT);

    pinMode(BlueButton, INPUT_PULLUP);
    pinMode(BlueLED, OUTPUT);

    // delay(1000);
    // oled.clear(PAGE);
}

void loop() {
    String btnState;

    // oled.clear(PAGE);
    // printMsg(53, 40, 0, "on");

    int pushButtonState = digitalRead(GreenButton);
    if (pushButtonState == LOW) { // button pressed
        Particle.publish("debug", "green button pressed");
        GreenButtonPressed = 1;
        GreenStartTime = millis();
        GreenCountdownTime = millis() + (countdown * 1000);
    }
    
    pushButtonState = digitalRead(BlueButton);
    if (pushButtonState == LOW) { // button pressed
        Particle.publish("debug", "blue button pressed");
        BlueButtonPressed = 1;
        BlueStartTime = millis();
        BlueCountdownTime = millis() + (countdown * 1000);
    }

    if (GreenButtonPressed) {
        if (GreenCountdownTime - millis() > 0) {
            int blinkTime = 250;
            if (GreenCountdownTime - millis() < 5000) {
                blinkTime = 100;
            }
            digitalWrite(GreenLED, LOW);
            delay(blinkTime);
            digitalWrite(GreenLED, HIGH);
            // printMsg(0,30,0,String::format("%5d", GreenCountdownTime - millis()));
            // print_time(1+(GreenCountdownTime - millis())/1000);
        }
        if (GreenCountdownTime < millis()) {
            GreenButtonPressed = 0;
            digitalWrite(GreenLED, LOW);
            GreenStartTime = 0;
            GreenCountdownTime = 0;
        }
    }
    if (BlueButtonPressed) {
        if (BlueCountdownTime - millis() > 0) {
            int blinkTime = 250;
            if (BlueCountdownTime - millis() < 5000) {
                blinkTime = 100;
            }
            digitalWrite(BlueLED, LOW);
            delay(blinkTime);
            digitalWrite(BlueLED, HIGH);
            // printMsg(0,30,0,String::format("%5d", BlueCountdownTime - millis()));
            // print_time(1+(BlueCountdownTime - millis())/1000);
        }
        if (BlueCountdownTime < millis()) {
            BlueButtonPressed = 0;
            digitalWrite(BlueLED, LOW);
            BlueStartTime = 0;
            BlueCountdownTime = 0;
        }
    }

    // printMsg(0,16,0,String::format("%d", GreenButtonPressed));

    delay(PAUSE);
    lastTime = millis();
}


void printMsg(int x, int y, int fontType, String msg) {
    // oled.setFontType(fontType);
    // oled.setCursor(x, y);
    // oled.print(msg);
    // oled.display();
}

void print_time(int seconds) {
    printMsg(0,0,1, String::format("%02ds", seconds));
}

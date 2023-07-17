#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9
#define BUTTON_PIN 3
#define LED_PIN_SIGNAL 4   // Signal pin connected to the LED module
#define LED_PIN_POSITIVE 5 // Positive pin connected to the LED module

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo doorLock;

// Define the authorized RFID card UIDs
byte authorizedCard1[] = {0xBC, 0x53, 0x03, 0x38};
byte authorizedCard2[] = {0x55, 0x66, 0x77, 0x88};
// Add more authorized cards if needed

bool doorOpen = false;
unsigned long doorOpenTime = 0;
const unsigned long doorOpenDuration = 3000; // 3 seconds

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  doorLock.attach(2); // Connect the door lock control pin to digital pin 2
  doorLock.write(90); // Initialize the door lock as locked
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Configure the button pin as an input with internal pull-up resistor
  pinMode(LED_PIN_SIGNAL, OUTPUT);
  pinMode(LED_PIN_POSITIVE, OUTPUT);
  digitalWrite(LED_PIN_SIGNAL, HIGH); // Turn off the LED to save power
  Serial.println("Door lock system initialized.");
}

void loop() {
  // Check for button press
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (!doorOpen) {
      doorOpen = true;
      unlockDoor();
    }
  }

  // Check for RFID card presence
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    byte currentCardUID[4];
    for (byte i = 0; i < 4; i++) {
      currentCardUID[i] = mfrc522.uid.uidByte[i];
    }

    Serial.print("Detected Card UID: ");
    for (byte i = 0; i < 4; i++) {
      Serial.print(currentCardUID[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Compare the card UID with authorized cards
    if (compareCardUID(currentCardUID, authorizedCard1) || compareCardUID(currentCardUID, authorizedCard2)) {
      unlockDoor();
    } else {
      Serial.println("Access denied.");
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  // Check if the door open duration has expired
  if (doorOpen && (millis() - doorOpenTime >= doorOpenDuration)) {
    lockDoor();
  }

  // Update the LED color based on the door status
  if (doorOpen) {
    // Turn on the green LED by setting the signal pin LOW and the positive pin HIGH
    digitalWrite(LED_PIN_SIGNAL, HIGH);
    digitalWrite(LED_PIN_POSITIVE,LOW);
  } else {
    // Turn on the red LED by setting the signal pin HIGH and the positive pin LOW
    digitalWrite(LED_PIN_SIGNAL, LOW);
    digitalWrite(LED_PIN_POSITIVE, HIGH);
  }
}

void unlockDoor() {
  doorLock.attach(2); // Attach the servo to the control pin
  doorLock.write(0); // Unlock the door by setting the servo position to 90 degrees
  doorOpen = true;
  doorOpenTime = millis();
  Serial.println("Door unlocked.");
}

void lockDoor() {
  doorLock.write(85); // Lock the door by setting the servo position to 0 degrees
  doorOpen = false;
  Serial.println("Door locked.");
}

bool compareCardUID(byte* cardUID1, byte* cardUID2) {
  for (byte i = 0; i < 4; i++) {
    if (cardUID1[i] != cardUID2[i]) {
      return false;
    }
  }
  return true;
}
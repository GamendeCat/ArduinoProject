#include <FastLED.h>

#include <NintendoExtensionCtrl.h>

// TCA9548A

/*
  Project:
  AANPASSEN !!!! Een looplicht dat oplicht van de ene naar de andere kant.
  We maken gebruik van array, for-lus en een functie om een led te laten pinken
  Deze code is gemaakt als opdracht in het VTI-Ieper door Jules Pruvost
  van klas 4IW, schooljaar 2020-21.
  Alle code en methodes vallen onder de licentie Creative Common Licence:
  https://creativecommons.org/licenses/by-nc-sa/4.0/deed.nl
  Indien mijn code gebaseerd is op bestaande code, dan wordt de bron steeds als
  commentaar vermeld.
  http://dronebotworkshop.com/multiple-i2c-bus/
*/
//constants
const int dataPin = 2; //geel-blauw-bruin
const int NUM_LEDS = 67;
//const int buttonPin = 11;
//const int buttonPinTwo = 12;
const int redLeds[12] = {0, 1, 2, 3, 4, 5, (NUM_LEDS - 1), (NUM_LEDS - 2), (NUM_LEDS - 3) , (NUM_LEDS - 4) , (NUM_LEDS - 5), (NUM_LEDS - 6)};
const int left[6] = {0, 1, 2, 3, 4, 5};
const int right[6] = {(NUM_LEDS - 1), (NUM_LEDS - 2), (NUM_LEDS - 3) , (NUM_LEDS - 4) , (NUM_LEDS - 5), (NUM_LEDS - 6)};

//variablen
boolean timerON = false;
unsigned long timer, startTime = 0;
unsigned long delayTime = 100;
int ledCount = 0;
boolean optellen = true; //true countup | false = countdown
boolean firsTime = true;
int score = 0;
boolean active = true;
boolean leftRight = true; // true = Right | false = left

//libraries definen
//Bounce2::Button button = Bounce2::Button();
//Bounce2::Button button1 = Bounce2::Button();
CRGB leds[NUM_LEDS];
Nunchuk nchuk1;
Nunchuk nchuk2;

void TCA9548A(uint8_t bus)
{
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

void setup() {
  delay(2000);
  /*button.attach(buttonPin, INPUT_PULLUP);
    button.interval(5);
    button.setPressedState(LOW);
    button1.attach(buttonPinTwo, INPUT_PULLUP);
    button1.interval(5);
    button1.setPressedState(LOW);
  */

  //ledstrip aanvoer rechts

  //wit links
  //zwart rechts

  FastLED.addLeds<NEOPIXEL, dataPin>(leds, NUM_LEDS);
  Serial.begin(9600);
  Serial.print("----Start Debugging----");
  for (int i : leds) {
    leds[i].setHSV(0, 255, 0);
  }

  // start nunchuk's
  TCA9548A(0);
  nchuk1.begin();
  TCA9548A(1);
  nchuk2.begin();


  TCA9548A(0);

  while (!nchuk1.connect()) {
    Serial.println("Nunchuk 1 not detected!");
    delay(1000);
  }

  TCA9548A(1);

  while (!nchuk2.connect()) {
    Serial.println("Nunchuk 2 not detected!");
    delay(1000);
  }

  I2Cscanner();

  startTime = millis();
}

void loop() {
  if (active) {
    // making 1 second (time-variables must be of type ’long’)
    //               |<------------------- delayTime ------------->|
    //     |---------|=================================|-----------|-----> t(s)
    // millis()   startTime                          timer
    //               |<------  timer-startTime   ----->|
    timer = millis();
    if ((timer - startTime) > delayTime) {
      timerON = true;
      Serial.println(timer - startTime);
      startTime = millis();
      if (!firsTime) {
        if (leftRight) {
          for (int i : right) {
            if (i == ledCount) {
              leftRight = !leftRight;
              // 0 - 35
              if ((delayTime - 25) <= 5000) {
                delayTime -= 25;
              }
            }
          }
        } else {
          for (int i : left) {
            if (i == ledCount) {
              leftRight = !leftRight;
              if ((delayTime - 25) <= 5000) {
                delayTime -= 25;
              }
            }
          }
        }
      }
      firsTime = false;
    }

    if (timerON) {
      if (ledCount < 0 || ledCount > NUM_LEDS) {
        endGame();
        return;
      }
      ledsTurnOff(ledCount, (ledCount - 1), (ledCount - 2));
      if (optellen) {
        ledCount++;
      } else {
        ledCount--;
      }
      timerON = !timerON;
    }

    readInfo();

    //button.update();
    //button1.update();
    /*if (optellen) {
      buttonPress(button);
      } else {
      buttonPress(button1);
      }
    */
  }
}

void readInfo() {
  TCA9548A(0);
  boolean success1 = nchuk1.update();  // Get new data from the controller
  TCA9548A(1);
  boolean success2 = nchuk2.update();  // Get new data from the controller
  if (!success1 || !success2) {
    Serial.println("One of the controllers is disconnected!");
    delay(1000);
  } else {
    if (optellen) {
      // nog niet zeker welke nunchuck welke kant is
      TCA9548A(0);
      boolean zButton1 = nchuk1.buttonZ();

      if (zButton1) {
        for (int j : right) {
          if (ledCount == j) {
            Serial.println("ZButton1");
            buttonPress(ledCount, 6);
            optellen = !optellen;
            Serial.print("Delaytime: ");
            Serial.println(delayTime);
            return;
          }
        }
      }
    } else {
      TCA9548A(1);

      boolean zButton2 = nchuk2.buttonZ();

      if (zButton2) {
        for (int j : left) {
          if (ledCount == j) {
            Serial.println("ZButton2");
            buttonPress(ledCount, 6);
            optellen = !optellen;
            Serial.print("Delaytime: ");
            Serial.println(delayTime);
            return;
          }
        }
      }
    }
  }
}


void ledsTurnOff(int currentLed, int beforeLed, int bbLed) {
  if (!(currentLed < 0) || !(beforeLed < 0) || !(bbLed < 0) || !(currentLed > NUM_LEDS) || !(beforeLed > NUM_LEDS) || !(bbLed > NUM_LEDS)) {
    if (optellen) {
      for (int i : right) {
        redOrGreen(i, currentLed, 100);
        redOrGreen((i - 1), beforeLed, 50);
        redOrGreen((i - 2), bbLed, 0);
      }
    } else {
      for (int i : left) {
        redOrGreen(i, currentLed, 100);
        redOrGreen((i + 1), (currentLed + 1), 50);
        redOrGreen((i + 2), (currentLed + 2), 0);
      }
    }
    FastLED.show();
  }
}
/*void buttonPress(Bounce2::Button &buttonOne) {
  if (buttonOne.pressed()) {
    if (optellen) {
      for (int j : right) {
        Serial.print("j: ");
        Serial.println(j);
        Serial.print("LedCount: ");
        Serial.println(ledCount);
        if (ledCount == j) {
          Serial.println("Pressed");
          buttonPress(ledCount, 6);
          optellen = !optellen;
          return;
        }
      }
    } else {
      for (int j : left) {
        Serial.print("j: ");
        Serial.println(j);
        Serial.print("LedCount: ");
        Serial.println(ledCount);
        if (ledCount == j) {
          Serial.println("Pressed");
          buttonPress(ledCount, 6);
          optellen = !optellen;
          return;
        }
      }
    }
    endGame();
  }
  }
*/

void buttonPress(int ledGetIn, int amountOfLets) {
  for (int i = 1; i > (amountOfLets + 1); i++) {
    if (i == ledGetIn) {
      score += i;
      leftRight = !leftRight;
    }

    if (i == (NUM_LEDS - i)) {
      score += i;
      leftRight = !leftRight;
    }
  }

}

void redOrGreen(int i, int led, int brightness) {
  if (i == led) {
    leds[led].setHSV(0, 255, brightness);
  } else {
    leds[led].setHSV(120, 255, brightness);
  }
}

void endGame() {
  for (int i = 0; i < (NUM_LEDS + 1); i++) {
    leds[i].setHSV(0, 255, 50);
  }
  FastLED.show();
  Serial.println("EndGame?");
  Serial.print("Your score is: ");
  Serial.println(score);
  active = !active;
}

void I2Cscanner() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000);           // wait 5 seconds for next scan
}

/**************************************************
  Includes
**************************************************/
#include <PN5180ISO15693.h>
#include <Adafruit_NeoPixel.h>

/**************************************************
  Defines
**************************************************/
#define VERSION "1.0"
#define SKETCHNAME "Pn5180Esp"
#define LED_BRIGHTNESS 10

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_NANO)

#define PN5180_NSS  10
#define PN5180_BUSY 9
#define PN5180_RST  7

#elif defined(ARDUINO_ARCH_ESP32)

#define PN5180_NSS  16
#define PN5180_BUSY 5
#define PN5180_RST  17

#elif defined(ARDUINO_ARCH_ESP8266)

#define PN5180_NSS 4
#define PN5180_BUSY 16
#define PN5180_RST 5
#define WS2812B_PIN D8

#else
#error Please define your pinout here!
#endif

/**************************************************
  Variables
**************************************************/
PN5180ISO15693 nfc15693(PN5180_NSS, PN5180_BUSY, PN5180_RST);
uint8_t password[] = {0x0F, 0x0F, 0x0F, 0x0F};      // default password
uint8_t password2[] = {0x5B, 0x6E, 0xFD, 0x7F}; // t***e password
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, WS2812B_PIN, NEO_GRB + NEO_KHZ800);

/*************************************
  Setup
*************************************/
void setup()
{
  pixels.begin();
  ledFeedback(LED_BRIGHTNESS,LED_BRIGHTNESS,LED_BRIGHTNESS,100);

  Serial.setTimeout(50);
  Serial.begin(115200);
  Serial.println(F("=================================="));
  Serial.println(F("Uploaded: " __DATE__ " " __TIME__));
  Serial.println(F("PN5180 NFC15693 Demo Sketch"));

  nfc15693.begin();

  Serial.println(F("----------------------------------"));
  Serial.println(F("PN5180 Hard-Reset..."));
  nfc15693.reset();

  Serial.println(F("----------------------------------"));
  Serial.println(F("Reading product version..."));
  uint8_t productVersion[2];
  nfc15693.readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  Serial.print(F("Product version="));
  Serial.print(productVersion[1]);
  Serial.print(".");
  Serial.println(productVersion[0]);

  if (0xff == productVersion[1]) { // if product version 255, the initialization failed
    Serial.println(F("Initialization failed!?"));
    Serial.println(F("Press reset to restart..."));
    Serial.flush();
    exit(-1); // halt
  }

  Serial.println(F("----------------------------------"));
  Serial.println(F("Reading firmware version..."));
  uint8_t firmwareVersion[2];
  nfc15693.readEEprom(FIRMWARE_VERSION, firmwareVersion, sizeof(firmwareVersion));
  Serial.print(F("Firmware version="));
  Serial.print(firmwareVersion[1]);
  Serial.print(".");
  Serial.println(firmwareVersion[0]);

  Serial.println(F("----------------------------------"));
  Serial.println(F("Reading EEPROM version..."));
  uint8_t eepromVersion[2];
  nfc15693.readEEprom(EEPROM_VERSION, eepromVersion, sizeof(eepromVersion));
  Serial.print(F("EEPROM version="));
  Serial.print(eepromVersion[1]);
  Serial.print(".");
  Serial.println(eepromVersion[0]);

  Serial.println(F("----------------------------------"));
  Serial.println(F("Enable RF field..."));
  nfc15693.setupRF();
}

/*************************************
  Loop
*************************************/
void loop()
{
  serialInterface();
}

/**************************************************
  Serial interface for communication to the PC
**************************************************/
void serialInterface()
{
  // handle input strings
  if (Serial.available())
  {
    // read the string from the interface
    String command = Serial.readString();

    // handle command
    handleCommand(command);
  }
}

void handleCommand(String command)
{
  String response = "";
  uint8_t uid[10];

  // handle the command
  if (command.startsWith("v"))
  {
    response = (String)SKETCHNAME + " - " + (String)VERSION;
    Serial.println(response);
  }
  else if (command.startsWith("u"))
  {
    nfc15693.reset();
    nfc15693.setupRF();
    delay(50);

    // try to unlock with t***e password
    ISO15693ErrorCode myrc = nfc15693.disablePrivacyMode(password2);
    if (ISO15693_EC_OK == myrc)
    {
      Serial.println("ok");
      ledFeedback(0,LED_BRIGHTNESS,0,100);
      return;
    }
    else
    {
      nfc15693.reset();
      nfc15693.setupRF();
      delay(50);

      //try to unlock with default password");
      ISO15693ErrorCode myrc = nfc15693.disablePrivacyMode(password);
      if (ISO15693_EC_OK == myrc)
      {
        Serial.println("ok");
        ledFeedback(0,LED_BRIGHTNESS,0,100);
        return;
      }
    }
    
    Serial.println("nok");
    ledFeedback(LED_BRIGHTNESS,0,0,100);
  }
  else if (command.startsWith("l"))
  {
    Serial.println("nok");
  }
  else if (command.startsWith("i"))
  {
    nfc15693.reset();
    nfc15693.setupRF();
    delay(50);

    // try to read ISO15693 inventory
    ISO15693ErrorCode rc = nfc15693.getInventory(uid);
    if (rc == ISO15693_EC_OK)
    {
      for (int i = 0; i < 8; i++)
      {
        response = response + (uid[7 - i] < 0x10 ? "0" : "");
        response = response + String(uid[7 - i], HEX);

      }
      response.toUpperCase();

      ledFeedback(0,LED_BRIGHTNESS,0,100);
    }
    else
    {
      ledFeedback(LED_BRIGHTNESS,0,0,100);
    }

    Serial.println(response);
  }
}

void ledFeedback(int r, int g, int b, int delayMs)
{
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  delay(delayMs);
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();
}

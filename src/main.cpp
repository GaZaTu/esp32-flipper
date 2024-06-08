#include "credentials.hpp"
#include "gazatu-rf.hpp"
#include "lilygo.hpp"
#include "pin_config.hpp"
#include "ui_label.hpp"
#include "ui_loading_bar.hpp"
#include "ui_scrolling_text.hpp"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Arduino_GFX_Library.h>
#include <OneButton.h>
#include <WiFi.h>

#define LCD_IPS true
#define LCD_WIDTH 320
#define LCD_HEIGHT 170
#define LCD_ROTATION 1

Arduino_ESP32PAR8Q bus{PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_WR, PIN_LCD_RD, PIN_LCD_D0, PIN_LCD_D1, PIN_LCD_D2, PIN_LCD_D3,
    PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7};
Arduino_ST7789 gfx{&bus, PIN_LCD_RST, 0, LCD_IPS, LCD_HEIGHT, LCD_WIDTH, 35, 0, 35, 0};

ui_scrolling_text textarea{&gfx, 4, 32, LCD_WIDTH - 4 - 4, LCD_HEIGHT - 32 - 4};
ui_label loadingbar_label{&gfx, 4, 4};
ui_loading_bar loadingbar{&gfx, 4, 14, LCD_WIDTH - 4 - 4, 8};

OneButton button2{PIN_BUTTON_2};

AsyncRadio<SX1262> radio{new Module(PIN_SX1262_CS, PIN_SX1262_DIO1, PIN_SX1262_RST, PIN_SX1262_BUSY)};

int16_t setupRadio(void) {
  constexpr float FREQ = 869.47; // frequency in MHz
  constexpr float BW = 20.8;     // bandwidth in kHz (10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125)
  constexpr uint8_t SF = 10;     // spreading factor (6 to 12)
  constexpr uint8_t CR = 6;      // coding rate (5 to 8, error correction)
  constexpr uint8_t SYNC = 27;   // sync word
  constexpr uint8_t POWER = 22;  // output power in dBm
  constexpr uint8_t PRELEN = 8;  // preamble length

  int16_t rc = radio.begin(FREQ, BW, SF, CR, SYNC, POWER, PRELEN);

  if (rc == RADIOLIB_ERR_NONE) {
    radio.setCurrentLimit(125);
    radio.setRxBoostedGainMode(true);
  }

  return rc;
}

bool sleeping = false;
void beginSleep() {
  sleeping = true;
  textarea.printf("sleep\n");

  enableDisplay(false);

  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON_2, LOW);
  esp_light_sleep_start();

  enableDisplay(true);

  textarea.printf("sleep done\n");
  sleeping = false;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);
  Serial.printf("booted\n");

  enableDisplay();

  gfx.begin();
  gfx.setRotation(LCD_ROTATION);
  gfx.fillScreen(BLACK);
  gfx.setTextSize(1);
  gfx.setTextColor(WHITE, BLACK);

  textarea.printf("booted");

  textarea.printf("connecting to wifi...\n");
  WiFi.setHostname("lilygo");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    textarea.printf("failed to connect to wifi\n");
  }

  textarea.printf("local ipv4 = %s\n", WiFi.localIP().toString().c_str());

  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.begin();
    ArduinoOTA.onStart([]() {
      loadingbar_label.printf("updating...");
      loadingbar.show();
    });
    ArduinoOTA.onProgress([](uint32_t progress, uint32_t max) {
      loadingbar.updateByValue(progress, max);
    });
  }

  button2.attachClick([]() {
    int16_t radioBeginState = setupRadio();
    textarea.printf("RF %d\n", radioBeginState);

    int16_t radioListenState = radio.listen();
    textarea.printf("RX %d\n", radioListenState);
  });

  // button2.attachLongPressStop([]() {
  //   beginSleep();
  // });

  radio.onTransmit([]() {
    textarea.printf("[SX1262] transmit done at %fbps\n", radio.getDataRate());
  });

  radio.onPreamble([]() {
    textarea.printf("[SX1262] preamble\n");
  });

  radio.onPacket([]() {
    int32_t rssi = radio.getRSSI();

    textarea.printf("RSSI: %d dBM\n", rssi);

    rf::handle(radio,
        rf::handlers{
            [](std::monostate&) {
              // ignore
            },
            [](rf::SignalStrengthRequest&) {
              textarea.printf("[SX1262] Sending Answer...\n");

              int32_t rssi = radio.getRSSI();
              float snr = radio.getSNR();
              int32_t ferr = radio.getFrequencyError();

              delay(500);

              rf::SignalStrengthResponse response;
              response.rssi = rssi * -1;
              response.snr = snr;
              response.ferr = ferr / 100;

              rf::transmit(radio, response);
            },
            [](rf::SignalStrengthResponse&) {
              // ignore
            },
        });
  });
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  }

  button2.tick();

  radio.tick();
}

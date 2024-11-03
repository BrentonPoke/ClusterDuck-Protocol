#pragma once
#ifdef ARDUINO_HELTEC_WIRELESS_STICK
#define CDPCFG_PIN_BAT 1
//#define CDPCFG_BAT_MULDIV 200 / 100
#define CDPCFG_PIN_VEXT 36
#define CDPCFG_PIN_LED1 35
#define CDPCFG_OLED_64x32

// LoRa configuration
#define CDPCFG_PIN_LORA_CS 8
#define CDPCFG_PIN_LORA_DIO0 14
#define CDPCFG_PIN_LORA_DIO1 14
#define CDPCFG_PIN_LORA_RST 12
#define CDPCFG_PIN_LORA_BUSY 13

//GPS configuration
//#define CDPCFG_GPS_RX 34
//#define CDPCFG_GPS_TX 12

// OLED display settings
#define CDPCFG_PIN_OLED_CLOCK 18
#define CDPCFG_PIN_OLED_DATA 17
#define CDPCFG_PIN_OLED_RESET 21
#define CDPCFG_PIN_OLED_ROTATION U8G2_R0
#endif
#include <Arduino.h>
#include <SPI.h>
#include <Ticker.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Esp.h>
#include <esp_wifi.h>
#include <esp_system.h>

#include <my-log.h>

// Includes for BLE
#include <NimBLEUtils.h>
#include <NimBLEServer.h>
#include <NimBLEDevice.h>
#include <NimBLEAdvertising.h>
#include <Preferences.h>

#include <nvs.h>
#include <nvs_flash.h>

#include <ArduinoJson.h>

void blinkLED(void);

void init_wifi(void);
bool init_ntp(void);
void try_get_time(void);
void get_preferences(void);
void create_name();
void wifi_event_callback(WiFiEvent_t wifi_event);

extern char g_ap_name[];
extern String g_ssid_prim;
extern String g_ssid_sec;
extern String g_pw_prim;
extern String g_pw_sec;
extern bool g_has_credentials;
extern bool conn_status_changed;
extern volatile bool isConnected;
extern WiFiMulti wifi_multi;

void init_ble();
void start_ble(void);
void start_ble_adv(void);
void stop_ble_adv(void);

extern bool g_ble_is_on;

extern char g_sw_version[];
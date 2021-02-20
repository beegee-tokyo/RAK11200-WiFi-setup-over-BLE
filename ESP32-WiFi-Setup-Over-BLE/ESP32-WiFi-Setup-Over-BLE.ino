#include "main.h"

/** SW version of the gateway */
char g_sw_version[10]; // = SW_VERSION "." SW_MINOR;
/** Compilation date of the SW */
const char compile_date[] = __DATE__ " " __TIME__;

/** Arduino setup function, called once after reboot */
void setup(void)
{
  Serial.begin(115200);
  Serial.println("=====================================");
  Serial.println("ESP32 WiFi setup over BLE");
  Serial.println("=====================================");
  sprintf(g_sw_version, "%s", String(SW_VERSION).c_str());
  Serial.printf("Version: %s\n", g_sw_version);
  Serial.printf("Compiled: %s\n", compile_date);
  Serial.printf("SDK version: %s\n", ESP.getSdkVersion());

  // Initialize WiFi
  init_wifi();

  // Initialize BLE
  init_ble();

  Serial.println("Starting loop");
}

/** Counter for WiFi failure to connect */
uint8_t no_ap_found = 0;

/**
   Arduino main task
   Checking WiFi connectivity and
   request for OTA update
*/
void loop(void)
{
  // It makes only sense to check WiFi connection if we have WiFi credentials
  if (g_has_credentials)
  {
    uint8_t wifi_scan_result = wifi_multi.run(15000U);
    if (wifi_scan_result != WL_CONNECTED)
    {
      if (wifi_scan_result == WL_NO_SSID_AVAIL)
      {
        no_ap_found++;
        if (no_ap_found >= 50)
        {
          Serial.printf("WiFi not connected! Error %d\n", wifi_scan_result);
          // stored APs not found
          no_ap_found = 0;
        }
      }
      else
      {
        // retry in a second
        delay(1000);
      }
    }
  }
  delay(100);
}

#include "main.h"

/** Unique device name */
char g_ap_name[] = "RAK-xxxxxxxxxxxx";
// const char *devAddr;
uint8_t dev_addr_array[8];


/** Flag if stored AP credentials are available */
bool g_has_credentials = false;
/** Connection status */
volatile bool isConnected = false;
/** Connection change status */
bool conn_status_changed = false;

/** WiFiMulti class */
WiFiMulti wifi_multi;

/** SSID of primary WiFi networks */
String g_ssid_prim;
/** SSID of secondary WiFi networks */
String g_ssid_sec;
/** Password for primary local WiFi network */
String g_pw_prim;
/** Password for secondary local WiFi network */
String g_pw_sec;

/**
   @brief Callback for WiFi events
   @param event
  			WiFi event
*/
void wifi_event_callback(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);
  IPAddress local_ip;
  switch (event)
  {
    case SYSTEM_EVENT_STA_GOT_IP:
      conn_status_changed = true;

      local_ip = WiFi.localIP();
      Serial.printf("Connected to AP: %s with IP: %d.%d.%d.%d RSSI: %d\n",
                    WiFi.SSID().c_str(),
                    local_ip[0], local_ip[1], local_ip[2], local_ip[3],
                    WiFi.RSSI());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      conn_status_changed = true;
      Serial.println("WiFi lost connection");
      if (!g_ble_is_on)
      {
        start_ble_adv();
      }
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      Serial.println("WiFi scan finished");
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("WiFi STA connected");
      break;
    case SYSTEM_EVENT_WIFI_READY:
      Serial.println("WiFi interface ready");
      break;
    case SYSTEM_EVENT_STA_START:
      Serial.println("WiFi STA started");
      break;
    case SYSTEM_EVENT_STA_STOP:
      Serial.println("WiFi STA stopped");
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      Serial.println("Authentication mode of access point has changed");
      break;
    case SYSTEM_EVENT_STA_LOST_IP:
      Serial.println("Lost IP address and IP address is reset to 0");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
      Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
      break;
    default:
      break;
  }
}

/**
   Get stored data from the preferences
*/
void get_preferences(void)
{
  Preferences preferences;
  preferences.begin("WiFiCred", false);
  bool hasPref = preferences.getBool("valid", false);
  if (hasPref)
  {
    g_ssid_prim = preferences.getString("ssidPrim", "");
    g_ssid_sec = preferences.getString("ssidSec", "");
    g_pw_prim = preferences.getString("pwPrim", "");
    g_pw_sec = preferences.getString("pwSec", "");

    if (g_ssid_prim.equals("") || g_pw_prim.equals("") || g_ssid_sec.equals("") || g_pw_sec.equals(""))
    {
      Serial.println("Found preferences but credentials are invalid");
    }
    else
    {
      Serial.printf("Preferences: primary SSID: %s pw: %s secondary SSID: %s pw: %s\n",
                    g_ssid_prim.c_str(), g_pw_prim.c_str(),
                    g_ssid_sec.c_str(), g_pw_sec.c_str());
      g_has_credentials = true;
    }
  }
  else
  {
    Serial.println("Could not find preferences, need send data over BLE");
  }
  preferences.end();
}

/**
   Create unique device name from MAC address
 **/
void create_name(void)
{
  // Get MAC address for WiFi station
  esp_wifi_get_mac(WIFI_IF_STA, dev_addr_array);

  // Write unique name into apName
  sprintf(g_ap_name, "RAK-%02X%02X%02X%02X%02X%02X",
          dev_addr_array[0], dev_addr_array[1],
          dev_addr_array[2], dev_addr_array[3],
          dev_addr_array[4], dev_addr_array[5]);
  Serial.printf("Device name: %s\n", g_ap_name);
}

/**
   Initialize WiFi
   - Check if WiFi credentials are stored in the preferences
   - Create unique device name
   - Register WiFi event callback function
   - Try to connect to WiFi if credentials are available
*/
void init_wifi(void)
{
  get_preferences();

  if (!g_has_credentials)
  {
    return;
  }

  WiFi.disconnect(true);
  delay(100);
  WiFi.enableSTA(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.onEvent(wifi_event_callback);

  create_name();

  if (g_has_credentials)
  {
    // Using WiFiMulti to connect to best AP
    wifi_multi.addAP(g_ssid_prim.c_str(), g_pw_prim.c_str());
    wifi_multi.addAP(g_ssid_sec.c_str(), g_pw_sec.c_str());
  }
}

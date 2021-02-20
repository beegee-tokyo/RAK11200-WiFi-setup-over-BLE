#include "main.h"

// List of Service and Characteristic UUIDs
/** Service UUID for WiFi settings */
#define SERVICE_UUID "0000aaaa-ead2-11e7-80c1-9a214cf093ae"
/** Characteristic UUID for WiFi settings */
#define WIFI_UUID "00005555-ead2-11e7-80c1-9a214cf093ae"
/** Service UUID for Uart */
#define UART_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
/** Characteristic UUID for receiver */
#define RX_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
/** Characteristic UUID for transmitter */
#define TX_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

/** Characteristic for digital output */
BLECharacteristic *wifi_characteristic;
/** Characteristic for BLE-UART TX */
BLECharacteristic *uart_tx_characteristic;
/** Characteristic for BLE-UART RX */
BLECharacteristic *uart_rx_characteristic;
/** BLE Advertiser */
BLEAdvertising *ble_advertising;
/** BLE Service for WiFi*/
BLEService *wifi_service;
/** BLE Service for Uart*/
BLEService *uart_service;
/** BLE Server */
BLEServer *ble_server;

/** Buffer for JSON string */
// Max size 79 bytes for frame:
// {"ssidPrim":"","pwPrim":"","ssidSec":"","pwSec":"","ip":"","ap":"","progress":}
// + 4 x 32 bytes for 2 SSID's and 2 passwords
// + 15 bytes for local IP
// + 32 bytes for WiFi SSID
// + 3 bytes for progress
// ==> 257 bytes !! LETS MAKE IT 512 !!
StaticJsonDocument<512> json_buffer;

/** Flag used to detect if a BLE advertising is enabled */
bool g_ble_is_on = false;
/** Flag if device is connected */
bool g_ble_connected = false;

/**
 * Callbacks for client connection and disconnection
 */
class MyServerCallbacks : public BLEServerCallbacks
{
	/**
	 * Callback when a device connects
	 * @param ble_server
	 * 			Pointer to server that was connected
	 */
	void onConnect(BLEServer *ble_server)
	{
		myLog_d("BLE client connected");
		g_ble_connected = true;
	};

	/**
	 * Callback when a device disconnects
	 * @param ble_server
	 * 			Pointer to server that was disconnected
	 */
	void onDisconnect(BLEServer *ble_server)
	{
		myLog_d("BLE client disconnected");
		g_ble_connected = false;
		ble_advertising->start();
	}
};

/**
 * Callbacks for BLE client read/write requests
 * on WiFi characteristic
 */
class WiFiCallBackHandler : public BLECharacteristicCallbacks
{
	/**
	 * Callback for write request on WiFi characteristic
	 * @param pCharacteristic
	 * 			Pointer to the characteristic
	 */
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		std::string rx_value = pCharacteristic->getValue();
		if (rx_value.length() == 0)
		{
			myLog_d("Received empty characteristic value");
			return;
		}

		// Decode data
		int key_index = 0;
		for (int index = 0; index < rx_value.length(); index++)
		{
			rx_value[index] = (char)rx_value[index] ^ (char)g_ap_name[key_index];
			key_index++;
			if (key_index >= strlen(g_ap_name))
				key_index = 0;
		}

		for (int idx = 0; idx < rx_value.length(); idx++)
		{
			Serial.printf("%c", rx_value[idx]);
		}
		Serial.println("");

		/** Json object for incoming data */
		auto json_error = deserializeJson(json_buffer, (char *)&rx_value[0]);
		if (json_error == 0)
		{
			if ((json_buffer.containsKey("ssidPrim") &&
				 json_buffer.containsKey("pwPrim") &&
				 json_buffer.containsKey("ssidSec") &&
				 json_buffer.containsKey("pwSec")) ||
				(json_buffer.containsKey("ssidPrim") &&
				 json_buffer.containsKey("pwPrim") &&
				 json_buffer.containsKey("ssidSec") &&
				 json_buffer.containsKey("pwSec")))
			{
				g_ssid_prim = json_buffer["ssidPrim"].as<String>();
				g_pw_prim = json_buffer["pwPrim"].as<String>();
				g_ssid_sec = json_buffer["ssidSec"].as<String>();
				g_pw_sec = json_buffer["pwSec"].as<String>();

				Preferences preferences;
				preferences.begin("WiFiCred", false);
				preferences.putString("ssidPrim", g_ssid_prim);
				preferences.putString("ssidSec", g_ssid_sec);
				preferences.putString("pwPrim", g_pw_prim);
				preferences.putString("pwSec", g_pw_sec);
				preferences.putBool("valid", true);
				if (json_buffer.containsKey("lora"))
				{
					preferences.putShort("lora", json_buffer["lora"].as<byte>());
				}
				preferences.end();

				myLog_d("Received over Bluetooth:");
				myLog_d("primary SSID: %s password %s", g_ssid_prim.c_str(), g_pw_prim.c_str());
				myLog_d("secondary SSID: %s password %s", g_ssid_sec.c_str(), g_pw_sec.c_str());
				conn_status_changed = true;
				g_has_credentials = true;
				WiFi.disconnect(true, true);
				delay(1000);
				init_wifi();
			}
			else if (json_buffer.containsKey("erase"))
			{
				myLog_d("Received erase command");
				WiFi.disconnect(true, true);

				Preferences preferences;
				preferences.begin("WiFiCred", false);
				preferences.clear();
				preferences.end();
				conn_status_changed = true;
				g_has_credentials = false;
				g_ssid_prim = "";
				g_pw_prim = "";
				g_ssid_sec = "";
				g_pw_sec = "";

				int err = nvs_flash_init();
				myLog_d("nvs_flash_init: %d", err);
				err = nvs_flash_erase();
				myLog_d("nvs_flash_erase: %d", err);

				esp_restart();
			}
			else if (json_buffer.containsKey("reset"))
			{
				WiFi.disconnect();
				esp_restart();
			}
		}
		else
		{
			myLog_e("Received invalid JSON");
		}
		json_buffer.clear();
	};

	/**
	 * Callback for read request on WiFi characteristic
	 * @param pCharacteristic
	 * 			Pointer to the characteristic
	 */
	void onRead(BLECharacteristic *pCharacteristic)
	{
		myLog_v("BLE onRead request");
		String wifi_credentials;

		/** Json object for outgoing data */
		StaticJsonDocument<256> json_out;
		json_out["ssidPrim"] = g_ssid_prim;
		json_out["pwPrim"] = g_pw_prim;
		json_out["ssidSec"] = g_ssid_sec;
		json_out["pwSec"] = g_pw_sec;
		if (WiFi.isConnected())
		{
			json_out["ip"] = WiFi.localIP().toString();
			json_out["ap"] = WiFi.SSID();
		}
		else
		{
			json_out["ip"] = "0.0.0.0";
			json_out["ap"] = "";
		}

		json_out["sw"] = g_sw_version;

		// Convert JSON object into a string
		serializeJson(json_out, wifi_credentials);

		// encode the data
		int key_index = 0;
		myLog_d("Stored settings: %s", wifi_credentials.c_str());
		for (int index = 0; index < wifi_credentials.length(); index++)
		{
			wifi_credentials[index] = (char)wifi_credentials[index] ^ (char)g_ap_name[key_index];
			key_index++;
			if (key_index >= strlen(g_ap_name))
				key_index = 0;
		}
		wifi_characteristic->setValue((uint8_t *)&wifi_credentials[0], wifi_credentials.length());
		json_buffer.clear();
	}
};

/**
 * Callbacks for BLE client read/write requests
 * on WiFi characteristic
 */
class UartCallBackHandler : public BLECharacteristicCallbacks
{
	/**
	 * Callback for write request on UART characteristic
	 * @param pCharacteristic
	 * 			Pointer to the characteristic
	 */
	void onWrite(BLECharacteristic *ble_characteristic)
	{
		std::string rx_value = ble_characteristic->getValue();

		if (rx_value.length() > 0)
		{
			myLog_d("Received Value: %s", rx_value);
		}
	}
};

/**
 * Initialize BLE service and characteristic
 * Start BLE server and service advertising
 */
void init_ble()
{
	myLog_v("Initialize BLE");
	// Initialize BLE and set output power
	BLEDevice::init(g_ap_name);
	BLEDevice::setPower(ESP_PWR_LVL_P7);

	BLEAddress thisAddress = BLEDevice::getAddress();

	myLog_v("BLE address: %s\n", thisAddress.toString().c_str());

	// Create BLE Server
	ble_server = BLEDevice::createServer();

	// Set server callbacks
	ble_server->setCallbacks(new MyServerCallbacks());

	// Create WiFi BLE Service
	wifi_service = ble_server->createService(BLEUUID(SERVICE_UUID), 20);

	// Create BLE Characteristic for WiFi settings
	wifi_characteristic = wifi_service->createCharacteristic(
		BLEUUID(WIFI_UUID),
		NIMBLE_PROPERTY::READ |
			NIMBLE_PROPERTY::WRITE);
	wifi_characteristic->setCallbacks(new WiFiCallBackHandler());

	// Start the service
	wifi_service->start();

	// Create the UART BLE Service
	uart_service = ble_server->createService(UART_UUID);

	// Create a BLE Characteristic
	uart_tx_characteristic = uart_service->createCharacteristic(
		TX_UUID,
		NIMBLE_PROPERTY::NOTIFY);

	uart_rx_characteristic = uart_service->createCharacteristic(
		RX_UUID,
		NIMBLE_PROPERTY::WRITE);

	uart_rx_characteristic->setCallbacks(new UartCallBackHandler());

	// Start the service
	uart_service->start();

	// Start advertising
	ble_advertising = ble_server->getAdvertising();
	ble_advertising->addServiceUUID(SERVICE_UUID);
	ble_advertising->addServiceUUID(UART_UUID);
	start_ble_adv();
}

/**
 * Stop BLE advertising
 */
void stop_ble_adv(void)
{
	/// \todo needs patch in BLEAdvertising.cpp -> handleGAPEvent() -> remove start(); from ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT
	ble_advertising->stop();
	g_ble_is_on = false;
}

/**
 * Start BLE advertising
 */
void start_ble_adv(void)
{
	ble_advertising->start();
	g_ble_is_on = true;
}

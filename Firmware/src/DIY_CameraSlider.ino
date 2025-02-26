#include <ESPmDNS.h>
#include <WiFiManager.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"
#include <FlexyStepper.h>
#include "config_cameraslider.h"
#include "config_wifi.h"
#include "include/PersistSettings.h"

WiFiManager wifiManager;
AsyncWebServer server(80);

// We will use persistent storage to store our Camera Slider configuration
// but also allow us to edit it trough web/api
// This way we can decouple firmware from electronics/mech changes
// i.e We can change rail lenght or motor direction without recompiling firmware
// Note: You should not edit config below. Instead modify defaults inside `config_cameraslider.h`
struct SliderConfigStruct
{
    static const unsigned int Version = 1;

    uint16_t rail_length = RAIL_LENGTH_MM;
    uint16_t min_slider_step = MIN_STEP_SLIDER;


    int homing_direction = -1;  // Specify if we should reverse homing direction
    int slider_direction = 1;   // Increase (1) or decrease(-1) steps to get positive movement
    int rotate_direction = 1;   // Increase (1) or decrease(-1) steps to get positive movement

    uint16_t slide_steps_per_mm = SLIDE_STEPS_PER_MM;
    uint16_t pan_steps_per_degree = PAN_STEPS_PER_DEGREE;

    uint16_t homing_speed_slide = DEFAULT_HOMING_SPEED_SLIDE;
    uint16_t homing_speed_pan   = DEFAULT_HOMING_SPEED_PAN;

    float default_slider_speed = DEFAULT_SLIDE_TO_POS_SPEED;
    float default_slider_accel = DEFAULT_SLIDE_TO_POS_ACCEL;
    float default_rotate_speed = DEFAULT_ROTATE_TO_POS_SPEED;
    float default_rotate_accel = DEFAULT_ROTATE_TO_POS_ACCEL;
};

PersistSettings<SliderConfigStruct> SliderConfig(SliderConfigStruct::Version);

void setup()
{
    // Configure Serial communication
    Serial.begin(115200);
    delay(2000);
    Serial.setDebugOutput(true);
    Serial.println("DIY Camera Slider");

    WiFi.mode(WIFI_STA);

    WiFi.setHostname(MDNS_NAME);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);

    int txPower = WiFi.getTxPower();
    Serial.print("TX power: ");
    Serial.println(txPower);

    delay(1000);

    // Peristent device config
    SliderConfig.Begin();
    if( SliderConfig.Valid() )
    {
        Serial.println("Reloading camera slider settings.");
    }
    else
    {
        Serial.println("Camera settings invalid. Resetting to default.");
    }

    // Configure and initialize GPIOs
	pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_MTR_nRST, OUTPUT);
	pinMode(PIN_MTR_nEN, OUTPUT);
	pinMode(PIN_END_SWICH_X, INPUT_PULLUP);
	digitalWrite(PIN_LED, HIGH);
	digitalWrite(PIN_MTR_nRST, HIGH);
	digitalWrite(PIN_MTR_nEN, LOW);

	delay(1000);

	// Connect to WiFi
    bool res;
    res = wifiManager.autoConnect("SK-DIY-CameraSlider");

    wifiManager.setHostname(MDNS_NAME);
    wifiManager.setConnectRetries(4);

    if(!res) {
        Serial.println("Failed to connect or hit timeout");
        ESP.restart();
    }
    else {
        //if you get here you have connected to the WiFi
        Serial.println("Connected.");
    }

    // Initialize LittleFS
    Serial.print("Starting LittleFS...");
    if(!LittleFS.begin()){
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }
    Serial.println("done.");

	Serial.print("WiFi IP: ");
	Serial.println(WiFi.localIP());

    if (!MDNS.begin(MDNS_NAME))
    {
        Serial.println("Error starting mDNS");
    }
    else
    {
        Serial.println((String) "mDNS http://" + MDNS_NAME + ".local");
    }

	// Setup motors
	setupMotors();
	CameraSlider_EnableMotors(true);
	digitalWrite(PIN_LED, LOW);

    // Initialize Web server
	setupWebServer();
	server.begin();

    // Debug message to signal we are initialized and entering loop
	Serial.println("Ready to go.");
}


void loop()
{
	while(1)
	{
		CameraSlider_tick();
	}
}

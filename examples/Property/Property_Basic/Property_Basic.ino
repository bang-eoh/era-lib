/*************************************************************
  Download latest ERa library here:
    https://github.com/eoh-jsc/era-lib/releases/latest
    https://www.arduino.cc/reference/en/libraries/era
    https://registry.platformio.org/libraries/eoh-ltd/ERa/installation

    ERa website:                https://e-ra.io
    ERa blog:                   https://iotasia.org
    ERa forum:                  https://forum.eoh.io
    Follow us:                  https://www.fb.com/EoHPlatform
 *************************************************************/

// Enable debug console
// Set CORE_DEBUG_LEVEL = 3 first
// #define ERA_DEBUG
// #define ERA_SERIAL Serial

/* Select ERa host location (VN: Viet Nam, SG: Singapore) */
#define ERA_LOCATION_VN
// #define ERA_LOCATION_SG

// You should get Auth Token in the ERa App or ERa Dashboard
#define ERA_AUTH_TOKEN "ERA2706"

#include <Arduino.h>
#include <ERa.hpp>
#include <ERa/ERaTimer.hpp>

#define LED_PIN  2

const char ssid[] = "YOUR_SSID";
const char pass[] = "YOUR_PASSWORD";

ERaTimer timer;
bool state = false;
unsigned long timeUp = 0;

/* This function print uptime every second */
void timerEvent() {
    timeUp = ERaMillis() / 1000L;
    ERA_LOG("Timer", "Uptime: %d", timeUp);
}

void setup() {
    /* Setup debug console */
    Serial.begin(115200);

    /* Setup pin mode led pin */
    pinMode(LED_PIN, OUTPUT);

    /* Set board id */
    // ERa.setBoardID("Board_1");

    /* Setup property virtual pin associate with variable */
    ERa.addPropertyVirtual(V0, state, PermissionT::PERMISSION_READ_WRITE).onUpdate([]() {
        digitalWrite(LED_PIN, state);
    });
    ERa.addPropertyVirtual(V1, timeUp, PermissionT::PERMISSION_READ).publishOnChange(1);

    /* Initializing the ERa library. */
    ERa.begin(ssid, pass);

    /* Setup timer called function every second */
    timer.setInterval(1000L, timerEvent);
}

void loop() {
    ERa.run();
    timer.run();
}

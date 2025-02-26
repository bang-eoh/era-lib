#ifndef INC_ERA_CONFIG_HPP_
#define INC_ERA_CONFIG_HPP_

#if !defined(ERA_MODEL_TYPE)
    #define ERA_MODEL_TYPE              "ERa"
#endif

#include <ERa/ERaVersion.hpp>

#define MAX_TOPIC_LENGTH                100
#define BASE_TOPIC_MQTT                 "eoh/chip"

#define ERA_SSL_PORT                    443
#define ERA_SSL_DOMAIN                  "backend.eoh.io"
#define ERA_NOSSL_PORT                  80
#define ERA_NOSSL_DOMAIN                "nossl-backend.eoh.io"

#define ERA_MQTT_PORT_SSL               8883
#define ERA_MQTT_PORT_NOSSL             1883

#if defined(ERA_AUTH_TOKEN)
    #define ERA_AUTHENTICATION_TOKEN    ERA_AUTH_TOKEN
#else
    #define ERA_AUTHENTICATION_TOKEN    ""
#endif

#if defined(DEFAULT_MQTT_CLIENT_ID)
    #define ERA_MQTT_CLIENT_ID          DEFAULT_MQTT_CLIENT_ID
#elif defined(ERA_BOARD_ID)
    #define ERA_MQTT_CLIENT_ID          ERA_BOARD_ID
#elif defined(ERA_AUTH_TOKEN)
    #define ERA_MQTT_CLIENT_ID          ERA_AUTH_TOKEN
#else
    #define ERA_MQTT_CLIENT_ID          ""
#endif

#if defined(DEFAULT_MQTT_HOST)
    #define ERA_MQTT_HOST               DEFAULT_MQTT_HOST
#else
    #if defined(ERA_LOCATION_VN)
        #define ERA_MQTT_HOST           "mqtt1.eoh.io"
    #else
        #define ERA_MQTT_HOST           "remarkable-accountant.cloudmqtt.com"
    #endif
#endif

#if defined(DEFAULT_MQTT_PORT)
    #define ERA_MQTT_PORT               DEFAULT_MQTT_PORT
#else
    #if defined(ERA_MQTT_SSL)
        #define ERA_OTA_SSL
        #define ERA_MQTT_PORT           ERA_MQTT_PORT_SSL
    #else
        #define ERA_MQTT_PORT           ERA_MQTT_PORT_NOSSL
    #endif
#endif

#if defined(DEFAULT_MQTT_USERNAME)
    #define ERA_MQTT_USERNAME           DEFAULT_MQTT_USERNAME
#elif defined(ERA_AUTH_TOKEN)
    #define ERA_MQTT_USERNAME           ERA_AUTH_TOKEN
#else
    #define ERA_MQTT_USERNAME           ""
#endif

#if defined(DEFAULT_MQTT_PASSWORD)
    #define ERA_MQTT_PASSWORD           DEFAULT_MQTT_PASSWORD
#elif defined(ERA_AUTH_TOKEN)
    #define ERA_MQTT_PASSWORD           ERA_AUTH_TOKEN
#else
    #define ERA_MQTT_PASSWORD           ""
#endif

#if defined(DEFAULT_MQTT_KEEP_ALIVE)
    #define ERA_MQTT_KEEP_ALIVE         DEFAULT_MQTT_KEEP_ALIVE
#else
    #define ERA_MQTT_KEEP_ALIVE         60
#endif

#if defined(DEFAULT_MQTT_SUBSCRIBE_QOS)
    #define ERA_MQTT_SUBSCRIBE_QOS      DEFAULT_MQTT_SUBSCRIBE_QOS
#else
    #define ERA_MQTT_SUBSCRIBE_QOS      0
#endif

#if defined(DEFAULT_MQTT_PUBLISH_QOS)
    #define ERA_MQTT_PUBLISH_QOS        DEFAULT_MQTT_PUBLISH_QOS
#else
    #define ERA_MQTT_PUBLISH_QOS        0
#endif

#if defined(DEFAULT_MQTT_PUBLISH_RETAINED)
    #define ERA_MQTT_PUBLISH_RETAINED   DEFAULT_MQTT_PUBLISH_RETAINED
#else
    #define ERA_MQTT_PUBLISH_RETAINED   true
#endif

#if defined(DEFAULT_SOCKET_TIMEOUT)
    #define ERA_SOCKET_TIMEOUT          DEFAULT_SOCKET_TIMEOUT
#else
    #define ERA_SOCKET_TIMEOUT          1
#endif

#if !defined(ERA_DISABLE_SYNC_CONFIG)
    #define ERA_ASK_CONFIG_WHEN_RESTART
#endif

#if !defined(ERA_PROTO_TYPE)
    #define ERA_PROTO_TYPE              "WiFi"
#endif

#define LIMIT_CONNECT_BROKER_MQTT       10

#define INFO_BOARD                      "board"
#define INFO_MODEL                      "model"
#define INFO_BOARD_ID                   "board_id"
#define INFO_AUTH_TOKEN                 "auth_token"
#define INFO_FIRMWARE_VERSION           "firmware_version"
#define INFO_SSID                       "ssid"
#define INFO_BSSID                      "bssid"
#define INFO_RSSI                       "rssi"
#define INFO_MAC                        "mac"
#define INFO_LOCAL_IP                   "ip"
#define INFO_PING                       "ping"

#define INFO_MB_CHIP_TEMPERATURE        "chip_temperature"
#define INFO_MB_TEMPERATURE             "temperature"
#define INFO_MB_VOLTAGE                 "voltage"
#define INFO_MB_IS_BATTERY              "is_battery"
#define INFO_MB_RSSI                    "rssi"
#define INFO_MB_WIFI_USING              "wifi_is_using"

#include <ERa/ERaDetect.hpp>

#endif /* INC_ERA_CONFIG_HPP_ */

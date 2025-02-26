#ifndef INC_ERA_PARTICLE_CLIENT_HPP_
#define INC_ERA_PARTICLE_CLIENT_HPP_

#if !defined(ERA_PROTO_TYPE)
    #define ERA_PROTO_TYPE            "Particle"
#endif

#include "application.h"

#undef retained

#include <ERa/ERaProtocol.hpp>
#include <MQTT/ERaMqtt.hpp>

class ERaFlash;

template <class Transport>
class ERaParticle
    : public ERaProto<Transport, ERaFlash>
{
    const char* TAG = "Particle";
    friend class ERaProto<Transport, ERaFlash>;
    typedef ERaProto<Transport, ERaFlash> Base;

public:
    ERaParticle(Transport& _transp, ERaFlash& _flash)
        : Base(_transp, _flash)
        , authToken(nullptr)
    {}
    ~ERaParticle()
    {}

    void config(const char* auth,
                const char* host = ERA_MQTT_HOST,
                uint16_t port = ERA_MQTT_PORT,
                const char* username = ERA_MQTT_USERNAME,
                const char* password = ERA_MQTT_PASSWORD) {
        Base::begin(auth);
        this->getTransp().setClient(&this->client);
        this->getTransp().config(host, port, username, password);
    }

    void begin(const char* auth,
                const char* host = ERA_MQTT_HOST,
                uint16_t port = ERA_MQTT_PORT,
                const char* username = ERA_MQTT_USERNAME,
                const char* password = ERA_MQTT_PASSWORD) {
        Base::init();
        this->config(auth, host, port, username, password);
        this->getTransp().setSSID(ERA_PROTO_TYPE);
        Base::connect();
    }

    void begin() {
        this->begin(ERA_AUTHENTICATION_TOKEN,
                    ERA_MQTT_HOST, ERA_MQTT_PORT,
                    ERA_MQTT_USERNAME, ERA_MQTT_PASSWORD);
    }

protected:
private:
    TCPClient client;
    const char* authToken;
};

template <class Proto, class Flash>
inline
void ERaApi<Proto, Flash>::addInfo(cJSON* root) {
    cJSON_AddStringToObject(root, INFO_BOARD, ERA_BOARD_TYPE);
    cJSON_AddStringToObject(root, INFO_MODEL, ERA_MODEL_TYPE);
	cJSON_AddStringToObject(root, INFO_BOARD_ID, this->thisProto().getBoardID());
	cJSON_AddStringToObject(root, INFO_AUTH_TOKEN, this->thisProto().getAuth());
    cJSON_AddStringToObject(root, INFO_FIRMWARE_VERSION, ERA_FIRMWARE_VERSION);
    cJSON_AddStringToObject(root, INFO_SSID, ERA_PROTO_TYPE);
    cJSON_AddStringToObject(root, INFO_BSSID, ERA_PROTO_TYPE);
    cJSON_AddNumberToObject(root, INFO_RSSI, 100);
    cJSON_AddStringToObject(root, INFO_MAC, ERA_PROTO_TYPE);
    cJSON_AddStringToObject(root, INFO_LOCAL_IP, ERA_PROTO_TYPE);
    cJSON_AddNumberToObject(root, INFO_PING, this->thisProto().getTransp().getPing());
}

template <class Proto, class Flash>
inline
void ERaApi<Proto, Flash>::addModbusInfo(cJSON* root) {
	cJSON_AddNumberToObject(root, INFO_MB_CHIP_TEMPERATURE, 5000);
	cJSON_AddNumberToObject(root, INFO_MB_TEMPERATURE, 0);
	cJSON_AddNumberToObject(root, INFO_MB_VOLTAGE, 999);
	cJSON_AddNumberToObject(root, INFO_MB_IS_BATTERY, 0);
	cJSON_AddNumberToObject(root, INFO_MB_RSSI, 100);
	cJSON_AddStringToObject(root, INFO_MB_WIFI_USING, ERA_PROTO_TYPE);
}

#endif /* INC_ERA_PARTICLE_CLIENT_HPP_ */

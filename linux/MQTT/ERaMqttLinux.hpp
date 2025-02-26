#ifndef INC_ERA_MQTT_LINUX_HPP_
#define INC_ERA_MQTT_LINUX_HPP_

#include <Utility/ERaUtility.hpp>
#include <Utility/ERacJSON.hpp>
#include <ERa/ERaDefine.hpp>
#include <ERa/ERaConfig.hpp>
#include <ERa/ERaApi.hpp>
#include "MQTT/MQTT.hpp"

#if defined(__has_include) &&       \
    __has_include(<functional>) &&  \
    !defined(ERA_IGNORE_STD_FUNCTIONAL_STRING)
    #include <functional>
    #define MQTT_HAS_FUNCTIONAL_H
#endif

#define ERA_MQTT_PUB_LOG(status, errorCode)                                                     \
    if (status) {                                                                               \
        ERA_LOG(TAG, ERA_PSTR("Publish (ok) %s: %s"), topic, payload);                          \
    }                                                                                           \
    else {                                                                                      \
        ERA_LOG(TAG, ERA_PSTR("Publish (error %d) %s: %s"), errorCode, topic, payload);         \
    }

#define ERR_MQTT_SUB_LOG(status, errorCode)                                                     \
    if (status) {                                                                               \
        ERA_LOG(TAG, ERA_PSTR("Subscribe (ok): %s, QoS: %d"), topicName, qos);                  \
    }                                                                                           \
    else {                                                                                      \
        ERA_LOG(TAG, ERA_PSTR("Subscribe (error %d): %s, QoS: %d"), errorCode, topicName, qos); \
    }

using namespace std;

template <class MQTT>
class ERaMqttLinux
{
    enum QoST {
        QOS0 = 0x00,
        QOS1 = 0x01,
        QOS2 = 0x02,
        SUBFAIL = 0x80
    };
#if defined(MQTT_HAS_FUNCTIONAL_H)
    typedef std::function<void(const char*, const char*)> MessageCallback_t;
#else
    typedef void (*MessageCallback_t)(const char*, const char*);
#endif

    const char* TAG = "MQTT";
    const char* ONLINE_MESSAGE = R"json({"ol":1%s%s})json";
    const char* OFFLINE_MESSAGE = R"json({"ol":0})json";
    const char* WIFI_INFO = R"json(,"wifi_ssid":"%s")json";
    const char* ASK_CONFIG_INFO = R"json(,"ask_configuration":1)json";
    const char* LWT_TOPIC = "/is_online";
    const bool LWT_RETAINED = true;
    const int LWT_QOS = QoST::QOS1;

public:
    ERaMqttLinux()
        : mqtt(0)
        , host(ERA_MQTT_HOST)
        , port(ERA_MQTT_PORT)
        , clientID(ERA_MQTT_CLIENT_ID)
        , username(ERA_MQTT_USERNAME)
        , password(ERA_MQTT_PASSWORD)
        , ssid(NULL)
        , ping(0L)
        , signalQuality(0)
        , askConfig(false)
        , _connected(false)
        , mutex(NULL)
    {
        memset(this->willTopic, 0, sizeof(this->willTopic));
    }
    ~ERaMqttLinux()
    {}

    void begin(const char* _host, uint16_t _port,
            const char* _username, const char* _password) {
        this->config(_host, _port, _username, _password);
    }

    void init(int readBufSize = ERA_MQTT_RX_BUFFER_SIZE,
            int writeBufSize = ERA_MQTT_TX_BUFFER_SIZE) {
        this->mqtt.init(readBufSize, writeBufSize);
    }

    void config(const char* _host, uint16_t _port,
            const char* _username, const char* _password);
    bool connect();
    void disconnect();
    bool connected();
    bool run();
    bool publishData(const char* topic, const char* payload,
                    bool retained = ERA_MQTT_PUBLISH_RETAINED,
                    QoST qos = (QoST)ERA_MQTT_PUBLISH_QOS);
    bool syncConfig();

    void setTimeout(uint32_t timeout) {
        this->mqtt.setTimeout(timeout);
    }

    void setClientID(const char* id) {
        this->clientID = id;
    }

    const char* getClientID() const {
        return this->clientID;
    }

    void setTopic(const char* topic) {
        this->ERaTopic = topic;
    }

    void setAuth(const char* auth) {
        this->ERaAuth = auth;
    }

    const char* getAuth() const {
        return this->ERaAuth;
    }

    void setSSID(const char* _ssid) {
        this->ssid = _ssid;
    }

    const char* getSSID() const {
        return this->ssid;
    }

    MillisTime_t getPing() const {
        return this->ping;
    }

    void setSignalQuality(int16_t signal) {
        this->signalQuality = signal;
    }

    int16_t getSignalQuality() const {
        return this->signalQuality;
    }

    void setAskConfig(bool enable) {
        this->askConfig = enable;
    }

    bool getAskConfig() const {
        return this->askConfig;
    }

    const char* getTag() const {
        return this->TAG;
    }

    void onMessage(MessageCallback_t cb) {
        this->mqtt.onMessage(cb);
    }

protected:
private:
    bool subscribeTopic(const char* baseTopic, const char* topic,
                        QoST qos = (QoST)ERA_MQTT_SUBSCRIBE_QOS);
    bool publishLWT(bool sync = false);

    MQTT mqtt;
    const char* host;
    uint16_t port;
    const char* clientID;
    const char* username;
    const char* password;
    const char* ERaTopic;
    const char* ERaAuth;
    const char* ssid;
    MillisTime_t ping;
    int16_t signalQuality;
    bool askConfig;
    bool _connected;
    char willTopic[MAX_TOPIC_LENGTH];
    ERaMutex_t mutex;
};

template <class MQTT>
inline
void ERaMqttLinux<MQTT>::config(const char* _host, uint16_t _port,
                            const char* _username, const char* _password) {
    this->host = _host;
    this->port = _port;
    this->username = _username;
    this->password = _password;
    ClearArray(this->willTopic);
    FormatString(this->willTopic, this->ERaTopic);
    FormatString(this->willTopic, LWT_TOPIC);
    this->mqtt.init(ERA_MQTT_RX_BUFFER_SIZE,
                    ERA_MQTT_TX_BUFFER_SIZE);
    this->mqtt.setKeepAlive(ERA_MQTT_KEEP_ALIVE);
    this->mqtt.setWill(this->willTopic, OFFLINE_MESSAGE, LWT_RETAINED, LWT_QOS);
    this->mqtt.begin(this->host, this->port);
}

template <class MQTT>
inline
bool ERaMqttLinux<MQTT>::connect() {
    size_t count {0};
    char _clientID[74] {0};
    this->_connected = false;
    if (this->clientID != nullptr && strlen(this->clientID)) {
        FormatString(_clientID, this->ERaAuth);
        if (!ERaStrCmp(this->clientID, this->ERaAuth)) {
            FormatString(_clientID, "_%s", this->clientID);
        }
    }
    this->mqtt.disconnect();
    while (this->mqtt.connect(_clientID, this->username, this->password) == false) {
        ERA_LOG(TAG, ERA_PSTR("MQTT: connect failed (%d), retrying in 5 seconds"), this->mqtt.lastError());
        ERaDelay(5000);
        if (++count >= LIMIT_CONNECT_BROKER_MQTT) {
            return false;
        }
    }

#if defined(ERA_ZIGBEE)
    subscribeTopic(this->ERaTopic, "/zigbee/+/down");
    subscribeTopic(this->ERaTopic, "/zigbee/permit_to_join");
    subscribeTopic(this->ERaTopic, "/zigbee/remove_device");
#endif

    subscribeTopic(this->ERaTopic, "/arduino_pin/+");
    subscribeTopic(this->ERaTopic, "/virtual_pin/+");
    subscribeTopic(this->ERaTopic, "/pin/down");
    subscribeTopic(this->ERaTopic, "/down");

#if defined(ERA_ASK_CONFIG_WHEN_RESTART)
    if (!this->publishLWT(true)) {
        return false;
    }
#else
    if (!this->publishLWT(this->getAskConfig())) {
        return false;
    }
#endif

    this->_connected = true;
    ERaOnConnected();
    return true;
}

template <class MQTT>
inline
void ERaMqttLinux<MQTT>::disconnect() {
    this->mqtt.disconnect();
}

template <class MQTT>
inline
bool ERaMqttLinux<MQTT>::connected() {
    return this->mqtt.connected();
}

template <class MQTT>
inline
bool ERaMqttLinux<MQTT>::run() {
    if (!this->mqtt.loop()) {
        ERaOnDisconnected();
        return this->connect();
    }
    return true;
}

template <class MQTT>
inline
bool ERaMqttLinux<MQTT>::subscribeTopic(const char* baseTopic, const char* topic,
                                        QoST qos) {
    bool status {false};
    char topicName[MAX_TOPIC_LENGTH] {0};
	FormatString(topicName, baseTopic);
	FormatString(topicName, topic);

    ERaGuardLock(this->mutex);
    if (this->connected()) {
        status = this->mqtt.subscribe(topicName, qos);
        ERR_MQTT_SUB_LOG(status, this->mqtt.lastError())
    }
    ERaGuardUnlock(this->mutex);

    return status;
}

template <class MQTT>
inline
bool ERaMqttLinux<MQTT>::publishData(const char* topic, const char* payload,
                                    bool retained, QoST qos) {
    if (!this->_connected) {
        return false;
    }

    bool status {false};

    ERaGuardLock(this->mutex);
    if (this->connected()) {
        status = this->mqtt.publish(topic, payload, retained, qos);
        ERA_MQTT_PUB_LOG(status, this->mqtt.lastError())
    }
    ERaGuardUnlock(this->mutex);

    return status;
}

template <class MQTT>
inline
bool ERaMqttLinux<MQTT>::syncConfig() {
    return this->publishLWT(true);
}

template <class MQTT>
inline
bool ERaMqttLinux<MQTT>::publishLWT(bool sync) {
    bool status {false};
    char wifiInfo[50] {0};
    char payload[100] {0};
    char* topic = this->willTopic;

    if (this->getSSID() != nullptr) {
        FormatString(wifiInfo, WIFI_INFO, this->getSSID());
    }

    FormatString(payload, ONLINE_MESSAGE, wifiInfo, sync ? ASK_CONFIG_INFO : "");

    ERaGuardLock(this->mutex);
    if (this->connected()) {
        this->ping = ERaMillis();
        status = this->mqtt.publish(topic, payload, LWT_RETAINED, LWT_QOS);
        this->ping = ERaMillis() - this->ping;
        ERA_MQTT_PUB_LOG(status, this->mqtt.lastError())
    }
    ERaGuardUnlock(this->mutex);

    return status;
}

#endif /* INC_ERA_MQTT_LINUX_HPP_ */

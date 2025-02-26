#ifndef INC_ERA_SERIAL_BLE_TRANSP_HPP_
#define INC_ERA_SERIAL_BLE_TRANSP_HPP_

#include <BLE/ERaBLEConfig.hpp>
#include <BLE/ERaParse.hpp>
#include <ERa/ERaOTP.hpp>
#include <ERa/ERaTransp.hpp>
#include <ERa/ERaProtocol.hpp>
#include <Utility/ERacJSON.hpp>
#include <Utility/ERaQueue.hpp>
#include <Utility/ERaUtility.hpp>

class ERaBLETransp
    : public ERaTransp
{
	const char* TAG = "BLETransp";

public:
    ERaBLETransp(Stream& _stream)
        : stream(_stream)
        , transpProp(ERaBluetooth::instance())
        , timeout(1000L)
        , _connected(false)
        , initialized(false)
        , topic(NULL)
        , callback(NULL)
#if defined(ERA_RTOS)
        , _bleTask(NULL)
#endif
    {
        ERa.setERaTransp(*this);
    }
    ~ERaBLETransp()
    {}

    void setTranspProperty(void* args) {
        if (args == NULL) {
            return;
        }
        char* ptr = (char*)args;
        this->transpProp->getInstance();
        this->transpProp->parseConfig(ptr);
    }

    void begin(void* args = NULL) override {
        if (args == NULL) {
            return;
        }
        this->setTranspProperty(args);
        if (!strlen(this->transpProp->address)) {
            return;
        }

        this->onConfig();

        if (this->initialized) {
            return;
        }
#if defined(ERA_RTOS) && !defined(ERA_NO_RTOS)
        #if !defined(ERA_MCU_CORE)
            #define ERA_MCU_CORE    0
        #endif
        this->_bleTask = ERaOs::osThreadNew(this->bleTask, "bleTask", 1024 * 5, this,
                                            configMAX_PRIORITIES - 10, ERA_MCU_CORE);
#endif
        this->_connected = true;
        this->initialized = true;
    }

    void run() override {
        if (!this->connected()) {
            return;
        }

#if !defined(ERA_RTOS) || defined(ERA_NO_RTOS)
        this->progressData();
#endif
    }

    void setTopic(const char* _topic) override {
        this->topic = _topic;
    }

    void onMessage(MessageCallback_t cb) override {
        this->callback = cb;
    }

    int connect(IPAddress ip, uint16_t port) override {
        ERA_FORCE_UNUSED(ip);
        ERA_FORCE_UNUSED(port);
        this->_connected = true;
        return true;
    }

    int connect(const char* host, uint16_t port) override {
        ERA_FORCE_UNUSED(host);
        ERA_FORCE_UNUSED(port);
        this->_connected = true;
        return true;
    }

    void disconnect() {
        return this->stop();
    }

    size_t write(uint8_t value) override {
        return this->stream.write(value);
    }

    size_t write(const uint8_t* buf, size_t size) override {
        return this->stream.write(buf, size);
    }

    size_t write(const char* buf) {
        return this->write((const uint8_t*)buf, strlen(buf));
    }

    int available() override {
        return this->stream.available();
    }

    int read() override {
        return this->stream.read();
    }

    int read(uint8_t* buf, size_t size) override {
        uint8_t* begin = buf;
        uint8_t* end = buf + size;
        MillisTime_t startMillis = ERaMillis();
        while ((begin < end) && (ERaMillis() - startMillis < this->timeout)) {
            int c = this->stream.read();
            if (c < 0) {
                continue;
            }
            *begin++ = (uint8_t)c;
        }
        return begin - buf;
    }

    int timedRead() {
        uint8_t c {0};
        this->read(&c, 1);
        return (int)c;
    }

    bool readBytesUntil(uint8_t* buf, size_t size, char terminator) {
        if (buf == nullptr) {
            return false;
        }
        if (!size) {
            return false;
        }
        size_t index {0};
        bool found {false};
        while (index < size) {
            int c = this->timedRead();
            if (c < 0) {
                break;
            }
            if (c == terminator) {
                found = true;
                break;
            }
            *buf++ = (uint8_t)c;
            index++;
        }
        return found;
    }

    int peek() override {
        return this->stream.peek();
    }

    void flush() override {
        this->stream.flush();
    }

    void stop() override {
        this->_connected = false;
    }

    uint8_t connected() override {
        return this->_connected;
    }

    operator bool() override {
        return this->_connected;
    }

protected:
#if defined(ERA_RTOS)
    static void bleTask(void* args) {
        if (args == NULL) {
            ERaOs::osThreadDelete(NULL);
        }
        ERaBLETransp* ble = (ERaBLETransp*)args;
        for (;;) {
            ERA_BLE_YIELD();
            ble->progressData();
        }
        ERaOs::osThreadDelete(NULL);
    }
#endif

    void progressData() {
        if (!this->connected()) {
            return;
        }

        size_t len = this->available();
        if (!len) {
            return;
        }

        size_t size {0};
        uint8_t locBuf[256] {0};
        uint8_t* buf = locBuf;

        do {
            size += len;
            if (size >= sizeof(locBuf)) {
                if (buf == locBuf) {
                    buf = (uint8_t*)ERA_MALLOC(size + 1);
                    memcpy(buf, locBuf, size);
                }
                else {
                    uint8_t* copy = (uint8_t*)ERA_REALLOC(buf, size + 1);
                    if (copy == nullptr) {
                        free(buf);
                        buf = nullptr;
                        return;
                    }
                    buf = copy;
                }
                if (buf == nullptr) {
                    return;
                }
                buf[size] = 0;
            }
            this->read(buf + size - len, len);
            MillisTime_t startMillis = ERaMillis();
            do {
                ERaDelay(1);
                len = this->available();
            } while(!len && (ERaMillis() - startMillis < ERA_BLE_YIELD_MS));
        } while (len);

        this->progress((char*)buf);

        if (buf != locBuf) {
            free(buf);
        }
        buf = nullptr;
    }

    static ERaBLETransp instance;

private:
    void onConfig() {
        cJSON* root = cJSON_CreateObject();
        if (root == nullptr) {
            return;
        }

        char secretKey[17] {0};
        memcpy(secretKey, this->transpProp->secretKey, sizeof(this->transpProp->secretKey));

        cJSON_AddStringToObject(root, "type", "config");
        cJSON_AddStringToObject(root, "user_name", this->transpProp->address);
        cJSON_AddStringToObject(root, "password", this->transpProp->password);
        cJSON_AddStringToObject(root, "secret_key", secretKey);

        char* config = cJSON_PrintUnformatted(root);

        if (config != nullptr) {
            this->write(config);
        }

        cJSON_Delete(root);
        free(config);
        root = nullptr;
        config = nullptr;
    }

    void progress(const char* message) {
        cJSON* root = cJSON_Parse(message);
        if (!cJSON_IsObject(root)) {
            cJSON_Delete(root);
            root = nullptr;
            this->response("failed");
            return;
        }

        if (this->onCallback(root)) {
            this->response("ok");
        }
        else {
            this->response("failed");
        }

        cJSON_Delete(root);
        root = nullptr;
    }

    void response(const char* status) {
        cJSON* root = cJSON_CreateObject();
        if (root == nullptr) {
            return;
        }

        cJSON_AddStringToObject(root, "type", "command");
        cJSON_AddStringToObject(root, "status", status);

        char* rsp = cJSON_PrintUnformatted(root);

        if (rsp != nullptr) {
            this->write(rsp);
        }

        cJSON_Delete(root);
        free(rsp);
        root = nullptr;
        rsp = nullptr;
    }

    bool onCallback(cJSON* const root) {
        if ((this->topic == nullptr) ||
            (this->callback == nullptr)) {
            return false;
        }

        cJSON* item = cJSON_GetObjectItem(root, "type");
        if (!cJSON_IsString(item)) {
            return false;
        }
        if (!ERaStrCmp(item->valuestring, "command")) {
            return false;
        }
        item = cJSON_GetObjectItem(root, "alias");
        if (!cJSON_IsString(item)) {
            return false;
        }
        if (strlen(item->valuestring) != 36) {
            return false;
        }

        bool status {false};
        char* payload = nullptr;
        char _topic[MAX_TOPIC_LENGTH] {0};
        FormatString(_topic, this->topic);
        // Now support only Modbus
        FormatString(_topic, "/down");

        cJSON* object = cJSON_CreateObject();
        if (object == nullptr) {
            return false;
        }
        cJSON* subObject = cJSON_CreateObject();
        if (subObject == nullptr) {
            cJSON_Delete(object);
            object = nullptr;
            return false;
        }
        cJSON_AddStringToObject(object, "action", "send_command");
        cJSON_AddItemToObject(object, "data", subObject);
        cJSON* array = cJSON_CreateArray();
        if (array == nullptr) {
            cJSON_Delete(object);
            object = nullptr;
            return false;
        }

        cJSON_AddItemToArray(array, cJSON_CreateString(item->valuestring));
        cJSON_AddItemToObject(subObject, "commands", array);
        payload = cJSON_PrintUnformatted(object);

        if (payload != nullptr) {
            status = true;
            this->callback(_topic, payload);
            free(payload);
        }

        cJSON_Delete(object);
        payload = nullptr;
        object = nullptr;
        return status;
    }

    Stream& stream;
    ERaBluetooth*& transpProp;
    unsigned long timeout;
    bool _connected;
    bool initialized;
    const char* topic;
    MessageCallback_t callback;
#if defined(ERA_RTOS)
    TaskHandle_t _bleTask;
#endif
};

#endif /* INC_ERA_SERIAL_BLE_TRANSP_HPP_ */

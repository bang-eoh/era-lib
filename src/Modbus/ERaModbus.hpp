#ifndef INC_ERA_MODBUS_HPP_
#define INC_ERA_MODBUS_HPP_

#include <math.h>
#include <ERa/ERaParam.hpp>
#include <Utility/ERaUtility.hpp>
#include <Modbus/ERaModbusState.hpp>
#include <Modbus/ERaParse.hpp>
#include <Modbus/ERaModbusConfig.hpp>
#include <Modbus/ERaDefineModbus.hpp>
#include <Modbus/ERaModbusTransp.hpp>

using namespace std;

template <class Api>
class ERaModbus
    : public ERaModbusTransp < ERaModbus<Api> >
{
    typedef struct __ModbusAction_t {
        char* key;
    } ModbusAction_t;
    typedef struct __TCPIp_t {
        IPAddress ip;
        uint16_t port;
    } TCPIp_t;
    typedef void* TaskHandle_t;
    typedef void* QueueMessage_t;
    const char* TAG = "Modbus";
    const char* FILENAME_CONFIG = FILENAME_MODBUS_CONFIG;
    const char* FILENAME_CONTROL = FILENAME_MODBUS_CONTROL;
    const IPAddress ipNone{0, 0, 0, 0};

    friend class ERaModbusTransp < ERaModbus<Api> >;
	typedef ERaModbusTransp < ERaModbus<Api> > ModbusTransp;

public:
    ERaModbus()
        : modbusConfig(ERaApplication::config())
        , modbusControl(ERaApplication::control())
        , timeout(DEFAULT_TIMEOUT_MODBUS)
        , prevMillis(0)
        , total(0)
        , failRead(0)
        , failWrite(0)
        , dePin(-1)
        , transp(0)
        , streamRTU(NULL)
        , clientTCP(NULL)
        , ip {
            .ip = {},
            .port = 502
        }
        , stream(NULL)
        , _streamDefault(false)
        , _modbusTask(NULL)
        , _writeModbusTask(NULL)
        , messageHandle(NULL)
        , mutex(NULL)
    {}
    ~ERaModbus()
    {
        this->switchToModbusRTU();
    }

    void setModbusClient(Client& _client, IPAddress _ip = IPAddress(0, 0, 0, 0), uint16_t _port = 502) {
        this->clientTCP = &_client;
        this->ip.ip = _ip;
        this->ip.port = _port;
    }

    void setModbusStream(Stream& _stream) {
        this->streamRTU = &_stream;
    }

    void setModbusTimeout(uint32_t _timeout) {
        this->timeout = _timeout;
    }

    void setPubModbusInterval(uint32_t _interval) {
        if (!_interval) {
            return;
        }
        ERaApplication::getConfig()->pubInterval.delay = _interval;
    }

    void setModbusDEPin(int _pin);

protected:
    void begin() {
        ERaApplication::getConfig();
        ERaApplication::getControl();
        this->dataBuff.allocate(MODBUS_DATA_BUFFER_SIZE);
        this->configModbus();
        this->initModbusConfig();
        this->initModbusTask();
    }

    void run(const bool isRead) {
        this->waitTCPIpReady();
        for (;;) {
            if (isRead) {
                this->runRead();
            } else {
                this->runWrite();
            }
            ERA_MODBUS_YIELD();
        }
    }

    void runRead() {
        ModbusState::set(ModbusStateT::STATE_MB_RUNNING);
        if (!ERaRemainingTime(this->modbusConfig->modbusInterval.prevMillis, this->modbusConfig->modbusInterval.delay)) {
            this->modbusConfig->modbusInterval.prevMillis = ERaMillis();
            this->readModbusConfig();
        }
    }

    void runWrite() {
        if (!this->isRequest()) {
            return;
        }
        ModbusAction_t& req = this->getRequest();
        if (req.key == nullptr) {
            return;
        }
        this->actionModbus(req.key);
        if (this->isEmptyRequest()) {
            this->executeNow();
            ModbusState::set(ModbusStateT::STATE_MB_CONTROLLED);
        }
        free(req.key);
        req.key = nullptr;
    }

    void parseModbusConfig(char* config, const char* hash, const char* buf, bool isControl = false) {
        if (isControl) {
            ERaGuardLock(this->mutex);
            this->modbusControl->parseConfig(config);
            if (this->modbusControl->updateHashID(hash)) {
                this->thisApi().writeToFlash(FILENAME_CONTROL, buf);
            }
            ERaGuardUnlock(this->mutex);
        } else {
            ERaGuardLock(this->mutex);
            this->modbusConfig->parseConfig(config);
            if (this->modbusConfig->updateHashID(hash)) {
                this->clearDataBuff();
                this->thisApi().writeToFlash(FILENAME_CONFIG, buf);
            }
            ERaGuardUnlock(this->mutex);
            this->setBaudRate(this->modbusConfig->baudSpeed);
            this->executeNow();
        }
        ModbusState::set(ModbusStateT::STATE_MB_PARSE);
    }

    void removeConfigFromFlash() {
        if (this->modbusConfig != nullptr) {
            this->modbusConfig->deleteAll();
        }
        if (this->modbusControl != nullptr) {
            this->modbusControl->deleteAll();
        }
        this->thisApi().removeFromFlash(FILENAME_CONFIG);
        this->thisApi().removeFromFlash(FILENAME_CONTROL);
    }

    void clearDataBuff() {
        this->dataBuff.clearBuffer();
    }

    bool addModbusAction(const char* ptr) {
        if (strlen(ptr) != 36) {
            return false;
        }
	    if (!this->queue.writeable()) {
            return false;
        }
        char* buf = (char*)ERA_MALLOC(37 * sizeof(char));
        if (buf == nullptr) {
            return false;
        }
        ModbusAction_t req;
        req.key = buf;
        memset(req.key, 0, 37 * sizeof(char));
        strcpy(req.key, ptr);
        this->queue += req;
        return true;
    }

    void initModbusTask();

#if defined(LINUX)
    static void* modbusTask(void* args);
    static void* writeModbusTask(void* args);
#else
    static void modbusTask(void* args);
    static void writeModbusTask(void* args);
#endif

private:
    void initModbusConfig() {
        char* ptr = nullptr;
        ptr = this->thisApi().readFromFlash(FILENAME_CONFIG);
        this->updateConfig(this->modbusConfig, ptr, "configuration");
        this->setBaudRate(this->modbusConfig->baudSpeed);
        free(ptr);
        ptr = this->thisApi().readFromFlash(FILENAME_CONTROL);
        this->updateConfig(this->modbusControl, ptr, "control");
        free(ptr);
        ptr = nullptr;
    }

    void updateConfig(ERaApplication* config, const char* buf, const char* name) {
        cJSON* root = cJSON_Parse(buf);
        if (!cJSON_IsObject(root)) {
            cJSON_Delete(root);
            root = nullptr;
            return;
        }

        cJSON* data = cJSON_GetObjectItem(root, "data");
        cJSON* item = cJSON_GetObjectItem(data, "hash_id");
        if (cJSON_IsString(item)) {
            config->updateHashID(item->valuestring);
        }
        item = cJSON_GetObjectItem(data, name);
        if (cJSON_IsString(item)) {
            config->parseConfig(item->valuestring);
        }

        cJSON_Delete(root);
        root = nullptr;
    }

    void nextTransport(const ModbusConfig_t& param) {
        this->switchToModbusRTU();
        if (this->clientTCP == nullptr) {
            return;
        }
        if (param.ipSlave.ip.dword) {
            this->switchToModbusTCP();
        }
        if ((ip.ip == IPAddress(param.ipSlave.ip.dword)) ||
            (param.ipSlave.ip.dword == 0)) {
            return;
        }
        if (param.ipSlave.port == 0) {
            return;
        }
        ip.ip = param.ipSlave.ip.dword;
        ip.port = param.ipSlave.port;
        // TODO
        if (this->clientTCP->connected()) {
            this->clientTCP->stop();
        }
        this->clientTCP->connect(ip.ip, ip.port);
    }

    void switchToModbusRTU() {
        this->stream = this->streamRTU;
        this->transp = ModbusTransportT::MODBUS_TRANSPORT_RTU;
    }

    void switchToModbusTCP() {
        this->stream = this->clientTCP;
        this->transp = ModbusTransportT::MODBUS_TRANSPORT_TCP;
    }

    void configModbus();
    void setBaudRate(uint32_t baudrate);
    void readModbusConfig();
    bool checkPubDataInterval();
    void delayModbus(const int address);
    ModbusConfigAlias_t* getModbusAlias(const char* key);
    ModbusConfig_t* getModbusConfig(int id);
    bool actionModbus(const char* key);
    bool eachActionModbus(Action_t& action, ModbusConfig_t*& config);
    void sendModbusRead(ModbusConfig_t& param);
    bool sendModbusWrite(ModbusConfig_t& param);
    void onData(ERaModbusRequest* request, ERaModbusResponse* response);
    void onError(ERaModbusRequest* request);
    bool waitResponse(ERaModbusResponse* response);
    void sendCommand(uint8_t* data, size_t size);
    void switchToTransmit();
    void switchToReceive();

    void waitTCPIpReady() {
        if (this->clientTCP == nullptr) {
            return;
        }
        do {
            ERaDelay(1000);
        } while (!this->thisApi().connected());
    }

    bool getBit(uint8_t byte, uint8_t pos) {
        return ((byte >> pos) & 0x01);
    }

    bool streamDefault() {
        return this->_streamDefault;
    }

    void executeNow() {
        this->modbusConfig->modbusInterval.prevMillis = ERaMillis() - this->modbusConfig->modbusInterval.delay + ERA_MODBUS_EXECUTE_MS;
    }

	bool isRequest() {
		return this->queue.readable();
	}

	ModbusAction_t& getRequest() {
		return this->queue;
	}

    bool isEmptyRequest() {
        return this->queue.isEmpty();
    }

	inline
	const Api& thisApi() const {
		return static_cast<const Api&>(*this);
	}

	inline
	Api& thisApi() {
		return static_cast<Api&>(*this);
	}

	ERaQueue<ModbusAction_t, 10> queue;
    ERaDataBuffDynamic dataBuff;
    ERaApplication*& modbusConfig;
    ERaApplication*& modbusControl;
    uint32_t timeout;
    unsigned long prevMillis;
    int total;
    int failRead;
    int failWrite;
    int dePin;
    uint8_t transp;
    Stream* streamRTU;
    Client* clientTCP;
    TCPIp_t ip;
    Stream* stream;
    bool _streamDefault;
    TaskHandle_t _modbusTask;
    TaskHandle_t _writeModbusTask;
    QueueMessage_t messageHandle;
    ERaMutex_t mutex;
};

template <class Api>
void ERaModbus<Api>::readModbusConfig() {
    if (this->modbusConfig->modbusConfigParam.isEmpty()) {
        return;
    }
    this->dataBuff.clear();
    const ERaList<ModbusConfig_t*>::iterator* e = this->modbusConfig->modbusConfigParam.end();
    for (ERaList<ModbusConfig_t*>::iterator* it = this->modbusConfig->modbusConfigParam.begin(); it != e; it = it->getNext()) {
        ModbusConfig_t* param = it->get();
        if (param == nullptr) {
            continue;
        }
        ERaGuardLock(this->mutex);
        this->sendModbusRead(*param);
        ERaGuardUnlock(this->mutex);
        this->delayModbus(param->addr);
        if (ModbusState::is(ModbusStateT::STATE_MB_PARSE) ||
            ModbusState::is(ModbusStateT::STATE_MB_CONTROLLED)) {
            this->executeNow();
            return;
        }
    }
    if (this->dataBuff.isEmpty()) {
        return;
    }
    if (this->dataBuff.isChange()) {
        this->prevMillis = ERaMillis();
    }
    else if (!this->checkPubDataInterval()) {
        return;
    }
    this->dataBuff.add_multi(ERA_F("fail_read"), this->failRead, ERA_F("fail_write"), this->failWrite, ERA_F("total"), this->total);
    this->thisApi().modbusDataWrite(&this->dataBuff);
}

template <class Api>
bool ERaModbus<Api>::checkPubDataInterval() {
    unsigned long currentMillis = ERaMillis();
    if (currentMillis - this->prevMillis < this->modbusConfig->pubInterval.delay) {
        return false;
    }
    unsigned long skipTimes = (currentMillis - this->prevMillis) / this->modbusConfig->pubInterval.delay;
    this->prevMillis += this->modbusConfig->pubInterval.delay * skipTimes;
    return true;
}

template <class Api>
void ERaModbus<Api>::delayModbus(const int address) {
    MillisTime_t delayMs {10L};
    MillisTime_t startMillis = ERaMillis();
    const ERaList<SensorDelay_t*>::iterator* e = this->modbusConfig->sensorDelay.end();
    for (ERaList<SensorDelay_t*>::iterator* it = this->modbusConfig->sensorDelay.begin(); it != e; it = it->getNext()) {
        if (it->get() == nullptr) {
            continue;
        }
        if (it->get()->address == address) {
            delayMs = it->get()->delay;
            break;
        }
    }
    do {
#if defined(ERA_NO_RTOS)
        ERaOnWaiting();
        this->thisApi().run();
#endif
        if (ModbusState::is(ModbusStateT::STATE_MB_PARSE) ||
            ModbusState::is(ModbusStateT::STATE_MB_CONTROLLED)) {
            break;
        }
        ERA_MODBUS_YIELD();
    } while (ERaRemainingTime(startMillis, delayMs));
}

template <class Api>
ModbusConfigAlias_t* ERaModbus<Api>::getModbusAlias(const char* key) {
    const ERaList<ModbusConfigAlias_t*>::iterator* e = this->modbusConfig->modbusConfigAliasParam.end();
    for (ERaList<ModbusConfigAlias_t*>::iterator* it = this->modbusConfig->modbusConfigAliasParam.begin(); it != e; it = it->getNext()) {
        if (it->get() == nullptr) {
            continue;
        }
        if (!strcmp(it->get()->key, key)) {
            return it->get();
        }
    }
    return nullptr;
}

template <class Api>
bool ERaModbus<Api>::actionModbus(const char* key) {
    if (key == nullptr) {
        return false;
    }
    if (strlen(key) != 36) {
        return false;
    }

    ModbusConfigAlias_t* alias = this->getModbusAlias(key);
    if (alias == nullptr) {
        return false;
    }

    ModbusConfig_t* config = nullptr;
    for (int i = 0; i < alias->readActionCount; ++i) {
        ERaGuardLock(this->mutex);
        eachActionModbus(alias->action[i], config);
        ERaGuardUnlock(this->mutex);
        if ((config != nullptr) &&
            (i != (alias->readActionCount - 1))) {
            this->delayModbus(config->addr);
        }
    }
    
    return true;
}

template <class Api>
ModbusConfig_t* ERaModbus<Api>::getModbusConfig(int id) {
    const ERaList<ModbusConfig_t*>::iterator* e = this->modbusControl->modbusConfigParam.end();
    for (ERaList<ModbusConfig_t*>::iterator* it = this->modbusControl->modbusConfigParam.begin(); it != e; it = it->getNext()) {
        if (it->get() == nullptr) {
            continue;
        }
        if (it->get()->id == id) {
            return it->get();
        }
    }
    return nullptr;
}

template <class Api>
bool ERaModbus<Api>::eachActionModbus(Action_t& action, ModbusConfig_t*& config) {
    config = this->getModbusConfig(action.id);
    if (config == nullptr) {
        return false;
    }

    config->len1 = action.len1;
    config->len2 = action.len2;

    memcpy(config->extra, action.extra, sizeof(config->extra));

    return this->sendModbusWrite(*config);
}

template <class Api>
void ERaModbus<Api>::sendModbusRead(ModbusConfig_t& param) {
    bool status {false};
    this->nextTransport(param);
    switch (param.func) {
        case ModbusFunctionT::READ_COIL_STATUS:
            status = ModbusTransp::readCoilStatus(this->transp, param);
            break;
        case ModbusFunctionT::READ_INPUT_STATUS:
            status = ModbusTransp::readInputStatus(this->transp, param);
            break;
        case ModbusFunctionT::READ_HOLDING_REGISTERS:
            status = ModbusTransp::readHoldingRegisters(this->transp, param);
            break;
        case ModbusFunctionT::READ_INPUT_REGISTERS:
            status = ModbusTransp::readInputRegisters(this->transp, param);
            break;
        default:
            return;
    }

    if (status) {
        param.totalFail = 0;
    }
    else {
        this->failRead++;
        param.totalFail++;
    }
}

template <class Api>
bool ERaModbus<Api>::sendModbusWrite(ModbusConfig_t& param) {
    bool status {false};
    this->nextTransport(param);
    switch (param.func) {
        case ModbusFunctionT::FORCE_SINGLE_COIL:
            status = ModbusTransp::forceSingleCoil(this->transp, param);
            break;
        case ModbusFunctionT::PRESET_SINGLE_REGISTER:
            status = ModbusTransp::presetSingleRegister(this->transp, param);
            break;
        case ModbusFunctionT::FORCE_MULTIPLE_COILS:
            status = ModbusTransp::forceMultipleCoils(this->transp, param);
            break;
        case ModbusFunctionT::PRESET_MULTIPLE_REGISTERS:
            status = ModbusTransp::presetMultipleRegisters(this->transp, param);
            break;
        default:
            return false;
    }

    if (!status) {
        this->failWrite++;
    }
    return status;
}

template <class Api>
void ERaModbus<Api>::onData(ERaModbusRequest* request, ERaModbusResponse* response) {
    if ((request == nullptr) ||
        (response == nullptr)) {
        return;
    }

    switch (request->getFunction()) {
        case ModbusFunctionT::READ_COIL_STATUS:
        case ModbusFunctionT::READ_INPUT_STATUS: {
            LOC_BUFFER_MODBUS(1)
            uint16_t bits = request->getLength();
            for (int i = 0; (i < response->getBytes()) && (bits > 0); ++i) {
                for (int j = 0; j < ((bits > 8) ? 8 : bits); ++j) {
                    pData[i * 8 + j] = this->getBit(response->getData()[i], j);
                }
                bits -= ((bits > 8) ? 8 : bits);
            }
            this->dataBuff.add_hex_array(pData, pDataLen);
            FREE_BUFFER_MODBUS
        }
            break;
        default:
            this->dataBuff.add_hex_array(response->getData(), response->getBytes());
            break;
    }
    this->dataBuff.add("1");
}

template <class Api>
void ERaModbus<Api>::onError(ERaModbusRequest* request) {
    if (request == nullptr) {
        return;
    }
    if (ModbusState::is(ModbusStateT::STATE_MB_PARSE)) {
        return;
    }

    switch (request->getFunction()) {
        case ModbusFunctionT::READ_COIL_STATUS:
        case ModbusFunctionT::READ_INPUT_STATUS:
            if (!this->dataBuff.next()) {
                size_t pDataLen = request->getLength();
                this->dataBuff.add_zero_array(pDataLen);
            }
            break;
        default:
            if (!this->dataBuff.next()) {
                size_t pDataLen = request->getLength() * 2;
                this->dataBuff.add_zero_array(pDataLen);
            }
            break;
    }
    this->dataBuff.add("0");
}

template <class Api>
void ERaModbus<Api>::setModbusDEPin(int _pin) {
    if (_pin < 0) {
        return;
    }

    this->dePin = _pin;
#if defined(ARDUINO) || \
    (defined(LINUX) &&  \
    defined(RASPBERRY))
    pinMode(this->dePin, OUTPUT);
#endif
}

template <class Api>
void ERaModbus<Api>::switchToTransmit() {
    if (this->dePin < 0) {
        return;
    }

#if defined(ARDUINO) || \
    (defined(LINUX) &&  \
    defined(RASPBERRY))
    ::digitalWrite(this->dePin, HIGH);
#endif
}

template <class Api>
void ERaModbus<Api>::switchToReceive() {
    if (this->dePin < 0) {
        return;
    }

#if defined(ARDUINO) || \
    (defined(LINUX) &&  \
    defined(RASPBERRY))
    ::digitalWrite(this->dePin, LOW);
#endif
}

template <class S, typename... Args>
class SerialModBus {
public:
    static S& serial(Args... tail) {
        static S _serial(tail...);
        return _serial;
    }
};

#endif /* INC_ERA_MODBUS_HPP_ */

#ifndef INC_ERA_PARSE_HPP_
#define INC_ERA_PARSE_HPP_

#include <string.h>
#include <ERa/ERaDebug.hpp>
#include <ERa/ERaHelperDef.hpp>
#include <Utility/ERaQueue.hpp>

#ifndef MAX_DEVICE_MODBUS
    #define MAX_DEVICE_MODBUS           20
#endif

#ifndef DEFAULT_MODBUS_BAUD_SPEED
    #define DEFAULT_MODBUS_BAUD_SPEED   9600
#endif

#ifndef DEFAULT_MODBUS_INTERVAL
    #define DEFAULT_MODBUS_INTERVAL 	1000
#endif

#ifndef DEFAULT_PUB_MODBUS_INTERVAL
    #define DEFAULT_PUB_MODBUS_INTERVAL 60000
#endif

#ifndef DEFAULT_MIN_MODBUS_INTERVAL
    #define DEFAULT_MIN_MODBUS_INTERVAL 1000
#endif

typedef struct __SensorDelay_t {
	int delay;
	int address;
} SensorDelay_t;

typedef struct __IPSlave_t {
    union {
        uint8_t bytes[4];
        uint32_t dword;
    } ip;
    uint16_t port;
} IPSlave_t;

typedef struct __ModbusConfig_t {
	int id;
	uint8_t addr;
	uint8_t func;
	uint8_t sa1;
	uint8_t sa2;
	uint8_t len1;
	uint8_t len2;
	uint8_t extra[10];
	uint8_t type;
	uint8_t button;
	uint16_t delay;
	IPSlave_t ipSlave;
	uint8_t totalFail;
	uint8_t sizeData;
	uint32_t value;
	uint8_t ack;
} ModbusConfig_t;

typedef struct __Action_t {
	int id;
	uint8_t len1;
	uint8_t len2;
	uint8_t extra[10];
} Action_t;

typedef struct __ModbusConfigAlias_t {
	int readActionCount;
	int id;
	char key[37];
	Action_t action[10];
	uint16_t timer;
} ModbusConfigAlias_t;

typedef struct __IntervalDelay_t {
	uint32_t delay;
	unsigned long prevMillis;
} IntervalDelay_t;

class ERaApplication {
    enum ParseConfigT {
        PARSE_CONFIG_ID = 1,
        PARSE_BAUD_SPEED = 2,
        PARSE_TOTAL_ROW = 3,
        PARSE_SENSOR_DISPLAY = 4,
        PARSE_SENSOR_READ_WRITE = 5,
        PARSE_LOCAL_ALARM = 6,
        PARSE_CONTACT_PHONE = 7,
        PARSE_IS_WIFI = 8,
        PARSE_CONFIG_WIFI = 9,
        PARSE_CONFIG_ALIAS_TOTAL_ROW = 10,
        PARSE_CONFIG_ALIAS_DATA = 11,
        PARSE_IS_BLUETOOTH = 12
    };

    enum ParseIntervalT {
        PARSE_MODBUS_BAUDRATE = 0,
        PARSE_MODBUS_INTERVAL = 1,
        PARSE_PUB_MODBUS_INTERVAL = 2
    };

public:
    ERaApplication()
        : baudSpeed(DEFAULT_MODBUS_BAUD_SPEED)
        , modbusInterval {
            .delay = DEFAULT_MODBUS_INTERVAL,
            .prevMillis = 0
        }
        , pubInterval {
            .delay = DEFAULT_PUB_MODBUS_INTERVAL,
            .prevMillis = 0
        }
    {
        memset(this->hashID, 0, sizeof(this->hashID));
    }
    ~ERaApplication()
    {}

    void parseConfig(char* ptr);
    bool updateHashID(const char* hash);
    void deleteAll();

    unsigned int id;
    unsigned int baudSpeed;
    bool isWifi;
    char ssid[64];
    char password[64];
    char hashID[37];
    bool isBluetooth;

    size_t sensorCount;
    ERaList<SensorDelay_t*> sensorDelay;
    size_t readConfigCount;
    ERaList<ModbusConfig_t*> modbusConfigParam;
    size_t readConfigAliasCount;
    ERaList<ModbusConfigAlias_t*> modbusConfigAliasParam;

    IntervalDelay_t modbusInterval;
    IntervalDelay_t pubInterval;

    static ERaApplication*& config() {
        static ERaApplication* _config = nullptr;
        return _config;
    }

    static ERaApplication*& control() {
        static ERaApplication* _control = nullptr;
        return _control;
    }

    static ERaApplication* getConfig() {
        if (ERaApplication::config() == nullptr) {
            ERaApplication::config() = new ERaApplication();
        }
        return ERaApplication::config();
    }

    static ERaApplication* getControl() {
        if (ERaApplication::control() == nullptr) {
            ERaApplication::control() = new ERaApplication();
        }
        return ERaApplication::control();
    }

private:
    void processParseConfig(int part, char* ptr, size_t len);
    void processParseConfigId(char* ptr, size_t len);
    void processParseConfigBaudSpeed(char* ptr, size_t len);
    void processParseConfigTotalRow(char* ptr, size_t len);
    void processParseConfigSensorDelay(char* ptr, size_t len);
    void processParseConfigSensorReadWrite(char* ptr, size_t len);
    void parseOneConfigSensorReadWrite(char* ptr, size_t len, ModbusConfig_t& config);
    void actOneConfigSensorReadWrite(int* ptr, size_t len, ModbusConfig_t& config);
    void processParseConfigLocalAlarm(char* ptr, size_t len);
    void parseOneConfigLocalAlarm(char* ptr, size_t len);
    void processParseStationContactPhone(char* ptr, size_t len);
    void parseOneConfigPhoneNumber(char* ptr, size_t len);
    void processParseIsWifi(char* ptr, size_t len);
    void processParseConfigWifi(char* ptr, size_t len);
    void processParseConfigAliasTotalRow(char* ptr, size_t len);
    void processParseConfigAliasData(char* ptr, size_t len);
    void processOneConfigAlias(char* ptr, size_t len, ModbusConfigAlias_t& config);
    void parseOneConfigAlias(char* ptr, size_t len, ModbusConfigAlias_t& config);
    void actOneConfigAlias(int* ptr, size_t len, ModbusConfigAlias_t& config, const char* key);
    void processParseAction(char* ptr, size_t len, ModbusConfigAlias_t& config);
    void parseOneAction(char* ptr, size_t len, ModbusConfigAlias_t& config);
    void actOneAction(int* ptr, size_t len, ModbusConfigAlias_t& config);
    void processParseIsEnableBluetooth(char* ptr, size_t len);

    template <typename T>
    T* create() {
        T* ptr = (T*)ERA_MALLOC(sizeof(T));
        memset(ptr, 0, sizeof(T));
        return ptr;
    }

    template <typename T>
    void clear(ERaList<T*>& value) {
        const typename ERaList<T*>::iterator* e = value.end();
        for (typename ERaList<T*>::iterator* it = value.begin(); it != e; it = it->getNext()) {
            if (it->get() == nullptr) {
                continue;
            }
            free(it->get());
            it->get() = nullptr;
        }
        value.clear();
    }

};

inline
void ERaApplication::parseConfig(char* ptr) {
    if (ptr == nullptr) {
        return;
    }
    if (!strlen(ptr)) {
        return;
    }

    this->sensorCount = 0;
    this->readConfigCount = 0;
    this->readConfigAliasCount = 0;

    size_t len = strlen(ptr);
    int part = 0;
    size_t position = 0;
    for (size_t i = 0; i < len; ++i) {
        if (ptr[i] == ';') {
            part++;
            this->processParseConfig(part, ptr + position, i - position);
            position = i + 1;
        }
    }
}

inline
bool ERaApplication::updateHashID(const char* hash) {
    if (hash == nullptr) {
        return false;
    }
    if (strcmp(this->hashID, hash)) {
        snprintf(this->hashID, sizeof(this->hashID), hash);
        return true;
    }
    return false;
}

inline
void ERaApplication::deleteAll() {
    this->clear(this->sensorDelay);
    this->clear(this->modbusConfigParam);
    this->clear(this->modbusConfigAliasParam);
    *this = {};
}

inline
void ERaApplication::processParseConfig(int part, char* ptr, size_t len) {
	switch(part) {
		case ParseConfigT::PARSE_CONFIG_ID:
			this->processParseConfigId(ptr, len);
			break;
		case ParseConfigT::PARSE_BAUD_SPEED:
			this->processParseConfigBaudSpeed(ptr, len);
			break;
		case ParseConfigT::PARSE_TOTAL_ROW:
			this->processParseConfigTotalRow(ptr, len);
			break;
		case ParseConfigT::PARSE_SENSOR_DISPLAY:
			this->processParseConfigSensorDelay(ptr, len);
			break;
		case ParseConfigT::PARSE_SENSOR_READ_WRITE:
			this->processParseConfigSensorReadWrite(ptr, len);
			break;
		case ParseConfigT::PARSE_LOCAL_ALARM:
			this->processParseConfigLocalAlarm(ptr, len);
			break;
		case ParseConfigT::PARSE_CONTACT_PHONE:
			this->processParseStationContactPhone(ptr, len);
			break;
		case ParseConfigT::PARSE_IS_WIFI:
			this->processParseIsWifi(ptr, len);
			break;
		case ParseConfigT::PARSE_CONFIG_WIFI:
    		this->processParseConfigWifi(ptr, len);
			break;
		case ParseConfigT::PARSE_CONFIG_ALIAS_TOTAL_ROW:
			this->processParseConfigAliasTotalRow(ptr, len);
			break;
		case ParseConfigT::PARSE_CONFIG_ALIAS_DATA:
			this->processParseConfigAliasData(ptr, len);
			break;
		case ParseConfigT::PARSE_IS_BLUETOOTH:
			this->processParseIsEnableBluetooth(ptr, len);
			break;
		default:
			break;
	}
}

inline
void ERaApplication::processParseConfigId(char* ptr, size_t len) {
    LOC_BUFFER_PARSE

    memcpy(buf, ptr, len);
    this->id = atoi(buf);

    FREE_BUFFER_PARSE
}

inline
void ERaApplication::processParseConfigBaudSpeed(char* ptr, size_t len) {
    LOC_BUFFER_PARSE

    memcpy(buf, ptr, len);

    if (!strchr(buf, ',')) {
        this->baudSpeed = atoi(buf);
        FREE_BUFFER_PARSE
        return;
    }

    unsigned int step {0};
    char* token = strtok(buf, ",");
    while (token != nullptr) {
        switch (step++) {
			case ParseIntervalT::PARSE_MODBUS_BAUDRATE:
				this->baudSpeed = atoi(token);
				break;
			case ParseIntervalT::PARSE_MODBUS_INTERVAL:
				this->modbusInterval.delay = (atoi(token) < DEFAULT_MIN_MODBUS_INTERVAL ? DEFAULT_MIN_MODBUS_INTERVAL : atoi(token));
				break;
			case ParseIntervalT::PARSE_PUB_MODBUS_INTERVAL:
				this->pubInterval.delay = (atoi(token) < DEFAULT_MIN_MODBUS_INTERVAL ? DEFAULT_MIN_MODBUS_INTERVAL : atoi(token));
				break;
			default:
				break;
        }
        token = strtok(nullptr, ",");
    }

    FREE_BUFFER_PARSE
}

inline
void ERaApplication::processParseConfigTotalRow(char* ptr, size_t len) {
    ERA_FORCE_UNUSED(ptr);
    ERA_FORCE_UNUSED(len);
}

inline
void ERaApplication::processParseConfigSensorDelay(char* ptr, size_t len) {
    LOC_BUFFER_PARSE

    bool newSensor {true};
    size_t bufLen {0};

    this->clear(this->sensorDelay);

    SensorDelay_t* sensor = nullptr;
    for (size_t i = 0; i < len; ++i) { 
        buf[bufLen++] = ptr[i];

        if (ptr[i] == ',') {
            buf[--bufLen] = '\0';
            bufLen = 0;

            if (newSensor) {
                newSensor = false;
                sensor = this->create<SensorDelay_t>();
                if (sensor != nullptr) {
                    sensor->address = atoi(buf);
                }
            }
            else {
                newSensor = true;
                if (sensor != nullptr) {
                    sensor->delay = atoi(buf);
                    this->sensorDelay.put(sensor);
                    this->sensorCount++;
                }
                if (this->sensorCount >= MAX_DEVICE_MODBUS) {
                    break;
                }
            }
        }
    }

    FREE_BUFFER_PARSE
}

inline
void ERaApplication::processParseConfigSensorReadWrite(char* ptr, size_t len) {
    size_t position {0};

    this->clear(this->modbusConfigParam);

    for (size_t i = 0; i < len; ++i) {
        if (ptr[i] == '.') {
            ModbusConfig_t* config = this->create<ModbusConfig_t>();
            if (config == nullptr) {
                continue;
            }
            this->parseOneConfigSensorReadWrite(ptr + position, i - position, *config);
            position = i + 1;
            this->modbusConfigParam.put(config);
            if (this->readConfigCount++ >= MAX_DEVICE_MODBUS) {
                break;
            }
        }
    }
}

inline
void ERaApplication::parseOneConfigSensorReadWrite(char* ptr, size_t len, ModbusConfig_t& config) {
    LOC_BUFFER_PARSE

    size_t position {0};
    size_t configIndex {0};
    int configParam[20] {0};

    for (size_t i = 0; i < len; ++i) {
        buf[position++] = ptr[i];
        if (ptr[i] == ',') {
            buf[--position] = '\0';
            configParam[configIndex++] = atoi(buf);
            position = 0;
            if (configIndex >= 20) {
                break;
            }
        }
    }
    this->actOneConfigSensorReadWrite(configParam, configIndex, config);

    FREE_BUFFER_PARSE
}

inline
void ERaApplication::actOneConfigSensorReadWrite(int* ptr, size_t len, ModbusConfig_t& config) {
    if (ptr == nullptr) {
        return;
    }
    if (!len) {
        return;
    }

    config.addr = ptr[0];
    config.id = ptr[1];
    config.func = ptr[2];
    config.sa1 = ptr[3];
    config.sa2 = ptr[4];
    config.len1 = ptr[5];
    config.len2 = ptr[6];

    for (size_t i = 0; i < len - 7; ++i) {
        config.extra[i] = ptr[i + 7];
    }
}

inline
void ERaApplication::processParseConfigLocalAlarm(char* ptr, size_t len) {
    size_t position = 0;

    for (size_t i = 0; i < len; ++i) {
        if (ptr[i] == '.') {
            this->parseOneConfigLocalAlarm(ptr + position, i - position);
            position = i + 1;
        }
    }
}

inline
void ERaApplication::parseOneConfigLocalAlarm(char* ptr, size_t len) {
    ERA_FORCE_UNUSED(ptr);
    ERA_FORCE_UNUSED(len);
}

inline
void ERaApplication::processParseStationContactPhone(char* ptr, size_t len) {
    size_t position = 0;

    for (size_t i = 0; i < len; ++i) {
        if (ptr[i] == '.') {
            this->parseOneConfigPhoneNumber(ptr + position, i - position);
            position = i + 1;
        }
    }
}

inline
void ERaApplication::parseOneConfigPhoneNumber(char* ptr, size_t len) {
    ERA_FORCE_UNUSED(ptr);
    ERA_FORCE_UNUSED(len);
}

inline
void ERaApplication::processParseIsWifi(char* ptr, size_t len) {
    if (!len) {
        return;
    }
    this->isWifi = (ptr[0] == '1') ? true : false;
}

inline
void ERaApplication::processParseConfigWifi(char* ptr, size_t len) {
    bool isSsid {true};

    memset(this->ssid, 0, sizeof(this->ssid));
    memset(this->password, 0, sizeof(this->password));

    for (size_t i = 0; i < len; ++i) {
        if (ptr[i] == ',') {
            isSsid = false;
            continue;
        }
        if (isSsid) {
            sprintf(&this->ssid[strlen(this->ssid)], "%c", ptr[i]);
        }
        else {
            sprintf(&this->password[strlen(this->password)], "%c", ptr[i]);
        }
    }
}

inline
void ERaApplication::processParseConfigAliasTotalRow(char* ptr, size_t len) {
    ERA_FORCE_UNUSED(ptr);
    ERA_FORCE_UNUSED(len);
}

inline
void ERaApplication::processParseConfigAliasData(char* ptr, size_t len) {
    size_t position {0};

    this->clear(this->modbusConfigAliasParam);
    
    for (size_t i = 0; i < len; ++i) {
        if (ptr[i] == '.') {
            ModbusConfigAlias_t* config = this->create<ModbusConfigAlias_t>();
            if (config == nullptr) {
                continue;
            }
            this->processOneConfigAlias(ptr + position, i - position, *config);
            position = i + 1;
            this->modbusConfigAliasParam.put(config);
            if (this->readConfigAliasCount++ >= MAX_DEVICE_MODBUS) {
                break;
            }
        }
    }
}

inline
void ERaApplication::processOneConfigAlias(char* ptr, size_t len, ModbusConfigAlias_t& config) {
    if (len < 2) {
        return;
    }

    size_t position {0};

    for (size_t i = 0; i < len - 1; ++i) {
        if (ptr[i] == ',' && ptr[i + 1] == '-') {
            position = i + 1;
            this->parseOneConfigAlias(ptr, i + 1, config);
            this->processParseAction(ptr + position, len - position, config);
            break;
        }
    }
}

inline
void ERaApplication::parseOneConfigAlias(char* ptr, size_t len, ModbusConfigAlias_t& config) {
    if (len < 36) {
        return;
    }

    LOC_BUFFER_PARSE

    size_t position {0};
    size_t configIndex {0};
    int configParam[20] {0};
    bool isAlias {false};
    char key[37] {0};

    for (size_t i = 0; i < len; ++i) {
        buf[position++] = ptr[i];
        if (ptr[i] == ',') {
            buf[--position] = '\0';
            if (isAlias) {
                memcpy(key, buf, 36);
            }
            else {
                configParam[configIndex++] = atoi(buf);
            }
            position = 0;
            isAlias = true;
            if (configIndex >= 20) {
                break;
            }
        }
    }
    this->actOneConfigAlias(configParam, configIndex, config, key);

    FREE_BUFFER_PARSE
}

inline
void ERaApplication::actOneConfigAlias(int* ptr, size_t len, ModbusConfigAlias_t& config, const char* key) {
    if (!len) {
        return;
    }
    if (strlen(key) != 36) {
        return;
    }
    config.id = ptr[0];
    memcpy(config.key, key, 36);
}

inline
void ERaApplication::processParseAction(char* ptr, size_t len, ModbusConfigAlias_t& config) {
    size_t position {0};
    size_t numHyphen {0};

    config.readActionCount = 0;

    for (size_t i = 0; i < len; ++i) {
        if (ptr[i] == '-') {
            ++numHyphen;

            if (numHyphen == 2) {
                numHyphen = 0;
                this->parseOneAction(ptr + position + 1, i - position, config);
                position = i + 1;
                config.readActionCount++;
            }
        }
    }
}

inline
void ERaApplication::parseOneAction(char* ptr, size_t len, ModbusConfigAlias_t& config) {
    LOC_BUFFER_PARSE

    size_t position {0};
    size_t configIndex {0};
    int configParam[20] {0};

    for (size_t i = 0; i < len; ++i) {
        buf[position++] = ptr[i];
        if (ptr[i] == ',') {
            buf[--position] = '\0';
            configParam[configIndex++] = atoi(buf);
            position = 0;
            if (configIndex >= 20) {
                break;
            }
        }
    }
    this->actOneAction(configParam, configIndex, config);

    FREE_BUFFER_PARSE
}

inline
void ERaApplication::actOneAction(int* ptr, size_t len, ModbusConfigAlias_t& config) {
    Action_t& action = config.action[config.readActionCount];

    action.id = ptr[0];
    action.len1 = ptr[1];
    action.len2 = ptr[2];
    for (size_t i = 0; i < len - 3; ++i) {
        action.extra[i] = ptr[i + 3];
    }
}

inline
void ERaApplication::processParseIsEnableBluetooth(char* ptr, size_t len) {
    if (!len) {
        return;
    }
    this->isBluetooth = (ptr[0] == '1') ? true : false;
}

#endif /* INC_ERA_PARSE_HPP_ */

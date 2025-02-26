#ifndef INC_ERA_SIMPLE_MBED_GSM_HPP_
#define INC_ERA_SIMPLE_MBED_GSM_HPP_

#define ERA_NO_RTOS

#include <ERa/ERaApiMbedDef.hpp>
#include <Adapters/ERaGsmClient.hpp>
#include <ERa/ERaApiMbed.hpp>
#include <Modbus/ERaModbusArduino.hpp>
#include <Zigbee/ERaZigbeeArduino.hpp>
#include <Utility/ERaFlashMbed.hpp>
#include <Task/ERaTaskMbed.hpp>

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ERA)
    static ERaFlash flash;
    static ERaMqtt<TinyGsmClient, MQTTClient> mqtt;
    ERaGsm< ERaMqtt<TinyGsmClient, MQTTClient> > ERa(mqtt, flash);
#else
    extern ERaGsm< ERaMqtt<TinyGsmClient, MQTTClient> > ERa;
#endif

#include <ERa/ERaStatic.hpp>

#endif /* INC_ERA_SIMPLE_MBED_GSM_HPP_ */

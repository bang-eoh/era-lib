#ifndef INC_ERA_SIMPLE_SERIAL_HPP_
#define INC_ERA_SIMPLE_SERIAL_HPP_

#define ERA_NO_RTOS

#include <ERa/ERaApiArduinoDef.hpp>
#include <Adapters/ERaSerialClient.hpp>
#include <ERa/ERaApiArduino.hpp>
#include <Modbus/ERaModbusArduino.hpp>
#include <Utility/ERaFlashConfig.hpp>
#include <Task/ERaTaskConfig.hpp>

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ERA)
    static ERaFlash flash;
    static ERaMqtt<ERaClient, MQTTClient> mqtt;
    ERaStream< ERaMqtt<ERaClient, MQTTClient> > ERa(mqtt, flash);
#else
    extern ERaStream< ERaMqtt<ERaClient, MQTTClient> > ERa;
#endif

#include <ERa/ERaStatic.hpp>

#endif /* INC_ERA_SIMPLE_SERIAL_HPP_ */

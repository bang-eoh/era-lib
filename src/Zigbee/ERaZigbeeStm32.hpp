#ifndef INC_ERA_ZIGBEE_STM32_HPP_
#define INC_ERA_ZIGBEE_STM32_HPP_

#include <Zigbee/ERaZigbee.hpp>

HardwareSerial SerialZB(PB11, PB10);

template <class Api>
void ERaZigbee<Api>::configZigbee() {
    SerialZB.begin(9600);
}

template <class Api>
void ERaZigbee<Api>::handleZigbeeData() {
    int length = SerialZB.available();
    if (!length) {
        return;
    }
    int position {0};
    uint8_t receive[length < 256 ? 256 : length] {0};
    uint8_t payload[length < 256 ? 256 : length] {0};
    do {
        receive[position] = SerialZB.read();
    } while (++position < length);
    this->processZigbee(receive, length, (length < 256 ? 256 : length), payload, 0, 0);
}

template <class Zigbee>
ResultT ERaToZigbee<Zigbee>::waitResponse(Response_t rspWait, void* value) {
    int length {0};
    int position {0};
    uart_event_t event;
    Response_t rsp {
        .nwkAddr = NO_NWK_ADDR,
        .type = TypeT::ERR,
        .subSystem = 0,
        .command = 0,
        .zclId = ClusterIDT::NO_CLUSTER_ID,
        .transId = 0,
        .transIdZcl = 0,
        .cmdStatus = ZnpCommandStatusT::INVALID_PARAM
    };
    
	if(!rspWait.timeout || rspWait.timeout > MAX_TIMEOUT) {
		rspWait.timeout = MAX_TIMEOUT;
    }
    uint8_t cmdStatus = ZnpCommandStatusT::INVALID_PARAM;

    unsigned long startMillis = ERaMillis();

    do {
        int length = SerialZB.available();
        if (!length) {
            ERaDelay(10);
            continue;
        }
        position = 0;
        uint8_t receive[length < 256 ? 256 : length] {0};
        uint8_t payload[length < 256 ? 256 : length] {0};
        do {
            receive[position] = SerialZB.read();
        } while (++position < length);
        if (static_cast<Zigbee*>(this)->processZigbee(receive, length, (length < 256 ? 256 : length), payload, 0, 0, &cmdStatus, &rspWait, value)) {
            return ((cmdStatus != ZnpCommandStatusT::INVALID_PARAM) ? static_cast<ResultT>(cmdStatus) : ResultT::RESULT_SUCCESSFUL);
        }
        ERaDelay(10);
    } while (ERaRemainingTime(startMillis, rspWait.timeout));
    return ((cmdStatus != ZnpCommandStatusT::INVALID_PARAM) ? static_cast<ResultT>(cmdStatus) : ResultT::RESULT_TIMEOUT);
}

template <class Zigbee>
void ERaToZigbee<Zigbee>::sendByte(uint8_t byte) {
    SerialZB.write(byte, 1);
    SerialZB.flush();
}

template <class Zigbee>
void ERaToZigbee<Zigbee>::sendCommand(const vector<uint8_t>& data) {
    SerialZB.write(data.data(), data.size());
    SerialZB.flush();
}

#endif /* INC_ERA_ZIGBEE_STM32_HPP_ */

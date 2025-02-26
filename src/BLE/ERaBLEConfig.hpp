#ifndef INC_ERA_BLE_CONFIG_HPP_
#define INC_ERA_BLE_CONFIG_HPP_

#if !defined(ERA_BLE_YIELD)
    #if !defined(ERA_BLE_YIELD_MS)
        #define ERA_BLE_YIELD_MS     10
    #endif
    #if !defined(ERA_NO_YIELD)
        #define ERA_BLE_YIELD()      { ERaDelay(ERA_BLE_YIELD_MS); }
    #else
        #define ERA_BLE_YIELD()      {}
    #endif
#endif

#endif /* INC_ERA_BLE_CONFIG_HPP_ */

#include "key_port.h"
#include "../driver/key.h"
#include "../util/datatype.h"
#include "main.h"


// 按键信息
//  key_t keys[REAL_KEYS_SIZE] = {
//      [0] = {
//          .gpio = ,
//          .pin  = ,
//      }, 
//      [1] = {
//          .gpio = ,
//          .pin  = ,
//      }, 
//      [2] = {
//          .gpio = ,
//          .pin  = ,
//      }, 
//      [3] = {
//         .gpio = ,
//          .pin  = ,
//      }   
// };

u8 key_read_pin(void* gpio, u16 pin)
{
    // 读取按键的GPIO引脚状态，设置对应的引脚状态
    // 例如hal库的实现可以如下。
    // key->key_state = HAL_GPIO_ReadPin(key->gpioPort, key->gpioPin);
    // key->key_state = KEY_STATE_PRESS;

    if(HAL_GPIO_ReadPin((GPIO_TypeDef*) gpio, pin) == GPIO_PIN_SET){
        return KEY_STATE_RELEASE;
    }
    else{
        return KEY_STATE_PRESS;
    }

}

static const key_port_t g_key_port = {
    .read_pin = key_read_pin,
};

bool key_port_init(void)
{
    key_bind_port((key_port_t*)&g_key_port);
    return key_port_is_registered();
}

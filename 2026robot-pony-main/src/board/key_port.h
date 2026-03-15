#ifndef BOARD_KEY_PORT_H
#define BOARD_KEY_PORT_H

#include "../util/datatype.h"

// 根据实际按键个数的情况，定义这个宏，默认是4个按键
#define REAL_KEYS_SIZE 4

bool key_port_init(void);

#endif // !BOARD_KEY_PORT_H

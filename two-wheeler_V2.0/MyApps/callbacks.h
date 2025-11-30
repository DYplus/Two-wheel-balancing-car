
#ifndef __CALLBACKS_H
#define __CALLBACKS_H

#include "struct.h"

// 串口接收完成回调函数的实现
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif // __CALLBACKS_H


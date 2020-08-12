#ifndef CRIMSON_BLE_H
#define CRIMSON_BLE_H

#include <stdbool.h>
#include "app_uart.h"

void crimson_ble_init(void);
void advertising_start(bool erase_bonds);
void uart_event_handle(app_uart_evt_t * p_event);




#endif /* CRIMSON_BLE_H */
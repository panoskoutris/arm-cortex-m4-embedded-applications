#ifndef DHT11_DRIVER_H
#define DHT11_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"
#include "delay.h"

/**
 * @brief Initialize the DHT11 pin.
 * 
 * @param pin The GPIO pin connected to the DHT11 data line.
 */
void dht11_init(Pin pin);

/**
 * @brief Read temperature and humidity from DHT11 sensor.
 * 
 * @param[out] temperature Pointer to store temperature in °C.
 * @param[out] humidity Pointer to store humidity in %.
 * @return int 0 on success, -1 on failure.
 */
int dht11_read(uint8_t* temperature, uint8_t* humidity);

#endif // DHT11_DRIVER_H


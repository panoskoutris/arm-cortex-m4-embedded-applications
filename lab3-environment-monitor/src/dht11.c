#include "dht11.h"

static Pin dht_pin;

void dht11_init(Pin pin) {
    dht_pin = pin;
    gpio_set_mode(dht_pin, PullUp);  // Default state is high
}

// Waits for pin to become level (0 or 1) within timeout (in us)
static int wait_for_level(int level, uint32_t timeout_us) {
    while (gpio_get(dht_pin) != level) {
        if (--timeout_us == 0) return 0;
        delay_us(1);
    }
    return 1;
}

int dht11_read(uint8_t* temperature, uint8_t* humidity) {
    uint8_t data[5] = {0};

    // MCU sends start signal
    gpio_set_mode(dht_pin, Output);
    gpio_set(dht_pin, 0);
    delay_ms(20);  // Pull low for at least 18ms
    gpio_set(dht_pin, 1);
    delay_us(30);
    gpio_set_mode(dht_pin, Input);  // Let sensor respond

    // Wait for sensor response
    if (!wait_for_level(0, 80)) return -1;
    if (!wait_for_level(1, 80)) return -1;

    // Read 40 bits (5 bytes)
    for (int byte = 0; byte < 5; byte++) {
        for (int bit = 7; bit >= 0; bit--) {
            if (!wait_for_level(0, 60)) return -1;
            if (!wait_for_level(1, 70)) return -1;

            delay_us(40); //max 42
            if (gpio_get(dht_pin)) {
                data[byte] |= (1 << bit);
            }
        }
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (data[4] != checksum) return -1;

    if (humidity) *humidity = data[0];
    if (temperature) *temperature = data[2];

    return 0;
}

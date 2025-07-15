#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "uart.h"
#include "queue.h"
#include "timer.h"
#include "dht11.h"
#include "gpio.h"
#include "delay.h"
#include <ctype.h>

#define BUFF_SIZE 64
#define CORRECT_PASSWORD "1234"

Queue rx_queue;
char input_buffer[BUFF_SIZE];
volatile uint8_t input_ready = 0;
volatile uint8_t uart_command_ready = 0;
volatile uint8_t touch_event_flag = 0;

char line_buffer[BUFF_SIZE];
uint8_t line_index = 0;

uint8_t sampling_period_s = 5;
uint8_t display_mode = 2;
uint8_t last_temperature = 20;
uint8_t last_humidity = 30;

volatile uint32_t tick_counter = 0;
volatile uint8_t modeA_flag = 0;

volatile uint8_t modeB_enabled = 0;
volatile uint8_t touch_press_count = 0;

#define ALERT_TEMP_THRESHOLD 25
#define ALERT_HUM_THRESHOLD 60
#define PANIC_TEMP_THRESHOLD 35
#define PANIC_HUM_THRESHOLD 80

uint8_t consecutive_normal_readings = 0;
uint8_t panic_counter = 0;

uint8_t aem_digit1 = 0;
uint8_t aem_digit2 = 0;
uint8_t aem_sum = 0;

volatile uint8_t led_toggle_tick = 0;

int strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) return 1;
        s1++; s2++;
    }
    return (*s1 == *s2) ? 0 : 1;
}

void led_on(void) {
    gpio_set_mode(P_LED, Output);
    gpio_set(P_LED, 1);
}

void led_off(void) {
    gpio_set_mode(P_LED, Output);
    gpio_set(P_LED, 0);
}

void led_toggle(void) {
    static uint8_t state = 0;
    state = !state;
    gpio_set_mode(P_LED, Output);
    gpio_set(P_LED, state);
}

void print_menu(void) {
    uart_print("\r\n=== Environmental Control Menu ===\r\n");
    uart_print("Options:\r\n");
    uart_print("a: Decrease sampling period by 1s (min 2s)\r\n");
    uart_print("b: Increase sampling period by 1s (max 10s)\r\n");
    uart_print("c: Toggle display mode (Temp / Hum / Both)\r\n");
    uart_print("d: Print latest readings and system status\r\n");
}

void process_uart_command(char cmd) {
    char msg[64];
    switch (cmd) {
        case 'a':
            if (sampling_period_s > 2) {
                sampling_period_s--;
                sprintf(msg, "New sampling period: %ds\r\n", sampling_period_s);
            } else {
                sprintf(msg, "Already at minimum period (2s)\r\n");
            }
            uart_print(msg);
            break;
        case 'b':
            if (sampling_period_s < 10) {
                sampling_period_s++;
                sprintf(msg, "New sampling period: %ds\r\n", sampling_period_s);
            } else {
                sprintf(msg, "Already at maximum period (10s)\r\n");
            }
            uart_print(msg);
            break;
        case 'c':
            display_mode = (display_mode + 1) % 3;
            const char* modes[] = {"Temperature", "Humidity", "Both"};
            sprintf(msg, "Display mode set to: %s\r\n", modes[display_mode]);
            uart_print(msg);
            break;
        case 'd':
            sprintf(msg, "System status: Sampling = %ds, Mode = %s\r\n",
                    sampling_period_s, modeB_enabled ? "B (Alert)" : "A (Normal)");
            uart_print(msg);
            sprintf(msg, "Latest values: Temp = %d °C, Humidity = %d %%\r\n",
                    last_temperature, last_humidity);
            uart_print(msg);
            break;
        default:
            uart_print("Unknown command.\r\n");
            break;
    }
}

// === Interrupt Callbacks ===
void uart_rx_callback(uint8_t ch) {
    if (ch == '\r') {
        line_buffer[line_index] = '\0';
        input_ready = 1;
        uart_command_ready = 1;
        line_index = 0;
    } else if (ch >= 32 && ch < 127 && line_index < BUFF_SIZE - 1) {
        line_buffer[line_index++] = ch;
        uart_tx(ch);
    }
}

void timer_callback(void) {
    tick_counter++;

    // Main sampling flag
    if (tick_counter >= sampling_period_s * 10) {
        tick_counter = 0;
        modeA_flag = 1;
    }

    // LED toggle every 1s in alert mode with unsafe values
    led_toggle_tick++;
    if (led_toggle_tick >= 10) {
        led_toggle_tick = 0;
        if (modeB_enabled &&
            (last_temperature > ALERT_TEMP_THRESHOLD || last_humidity > ALERT_HUM_THRESHOLD)) {
            led_toggle();
        }
    }
}

void touch_callback(int pin) {
    touch_event_flag = 1;
}

int main(void) {
    queue_init(&rx_queue, BUFF_SIZE);
    uart_init(115200);
    uart_set_rx_callback(uart_rx_callback);
    uart_enable();
    __enable_irq();

    uart_print("\r\n=== Environmental Control System ===\r\n");

    while (1) {
        uart_print("Enter password: ");
        line_index = 0;
        while (!input_ready) __WFI();
        input_ready = 0;
        strcpy(input_buffer, line_buffer);
        uart_print("\r\n");

        if (strcmp(input_buffer, CORRECT_PASSWORD) == 0) {
            uart_print("Password accepted.\r\n");
            break;
        } else {
            uart_print("Incorrect password. Try again.\r\n");
        }
    }

    uart_print("Enter your AEM: ");
    line_index = 0;
    while (!input_ready) __WFI();
    input_ready = 0;
    strcpy(input_buffer, line_buffer);
    uart_print("\r\n");

    uart_print("AEM received: ");
    uart_print(input_buffer);
    uart_print("\r\n");

    uint8_t len = strlen(input_buffer);
    aem_digit1 = input_buffer[len - 2] - '0';
    aem_digit2 = input_buffer[len - 1] - '0';
    aem_sum = aem_digit1 + aem_digit2;
    char msg[64];
    sprintf(msg, "Last 2 digits: %d and %d (Sum = %d)\r\n", aem_digit1, aem_digit2, aem_sum);
    uart_print(msg);

    print_menu();

    dht11_init(P_DHT);
    delay_ms(2000);
    gpio_set_mode(P_TOUCH, PullUp);
    gpio_set_trigger(P_TOUCH, Rising);
    gpio_set_callback(P_TOUCH, touch_callback);

    timer_set_callback(timer_callback);
    timer_init(100000);  // 100 ms
    timer_enable();

    while (1) {
        if (touch_event_flag) {
            touch_event_flag = 0;
            modeB_enabled ^= 1;
            touch_press_count++;

            if (touch_press_count % 3 == 0) {
                sampling_period_s = aem_sum;
                if (sampling_period_s < 2) sampling_period_s = 2;
                if (sampling_period_s > 10) sampling_period_s = 10;
                sprintf(msg, "Touch x3 ? New sampling period: %ds\r\n", sampling_period_s);
                uart_print(msg);
            }

            sprintf(msg, "Touch detected. Alert mode %s. Total presses: %d\r\n",
                    modeB_enabled ? "ON" : "OFF", touch_press_count);
            uart_print(msg);
        }

        if (modeA_flag) {
            modeA_flag = 0;

            if (dht11_read(&last_temperature, &last_humidity) == 0) {
                if (last_temperature > PANIC_TEMP_THRESHOLD || last_humidity > PANIC_HUM_THRESHOLD) {
                    panic_counter++;
                    if (panic_counter >= 3) {
                        uart_print("[PANIC] Unsafe environment! Resetting system...\r\n");
                        NVIC_SystemReset();
                    }
                } else {
                    panic_counter = 0;
                }

                if (display_mode == 0)
                    sprintf(msg, "[TEMP] %d °C\r\n", last_temperature);
                else if (display_mode == 1)
                    sprintf(msg, "[HUM]  %d %%\r\n", last_humidity);
                else
                    sprintf(msg, "[TEMP] %d °C | [HUM] %d %%\r\n", last_temperature, last_humidity);
                uart_print(msg);

                if (modeB_enabled) {
                    if (last_temperature > ALERT_TEMP_THRESHOLD || last_humidity > ALERT_HUM_THRESHOLD) {
                        consecutive_normal_readings = 0;
                    } else {
                        consecutive_normal_readings++;
                        if (consecutive_normal_readings >= 5) led_off();
                    }
                }
            } else {
                uart_print("DHT11 read failed.\r\n");
            }
        }

        if (input_ready && uart_command_ready) {
            uart_command_ready = 0;
            input_ready = 0;
            strcpy(input_buffer, line_buffer);
            uart_print("\r\n");

            if (strcasecmp(input_buffer, "status") == 0) {
                char status_msg[128];
                sprintf(status_msg,
                        "[STATUS]\r\nMode: %s\r\nTemp: %d °C\r\nHum: %d %%\r\nProfile changes: %d\r\n",
                        modeB_enabled ? "B (Alert)" : "A (Normal)",
                        last_temperature,
                        last_humidity,
                        touch_press_count);
                uart_print(status_msg);
            } else if (strlen(input_buffer) == 1) {
                process_uart_command(input_buffer[0]);
            } else {
                uart_print("Unknown command.\r\n");
            }
        }

        __WFI();
    }
}

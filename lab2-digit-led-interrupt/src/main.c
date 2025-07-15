#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "uart.h"
#include "queue.h"
#include "timer.h"
#include "leds.h"
#include "delay.h"
#include "gpio.h"

#define BUFF_SIZE 128

Queue rx_queue; // UART input queue (FIFO buffer for received characters)

char input_buffer[BUFF_SIZE]; // Stores the user input string (digits + optional '-')

volatile uint32_t input_ready = 0;         // Set by UART ISR when Enter ('\r') is pressed
volatile uint32_t new_input_override = 0;  // Set if Enter is pressed while analysis is active (used to cancel current analysis)

volatile uint8_t button_event_flag = 0;    // Set by button ISR, handled in main loop
volatile uint8_t timer500_flag = 0;        // Set every 500ms (5 timer ticks), triggers digit processing
volatile uint8_t timer200_flag = 0;        // Set every 200ms (2 timer ticks), used for LED blinking

uint32_t tick_counter = 0;     // Increments every 100ms tick (used to derive 200ms/500ms)
uint32_t digit_index = 0;      // Index of the current digit being processed in input_buffer

uint8_t led_locked = 0;        // 1 = LED actions are blocked (toggled by button), 0 = LED allowed
uint8_t analysis_active = 0;   // 1 = system is currently processing input digits
uint8_t repeat_flag = 0;       // 1 = repeat mode ('-' at the end of input), 0 = one-time input
uint8_t blinking_active = 0;   // 1 = even digit is being processed; LED should blink
uint8_t led_on_state = 0;      // Shadow of LED state: 1 = ON, 0 = OFF (used for toggling logic)


// LED helper functions
void led_on(void) {
    leds_set(1, 0, 0);
    led_on_state = 1;
}
void led_off(void) {
    leds_set(0, 0, 0);
    led_on_state = 0;
}
void led_toggle(void) {
    if (led_on_state) led_off();
    else led_on();
}

// Reset process
void reset_analysis_state(void) {
    new_input_override = 0;
    analysis_active = 0;
    repeat_flag = 0;
    digit_index = 0;
    input_buffer[0] = '\0';
    timer_disable();
    blinking_active = 0;
}

// The process for every timer interrupt (non-blocking version) (NOT the isr/callback function)
void process_timer_interrupt(void) {
    char msg[32];

    if (new_input_override) {
        reset_analysis_state();
        return;
    }

    if (input_buffer[digit_index] == '\0') {
        if (repeat_flag) {
            digit_index = 0;
            return;
        }

        uart_print("End of sequence. Waiting for new number...\r\n");
        reset_analysis_state();
        uart_print("Enter a number: ");
        return;
    }

    char ch = input_buffer[digit_index++];
    if (ch < '0' || ch > '9') return;

    int digit = ch - '0';
    sprintf(msg, "Digit %d -> ", digit);
    uart_print(msg);

    if (!led_locked) {
        if (digit % 2 == 0) {
            uart_print("Blink LED\r\n");
            blinking_active = 1;
        } else {
            uart_print("Toggle LED\r\n");
            blinking_active = 0;
            led_toggle();
        }
    } else {
        uart_print("Skipped LED action\r\n");
    }
}

// The process for 200ms blinking (non-blocking blink)
void process_blinking_toggle(void) {
    if (blinking_active && !led_locked) {
        led_toggle();
    }
}

// The process for every button interrupt
void process_button_interrupt(void) {
    static int counter = 1;
    char msg[64];

    led_locked = !led_locked;
    if (led_locked) {
        sprintf(msg, "Interrupt: Button pressed. LED locked. Count = %d\r\n", counter);
    } else {
        sprintf(msg, "Interrupt: Button pressed. LED unlocked. Count = %d\r\n", counter);
    }
    uart_print(msg);
    counter++;
}

// Timer isr/calllback function
void timer_callback(void) {
    tick_counter++;

    if (tick_counter % 5 == 0) {
        timer500_flag = 1;
    }
    if (blinking_active && tick_counter % 2 == 0) { 
        timer200_flag = 1;
    }
}

// UART receive ISR
void uart_rx_callback(uint8_t rx) {
    if ((rx >= '0' && rx <= '9') || rx == '-' || rx == '\r') {
        queue_enqueue(&rx_queue, rx);
    }
    if (rx == '\r') {
        input_ready = 1;
        if (analysis_active) {
            new_input_override = 1;
        }
    }
}

// Button isr/callback function
void button_callback(int pin) {   // Pin is not used but it shows error without it 
    button_event_flag = 1;
}

// Button initializing function
void button_init(void) {
    gpio_set_mode(P_SW, PullUp);
    gpio_set_trigger(P_SW, Rising);
    gpio_set_callback(P_SW, button_callback);
    NVIC_SetPriority(EXTI15_10_IRQn, 1);
}

int main(void) {
    uint8_t rx_char;
    uint32_t index;

    queue_init(&rx_queue, BUFF_SIZE);
    uart_init(115200);
    uart_set_rx_callback(uart_rx_callback);
    uart_enable();
    leds_init();
    button_init();
    __enable_irq();

    uart_print("\r\nEnter a number: ");

    while (1) {
        if (timer500_flag) {
            timer500_flag = 0;
            process_timer_interrupt();
        }

        if (timer200_flag) {
            timer200_flag = 0;
            process_blinking_toggle();
        }

        if (button_event_flag) {
            button_event_flag = 0;
            process_button_interrupt();
        }

        if (!analysis_active) {
            index = 0;
            input_ready = 0;

            while (!input_ready && index < BUFF_SIZE - 1) {
                while (!queue_dequeue(&rx_queue, &rx_char))
                    __WFI();

                uart_tx(rx_char);
                if (rx_char == '\r') {
                    uart_print("\r\n");
                    break;
                }
                input_buffer[index++] = (char)rx_char;
            }

            input_buffer[index] = '\0';
            if (index > 0 && input_buffer[index - 1] == '-') {
                repeat_flag = 1;
            } else {
                repeat_flag = 0;
            }						
            digit_index = 0;
            analysis_active = 1;
            new_input_override = 0;

            timer_set_callback(timer_callback);
            timer_init(100000); // 100ms tick
            timer_enable();
        }
    }
}


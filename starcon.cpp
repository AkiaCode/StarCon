#include "Adafruit_TinyUSB_Arduino/src/Adafruit_TinyUSB.h"
#include "TinyUSB_Mouse_and_Keyboard/TinyUSB_Mouse_and_Keyboard.h"
#include "hardware/gpio.h"
#include "includes/Adafruit_USBD_CDC-stub.h"
#include "includes/usb.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <map>

// GPIO pins the keyswitch is on
#define BUTTON_PINS \
    { 0, 5, 18, 13, 22, 27, 28 }
// Debounce delay (ms)
#define DEBOUNCE_DELAY 5

// Adafruit TinyUSB instance
extern Adafruit_USBD_Device TinyUSBDevice;

static bool loopTask(repeating_timer_t *rt){
    led_blinking_task();
    return true;
}

int main() {
    std::map<int, uint8_t> KEYS;
    KEYS[0] = 'd';
    KEYS[5] = 'f';
    KEYS[18] = 'j'; 
    KEYS[13] = 'k'; 
    KEYS[22] = 's'; 
    KEYS[27] = ' ';
    KEYS[28] = 'l';

    
    bi_decl(bi_program_description("Sixtar Gate: STARTRAIL Controller"));
    bi_decl(bi_program_feature("USB HID Device"));
    
    board_init();
    TinyUSBDevice.begin();  // Initialise Adafruit TinyUSB
    
    struct repeating_timer timer;
    add_repeating_timer_ms(10, loopTask, NULL, &timer);

    // Initialise a keyboard (code will wait here to be plugged in)
    Keyboard.begin();

    // Initise GPIO pin as input with pull-up
    for (int pin : BUTTON_PINS) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }

    // Variables for detecting key press
    std::map<int, bool> lastState; 
    lastState[0] = true; 
    lastState[5] = true;
    lastState[18] = true;
    lastState[13] = true;
    lastState[22] = true;
    lastState[27] = true;
    lastState[28] = true; // pulled up by default

    std::map<int, uint32_t> lastTime;
    lastTime[0] = to_ms_since_boot(get_absolute_time());
    lastTime[5] = to_ms_since_boot(get_absolute_time());
    lastTime[18] = to_ms_since_boot(get_absolute_time());
    lastTime[13] = to_ms_since_boot(get_absolute_time());
    lastTime[22] = to_ms_since_boot(get_absolute_time());
    lastTime[27] = to_ms_since_boot(get_absolute_time());
    lastTime[28] = to_ms_since_boot(get_absolute_time());
    // i mean this works too

    // Main loop
    while (1) {
        // Check GPIO pin, and if more than DEBOUNCE_DELAY ms have passed since
        // the key changed press release key depending on value (delay is for
        // debounce, ie to avoid rapid changes to switch value)
        for (int pin : BUTTON_PINS) {
            bool state = gpio_get(pin);
            uint8_t key = KEYS[pin];
            uint32_t now = to_ms_since_boot(get_absolute_time());
            if (now - lastTime[pin] >= DEBOUNCE_DELAY) {
                if (state) {
                    Keyboard.release(key);
                } else {
                    Keyboard.press(key);
                }
                lastTime[pin] = now;
                lastState[pin] = state;
            }
        }
    }
}
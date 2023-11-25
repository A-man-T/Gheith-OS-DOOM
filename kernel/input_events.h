#ifndef _INPUTEVENTS_H_
#define _INPUTEVENTS_H_
#include "stdint.h"


struct InputEvent { // see README.md for more details
    uint8_t type : 2; 
    uint8_t code : 5; 
    int16_t value : 9; 
    uint16_t time : 16; // timestamp returns the time at which the event happened
} __attribute__ ((packed)); //32 bit version of linux input event (so we can return it from a syscall)

class InputEventsUtils {
    public:
        static InputEvent scan_code_to_event(uint8_t scancode, bool e0_last);
        static InputEvent mouse_buttons_to_event(uint8_t button_num, bool press);
        static InputEvent mouse_move_to_event(int16_t movement, uint8_t axis);
        static InputEvent snc_event();
        static uint32_t ev_to_int(InputEvent event);
        static InputEvent int_to_ev(uint32_t i);
};
#endif

#include "input_events.h"
#include "scancodes.h"
#include "pit.h"
#include "ps2.h"

uint16_t get_compressed_time(){
    return (Pit::jiffies >> 6) & 0xFFFF;
}

InputEvent InputEventsUtils::scan_code_to_event(uint8_t scancode, bool e0_last){ 
    //I now realize there's probably a way to format the struct and simply cast it because the difference between a press and release is 0x80 but i think this works too
    InputEvent event{};
    event.type = EV_KEY & 3;

    if(e0_last){
        if (scancode >= 0x47 && scancode <= 0x53){
            event.value = scancode + 30;
            if (PS2::is_pressed(event.value)){
                event.code = KEY_HELD; //held
            } else {
                event.code = KEY_PRESS; //new press 
            }
        } else if (scancode >= 0xC7 && scancode <= 0xD3){
            event.value = scancode - 0x80 + 30;
            event.code = KEY_RELEASE; //release
        } else {
            Debug::printf("scancode %x preceded by E0 not handled", scancode);
        }
    }
    else if (scancode >= 1 && scancode <= 0x58){
        event.value = scancode;
        if (PS2::is_pressed(event.value)){
            event.code = KEY_HELD; //held
        } else {
            event.code = KEY_PRESS; //new press 
        }
    } else if (scancode >= 0x81 && scancode <= 0xD8){
        event.value = scancode - 0x80;
        event.code = KEY_RELEASE; //release
    } else {
        Debug::printf("scancode %x not handled", scancode);
    }
    event.time = get_compressed_time();
    return event;
}

InputEvent InputEventsUtils::mouse_buttons_to_event(uint8_t button_num, bool press){
    InputEvent event{};
    event.type = EV_KEY & 3;
    event.value = button_num + BTN_LEFT;
    if (press){ 
        event.code = KEY_PRESS;
    } else { 
        event.code = KEY_RELEASE;
    }
    event.time = get_compressed_time();
    return event;
}

InputEvent InputEventsUtils::mouse_move_to_event(int16_t movement, uint8_t axis){
    InputEvent event{};
    event.type = EV_REL & 3;
    event.code = axis;
    movement = movement >= 0 ? movement : movement | (0xFF00); // retains negative after truncation for larger than 8 bit ints
    event.value = movement;
    event.time = get_compressed_time();
    return event;
}

InputEvent InputEventsUtils::snc_event(){
    InputEvent event{};
    event.type = EV_SYN;
    event.code = 1; //this doesn't mean anything i just want to preserve default value of 0
    event.time = get_compressed_time();
    return event;
}

uint32_t InputEventsUtils::ev_to_int(InputEvent event){
    int i = 0;
    i = ((event.type & 0x3) << 30) | ((event.code & 0x1F) << 25) | ((event.value & 0x1FF) << 16) | (event.time & 0xFFFF);
    return i;
}

InputEvent InputEventsUtils::int_to_ev(uint32_t i){
    InputEvent event{};
    event.type = (i >> 30) & 0x3;
    event.code = (i >> 25) & 0x1F;
    event.value = (i >> 16) & 0x1FF;
    event.time = i & 0xFFFF;
    return event;
}
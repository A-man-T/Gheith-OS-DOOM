#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_
#include "stdint.h"
#include "debug.h"
#include "circular_buffer.h"
#include "input_events.h"

class PS2{
    public:
        static int cur_packet_index;
        static bool e0_last;
        static uint8_t packet[4];        

        static CircularBuffer<struct InputEvent> key_events;
        static CircularBuffer<struct InputEvent> mouse_events;
        static uint8_t key_presses[126];
        static void disable();
        static void init();
        
        static void write_key_event(uint8_t key, bool last_e0);

        static InputEvent read_key_event(){
            return key_events.remove();
        }
        static InputEvent read_mouse_event(){
            return mouse_events.remove();
        }

        static void handle_packet(uint8_t byte0, uint8_t byte1, uint8_t byte2,uint8_t byte3);

        static void handle_packet();
    
        static bool is_pressed(int key) {
            return key_presses[key] > 0;
        }
        static bool is_held(int key) {
            return key_presses[key] == 2;
        }
};


#endif

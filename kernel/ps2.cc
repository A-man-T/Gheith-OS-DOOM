#include "ps2.h"
#include "idt.h"
#include "smp.h"
#include "config.h"
#include "scancodes.h"
#include "machine.h"
#include "irq.h"

//init globals
int PS2::cur_packet_index = 0;
uint8_t PS2::packet[4];
bool PS2::e0_last = false;

CircularBuffer<InputEvent> PS2::key_events{10000};
CircularBuffer<InputEvent> PS2::mouse_events{10000};
uint8_t PS2::key_presses[126];

uint8_t read_data_port(){
    while ((inb(0x64) & 1) == 0) pause(); //to write to 60 wait for first bit to be 1
    return inb(0x60);
}
uint8_t read_status(){
    return inb(0x64);
}

void write_command_port(uint8_t command){ //send to ps2 controller
    while ((inb(0x64) & 0b10) != 0) pause(); //to write to 64 wait for second bit to be 0
    outb(0x64, command);
}

void address_mouse(){ //send next data port command to second PS2 device
    write_command_port(0xD4);
}

void write_data_port(uint8_t command){ //send to device
    while ((inb(0x64) & 0b10) != 0) pause(); //to write to 64 wait for second bit to be 0
    outb(0x60, command);
}

void get_ack(){
    uint8_t ack = read_data_port();
    ASSERT(ack == 0xFA);
}

uint8_t identify(){ //id ps port 2
    address_mouse();
    write_data_port(0xF2);
    get_ack();
    return read_data_port();
}
void set_mouse_rate(uint8_t rate){
    address_mouse(); 
    write_data_port(0xF3);          
    get_ack();                     
    address_mouse();               
    write_data_port(rate); 
    get_ack();
}

uint8_t magic_z_axis(){
    set_mouse_rate(200);  
    set_mouse_rate(100);
    set_mouse_rate(80);
    return identify();
}

uint8_t magic_5_button(){ //qemu default mouse is 5 button
    if (!(identify() == 3)) return identify();
    set_mouse_rate(200);  
    set_mouse_rate(200);
    set_mouse_rate(80);
    return identify();
}

void PS2::init(){ 
    Debug::printf("| initalizing PS/2 keyboard and mouse\n");

    write_command_port(0xAD); //disable keyboard
    write_command_port(0xA7); //disable mouse
    inb(0x60); //flush data port (no need to check if anything's there)
    
    //qemu does 5 button mouse mode so lets match that
    uint8_t id = magic_z_axis();
    id = magic_5_button();
    ASSERT(id == 0x4);

    //disable stuff controller config byte
    write_command_port(0x20); // read controller config byte
    uint8_t status_byte = read_data_port(); //status /controller configuration byte
    status_byte = status_byte & 0b11011100;
    write_command_port(0x60); //Then send command byte 0x60 ("Set Compaq Status") to port 0x64
    write_data_port(status_byte); //followed by the modified Status byte to port 0x60 to actually write the statys

    //enable mouse (packet streaming)
    address_mouse();
    write_data_port(0xF4);
    get_ack();

    write_command_port(0xA8);  // enable mouse
    write_command_port(0xAE);  // enable key

    //enable stuff on controller cofig byte
    write_command_port(0x20); // read controller config byte
    status_byte = read_data_port(); //status/controller configuration byte
    status_byte = status_byte | 0b00000011; //enable ports 
    status_byte = status_byte & 0b11001111; //enable clocks 
    write_command_port(0x60); //Then send command byte 0x60 ("Set Compaq Status") to port 0x64
    write_data_port(status_byte); //write controller config byte

    //map mouse and keyboard to PS2 Interrupt Handler
    IRQ::map_IRQ(1, 0x21, (uint32_t)PS2InterruptHandler_);
    IRQ::map_IRQ(12, 0x21, (uint32_t)PS2InterruptHandler_);
    Debug::printf("| finished PS/2 keyboard and mouse initialization\n");

}

void PS2::write_key_event(uint8_t key, bool last_e0){
    struct InputEvent event = InputEventsUtils::scan_code_to_event(key, last_e0);
    if (event.value == 0) return;
    key_presses[event.value] = event.code;
    key_events.add(InputEventsUtils::snc_event());
    key_events.add(event);
}

void PS2::handle_packet(uint8_t byte0, uint8_t byte1, uint8_t byte2,uint8_t byte3){ //process a 4 byte mouse packet
    if (((byte0 >> 6) & 1) == 1 || ((byte0 >> 7) & 1) == 1){
        return;
    }
    mouse_events.add(InputEventsUtils::snc_event()); //all of the next events come from the same packet so occur at the same time

	int rel_x = byte1 - ((byte0 << 4) & 0x100);
	int rel_y = byte2 - ((byte0 << 3) & 0x100);

    bool buttons[5];
    buttons[0] = byte0 & 1;
    buttons[1] = byte0 & 0b10;
    buttons[2] = byte0 & 0b100;
    buttons[3] = byte3 & 0b10000;
    buttons[4] = byte3 & 0b100000;

    int button_max = 5;
    if (byte3 & 0b1000000){ //scroll horizontal
        button_max = 3; //buttons 4 and 5 values are used as part of the scroll movement
        if ((byte3 & 0x3F) != 0){
            int16_t h_scroll = (byte3 & 0x1F) - ((byte3 & 0x3F) & 0x20);
            struct InputEvent event = InputEventsUtils::mouse_move_to_event(h_scroll, REL_HWHEEL);
            mouse_events.add(event);
        }
    } else if ((byte3 & 0xF) != 0){
        int16_t v_scroll = (byte3 & 0x7) - ((byte3 & 0x8) & 0xF);
        struct InputEvent event = InputEventsUtils::mouse_move_to_event(v_scroll, REL_WHEEL);
        mouse_events.add(event);
    }
    
    for (int i = 0; i < button_max; i++){
        if (PS2::is_pressed(BTN_LEFT + i) != buttons[i]){
            struct InputEvent event = InputEventsUtils::mouse_buttons_to_event(i, buttons[i]);
            mouse_events.add(event);
            key_presses[BTN_LEFT + i] = event.code;
        }
    }

    struct InputEvent event = InputEventsUtils::mouse_move_to_event(rel_x, REL_X);
    mouse_events.add(event);
    event = InputEventsUtils::mouse_move_to_event(rel_y, REL_Y);
    mouse_events.add(event);
}

extern "C" void PS2InterruptHandler(uint32_t* frame) {
    if (!(0x20 & inb(0x64))) { //if 5th bit set 
        //keyboard interrupt
        uint8_t data = inb(0x60);
        if (data == 0xE0){
            PS2::e0_last = true;
            SMP::eoi_reg.set(0); 
            return;
        }
        bool temp = PS2::e0_last;
        PS2::e0_last = false;
        PS2::write_key_event(data, temp);
    } else {
        //mouse interrupt
        uint8_t data = inb(0x60);        
        PS2::packet[PS2::cur_packet_index] = data;
        if (PS2::cur_packet_index == 0 && ((PS2::packet[0] >> 3) & 1) != 1){
            Debug::printf("4th bit not set\n");
            SMP::eoi_reg.set(0);
            return;
        }
        PS2::cur_packet_index++;
        PS2::cur_packet_index = PS2::cur_packet_index % 4;
        if (PS2::cur_packet_index == 0){
            PS2::handle_packet(PS2::packet[0], PS2::packet[1], PS2::packet[2], PS2::packet[3]);
        }
    }
    SMP::eoi_reg.set(0); 
}


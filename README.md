# README

## DeviceIO 

### Summary
Mouse movements and all key presses generate interrupts which are handled in PS2InterruptHandler(uint32_t* frame) inside ps2.cc. For every movement/keypress, an InputEvent is generated in the interrupt, and added into a nonblocking buffer of device events. Events are dropped if the buffer (currently size 10000) is full. Each IO device has a buffer/queue (mouse_events and key_events). Syscalls are provided for users to process events from the two queues and read their values. Additionally, the state (up/down/held) of keys/buttons  is tracked in the kernel and can be queried.

### Syscalls/User interface:

- [2000] int is_pressed(int key)
    * returns whether or not a particular key has been pressed
    * the key is an integer representation of the actual key on the keyboard/mouse buttons, you can find these mappings in scancodes.h
 
- [2001] int read_key_event()
    * returns the last unprocessed event in the key_event queue as an int. 

- [2003] int read_mouse_event()
    * return the last unprocessed event in the read_mouse_event queue as an int 

- [2004] int is_held(int key)
    * returns whether or not a particular key is being held
    * input is the same format as syscall 2000(is_pressed)

### InputEvents

A device event takes the form of a 32 bit packed struct.
```cpp
struct InputEvent {  
    uint8_t type : 2; 
    uint8_t code : 5;
    int16_t value : 9;
    uint16_t time : 16;
} __attribute__ ((packed));
```
We roughly follow linux standards, with a couple of changes. More information can be found in scancodes.h, https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h, https://wiki.osdev.org/PS2_Keyboard#Scan_Code_Set_1

- type:  
    * EV_REL (2) for relative moment, such as mouse movement or scroll wheel movement
    * EV_KEY (1) for a key event, such as key mouse button press/release 
    * EV_SYN (0) signifies the end of a synchronous group of events. Sent after each keyboard event and each group of mouse events (ie a diagonal movement has an x and y component sent as two events but occur synchronously). 

- code: event code.
    * For type == EV_REL
        * REL_X (0) - horizontal mouse movement 
        * REL_Y (1) - vertical mouse movement   
        * REL_WHEEL (6) - scroll wheel movement (values 1 or -1 in qemu)
        * REL_HWHEEL (8) - horizontal scroll wheel movement (values 1 or -1 in qemu)
    * For type == EV_KEY
        * KEY_PRESS (1)
        * KEY_RELEASE (0)
        * KEY_HELD (2)
        * Note our code and value for keys are flipped from Linux to accommodate our field sizes)

- value: the value the event carries. 
    * For type == EV_REL
        * Relative change (positive or negative 9 bit two's complement int)
    * For type == EV_KEY
        * Key/button identifier (note this is equivalent to key code in Linux) find specific values in scancodes.h
        * Values should range from 1-125
        * KEY_A (30)
        * BTN_LEFT (121)
        * KEY_UP (103) - up arrow
        * Note some interpretations of the same key may change depending on other key states (like shift for capital letters)
        * Additionally keypad keys have a separate key code from potentially equivalent regular keys

- time: unsigned timestamp
    * The time since the OS booted when the event was generated. Unit is 64 ms

### Kernel interface
Other kernel processes may add code/access kernel data, but I will draw attention to some areas that may be important. 
Anywhere mouse_events.add(event) or key_events.add(event) is called is where an event is being added to the queue. 

#### ps2.h -> syscall equivalent functions
```InputEvent PS2::read_key_event()```

```InputEvent PS2::read_mouse_event()```

```bool PS2::is_pressed(int key)```

```bool PS2::is_held(int key)```

#### Scancodes.h
`bool ScancodeHelpers::is_char(int scancode)` -> true if the InputEvent.value corresponds to a character key (a-z, 0-9, punctuation, etc) else false

`bool ScancodeHelpers::is_num(int scancode)` -> true if the InputEvent.value corresponds to a number key 0-9 else false

`char ScancodeHelpers::get_char(int scancode)` -> not implemented but if it would be helpful implement it or let deviceIO know and we can

`int ScancodeHelpers::get_num(int scancode)` -> returns number represented by InputEvent.value, -1 if not a number


### Files:
- kernel/          contains the kernel files
	- circular_buffer.h	
	- input_events.cc/input_events.h
	- irq.h
	- ps2.cc/ps2.h
	- ps2_int.S
	- scancodes.h

### Tests/Syscall examples:
The tests for device IO require display and sdl graphics qemu flag. This requires X forwarding to be set up. (Mac users see notes at the end.) The output depends on user input so there is no .ok file and the program doesn't terminate until it’s manually killed.
Run tests with graphics with `./run_qemu deviceIO_test.{test_name}`.
**Note: When running with graphics, wait until the keyboard/mouse have been configured to generate any input as doing so before will cause it to fail. You can see it it configured when `| finished PS/2 keyboard and mouse initialization` prints.**
- check_pressed.dir: prints changes in the state of the key A using is_held and is_pressed. Refer to for reference on usage of these syscalls
- read_events.dir: Prints mouse and key events to the console. Refer to for reference on read_key_event and read_mouse_event and interpreting the returned int to InputEvent. 


### X-Forwarding Setup for mac:
To set up x forwarding for this application for mac a specific IGLX setting must be enabled. On your local machine ensure you have x quartz installed locally and ssh with -Y (or vs code ssh trusted x forwarding configured). Then you will likely get an error  that looks something like this https://unix.stackexchange.com/questions/429760/opengl-rendering-with-x11-forwarding. 

To resolve this, locally run
`defaults write org.xquartz.X11 enable_iglx -bool true`
You might also want to optionally run
`xhost +`
Then run 
`defaults read org.xquartz.X11`
Ensure it shows `"enable_iglx" = 1;`
Close and reopen xquartz and your ssh session.

Even with this fix I still receive part of the error message 
libGL error: No matching fbConfigs or visuals found libGL error: failed to load driver: swrast
and the mouse movements don’t always get sent, but it should be good enough to test most things and if anyone figures out a fix please lmk. 


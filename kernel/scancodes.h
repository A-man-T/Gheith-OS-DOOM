//we're on scan code set 1
//https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Sets.2C_Scan_Codes_and_Key_Codes
/*
0x01	escape pressed	0x02	1 pressed	0x03	2 pressed
0x04	3 pressed	0x05	4 pressed	0x06	5 pressed	0x07	6 pressed
0x08	7 pressed	0x09	8 pressed	0x0A	9 pressed	0x0B	0 (zero) pressed
0x0C	- pressed	0x0D	= pressed	0x0E	backspace pressed	0x0F	tab pressed
0x10	Q pressed	0x11	W pressed	0x12	E pressed	0x13	R pressed
0x14	T pressed	0x15	Y pressed	0x16	U pressed	0x17	I pressed
0x18	O pressed	0x19	P pressed	0x1A	[ pressed	0x1B	] pressed
0x1C	enter pressed	0x1D	left control pressed	0x1E	A pressed	0x1F	S pressed
0x20	D pressed	0x21	F pressed	0x22	G pressed	0x23	H pressed
0x24	J pressed	0x25	K pressed	0x26	L pressed	0x27	 ; pressed
0x28	' (single quote) pressed	0x29	` (back tick) pressed	0x2A	left shift pressed	0x2B	\ pressed
0x2C	Z pressed	0x2D	X pressed	0x2E	C pressed	0x2F	V pressed
0x30	B pressed	0x31	N pressed	0x32	M pressed	0x33	, pressed
0x34	. pressed	0x35	/ pressed	0x36	right shift pressed	0x37	(keypad) * pressed
0x38	left alt pressed	0x39	space pressed	0x3A	CapsLock pressed	0x3B	F1 pressed
0x3C	F2 pressed	0x3D	F3 pressed	0x3E	F4 pressed	0x3F	F5 pressed
0x40	F6 pressed	0x41	F7 pressed	0x42	F8 pressed	0x43	F9 pressed
0x44	F10 pressed	0x45	NumberLock pressed	0x46	ScrollLock pressed	0x47	(keypad) 7 pressed
0x48	(keypad) 8 pressed	0x49	(keypad) 9 pressed	0x4A	(keypad) - pressed	0x4B	(keypad) 4 pressed
0x4C	(keypad) 5 pressed	0x4D	(keypad) 6 pressed	0x4E	(keypad) + pressed	0x4F	(keypad) 1 pressed
0x50	(keypad) 2 pressed	0x51	(keypad) 3 pressed	0x52	(keypad) 0 pressed	0x53	(keypad) . pressed
0x57	F11 pressed
0x58	F12 pressed	
//				
0x81	escape released	0x82	1 released	0x83	2 released
0x84	3 released	0x85	4 released	0x86	5 released	0x87	6 released
0x88	7 released	0x89	8 released	0x8A	9 released	0x8B	0 (zero) released
0x8C	- released	0x8D	= released	0x8E	backspace released	0x8F	tab released
0x90	Q released	0x91	W released	0x92	E released	0x93	R released
0x94	T released	0x95	Y released	0x96	U released	0x97	I released
0x98	O released	0x99	P released	0x9A	[ released	0x9B	] released
0x9C	enter released	0x9D	left control released	0x9E	A released	0x9F	S released
0xA0	D released	0xA1	F released	0xA2	G released	0xA3	H released
0xA4	J released	0xA5	K released	0xA6	L released	0xA7	 ; released
0xA8	' (single quote) released	0xA9	` (back tick) released	0xAA	left shift released	0xAB	\ released
0xAC	Z released	0xAD	X released	0xAE	C released	0xAF	V released
0xB0	B released	0xB1	N released	0xB2	M released	0xB3	, released
0xB4	. released	0xB5	/ released	0xB6	right shift released	0xB7	(keypad) * released
0xB8	left alt released	0xB9	space released	0xBA	CapsLock released	0xBB	F1 released
0xBC	F2 released	0xBD	F3 released	0xBE	F4 released	0xBF	F5 released
0xC0	F6 released	0xC1	F7 released	0xC2	F8 released	0xC3	F9 released
0xC4	F10 released	0xC5	NumberLock released	0xC6	ScrollLock released	0xC7	(keypad) 7 released
0xC8	(keypad) 8 released	0xC9	(keypad) 9 released	0xCA	(keypad) - released	0xCB	(keypad) 4 released
0xCC	(keypad) 5 released	0xCD	(keypad) 6 released	0xCE	(keypad) + released	0xCF	(keypad) 1 released
0xD0	(keypad) 2 released	0xD1	(keypad) 3 released	0xD2	(keypad) 0 released	0xD3	(keypad) . released
0xD7	F11 released
0xD8	F12 released	
//					
0xE0, 0x10	(multimedia) previous track pressed						
0xE0, 0x19	(multimedia) next track pressed				
0xE0, 0x1C	(keypad) enter pressed	0xE0, 0x1D	right control pressed				
0xE0, 0x20	(multimedia) mute pressed	0xE0, 0x21	(multimedia) calculator pressed	0xE0, 0x22	(multimedia) play pressed		
0xE0, 0x24	(multimedia) stop pressed						
0xE0, 0x2E	(multimedia) volume down pressed		
0xE0, 0x30	(multimedia) volume up pressed			0xE0, 0x32	(multimedia) WWW home pressed		
0xE0, 0x35	(keypad) / pressed				
0xE0, 0x38	right alt (or altGr) pressed						
0xE0, 0x47	home pressed
0xE0, 0x48	cursor up pressed	0xE0, 0x49	page up pressed			0xE0, 0x4B	cursor left pressed
0xE0, 0x4D	cursor right pressed			0xE0, 0x4F	end pressed
0xE0, 0x50	cursor down pressed	0xE0, 0x51	page down pressed	0xE0, 0x52	insert pressed	0xE0, 0x53	delete pressed
0xE0, 0x5B	left GUI pressed
0xE0, 0x5C	right GUI pressed	0xE0, 0x5D	"apps" pressed	0xE0, 0x5E	(ACPI) power pressed	0xE0, 0x5F	(ACPI) sleep pressed
0xE0, 0x63	(ACPI) wake pressed
0xE0, 0x65	(multimedia) WWW search pressed	0xE0, 0x66	(multimedia) WWW favorites pressed	0xE0, 0x67	(multimedia) WWW refresh pressed
0xE0, 0x68	(multimedia) WWW stop pressed	0xE0, 0x69	(multimedia) WWW forward pressed	0xE0, 0x6A	(multimedia) WWW back pressed	0xE0, 0x6B	(multimedia) my computer pressed
0xE0, 0x6C	(multimedia) email pressed	0xE0, 0x6D	(multimedia) media select pressed				
0xE0, 0x90	(multimedia) previous track released						
0xE0, 0x99	(multimedia) next track released				
0xE0, 0x9C	(keypad) enter released	0xE0, 0x9D	right control released				
0xE0, 0xA0	(multimedia) mute released	0xE0, 0xA1	(multimedia) calculator released	0xE0, 0xA2	(multimedia) play released		
0xE0, 0xA4	(multimedia) stop released						
0xE0, 0xAE	(multimedia) volume down released		
0xE0, 0xB0	(multimedia) volume up released			0xE0, 0xB2	(multimedia) WWW home released		
0xE0, 0xB5	(keypad) / released				
0xE0, 0xB8	right alt (or altGr) released						
0xE0, 0xC7	home released
0xE0, 0xC8	cursor up released	0xE0, 0xC9	page up released			0xE0, 0xCB	cursor left released
0xE0, 0xCD	cursor right released			0xE0, 0xCF	end released
0xE0, 0xD0	cursor down released	0xE0, 0xD1	page down released	0xE0, 0xD2	insert released	0xE0, 0xD3	delete released
0xE0, 0xDB	left GUI released
0xE0, 0xDC	right GUI released	0xE0, 0xDD	"apps" released	0xE0, 0xDE	(ACPI) power released	0xE0, 0xDF	(ACPI) sleep released
0xE0, 0xE3	(ACPI) wake released
0xE0, 0xE5	(multimedia) WWW search released	0xE0, 0xE6	(multimedia) WWW favorites released	0xE0, 0xE7	(multimedia) WWW refresh released
0xE0, 0xE8	(multimedia) WWW stop released	0xE0, 0xE9	(multimedia) WWW forward released	0xE0, 0xEA	(multimedia) WWW back released	0xE0, 0xEB	(multimedia) my computer released
0xE0, 0xEC	(multimedia) email released	0xE0, 0xED	(multimedia) media select released				
0xE0, 0x2A, 0xE0, 0x37	print screen pressed
0xE0, 0xB7, 0xE0, 0xAA	print screen released
0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5	pause pressed		*/

/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */
/*
 * Input event codes
 *
 *    *** IMPORTANT ***
 * This file is not only included from C-code but also from devicetree source
 * files. As such this file MUST only contain comments and defines.
 *
 * Copyright (c) 1999-2002 Vojtech Pavlik
 * Copyright (c) 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#ifndef _UAPI_INPUT_EVENT_CODES_H
#define _UAPI_INPUT_EVENT_CODES_H


#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_HELD 2


/*
 * Event types
 */

#define EV_SYN			0x00
#define EV_KEY			0x01
#define EV_REL			0x02
#define EV_ABS			0x03
#define EV_MSC			0x04
#define EV_SW			0x05
#define EV_LED			0x11
#define EV_SND			0x12
#define EV_REP			0x14
#define EV_FF			0x15
#define EV_PWR			0x16
#define EV_FF_STATUS		0x17
#define EV_MAX			0x1f
#define EV_CNT			(EV_MAX+1)

/*
 * Relative axes
 */

#define REL_X			0x00
#define REL_Y			0x01
#define REL_Z			0x02
#define REL_RX			0x03
#define REL_RY			0x04
#define REL_RZ			0x05
#define REL_HWHEEL		0x06
#define REL_DIAL		0x07
#define REL_WHEEL		0x08
#define REL_MISC		0x09


/*
 * Keys and buttons
 *
 * Most of the keys/buttons are modeled after USB HUT 1.12
 * (see http://www.usb.org/developers/hidpage).
 * Abbreviations in the comments:
 * AC - Application Control
 * AL - Application Launch Button
 * SC - System Control
 */

#define KEY_RESERVED		0
#define KEY_ESC			1
#define KEY_1			2
#define KEY_2			3
#define KEY_3			4
#define KEY_4			5
#define KEY_5			6
#define KEY_6			7
#define KEY_7			8
#define KEY_8			9
#define KEY_9			10
#define KEY_0			11
#define KEY_MINUS		12
#define KEY_EQUAL		13
#define KEY_BACKSPACE		14
#define KEY_TAB			15
#define KEY_Q			16
#define KEY_W			17
#define KEY_E			18
#define KEY_R			19
#define KEY_T			20
#define KEY_Y			21
#define KEY_U			22
#define KEY_I			23
#define KEY_O			24
#define KEY_P			25
#define KEY_LEFTBRACE		26
#define KEY_RIGHTBRACE		27
#define KEY_ENTER		28
#define KEY_LEFTCTRL		29
#define KEY_A			30
#define KEY_S			31
#define KEY_D			32
#define KEY_F			33
#define KEY_G			34
#define KEY_H			35
#define KEY_J			36
#define KEY_K			37
#define KEY_L			38
#define KEY_SEMICOLON		39
#define KEY_APOSTROPHE		40
#define KEY_GRAVE		41
#define KEY_LEFTSHIFT		42
#define KEY_BACKSLASH		43
#define KEY_Z			44
#define KEY_X			45
#define KEY_C			46
#define KEY_V			47
#define KEY_B			48
#define KEY_N			49
#define KEY_M			50
#define KEY_COMMA		51
#define KEY_DOT			52
#define KEY_SLASH		53
#define KEY_RIGHTSHIFT		54
#define KEY_KPASTERISK		55
#define KEY_LEFTALT		56
#define KEY_SPACE		57
#define KEY_CAPSLOCK		58
#define KEY_F1			59
#define KEY_F2			60
#define KEY_F3			61
#define KEY_F4			62
#define KEY_F5			63
#define KEY_F6			64
#define KEY_F7			65
#define KEY_F8			66
#define KEY_F9			67
#define KEY_F10			68
#define KEY_NUMLOCK		69
#define KEY_SCROLLLOCK		70
#define KEY_KP7			71
#define KEY_KP8			72
#define KEY_KP9			73
#define KEY_KPMINUS		74
#define KEY_KP4			75
#define KEY_KP5			76
#define KEY_KP6			77
#define KEY_KPPLUS		78
#define KEY_KP1			79
#define KEY_KP2			80
#define KEY_KP3			81
#define KEY_KP0			82
#define KEY_KPDOT		83

#define KEY_ZENKAKUHANKAKU	85
#define KEY_102ND		86
#define KEY_F11			87
#define KEY_F12			88
#define KEY_RO			89
#define KEY_KATAKANA		90
#define KEY_HIRAGANA		91
#define KEY_HENKAN		92
#define KEY_KATAKANAHIRAGANA	93
#define KEY_MUHENKAN		94
#define KEY_KPJPCOMMA		95
#define KEY_KPENTER		96
#define KEY_RIGHTCTRL		97
#define KEY_KPSLASH		98
#define KEY_SYSRQ		99
#define KEY_RIGHTALT		100
#define KEY_LINEFEED		101
#define KEY_HOME		102
#define KEY_UP			103
#define KEY_PAGEUP		104
#define KEY_LEFT		105
#define KEY_RIGHT		106
#define KEY_END			107
#define KEY_DOWN		108
#define KEY_PAGEDOWN		109
#define KEY_INSERT		110
#define KEY_DELETE		111
#define KEY_MACRO		112
#define KEY_MUTE		113
#define KEY_VOLUMEDOWN		114
#define KEY_VOLUMEUP		115
#define KEY_POWER		116	/* SC System Power Down */
#define KEY_KPEQUAL		117
#define KEY_KPPLUSMINUS		118
#define KEY_PAUSE		119
#define KEY_SCALE		120	/* AL Compiz Scale (Expose) */

//this part is different than linux
#define BTN_LEFT			121
#define BTN_RIGHT			122
#define BTN_MIDDLE			123
#define BTN_4			124
#define BTN_5			125
//some scancode helpers

class ScancodeHelpers{
    public:
        static bool is_char(uint8_t scancode){
            return (scancode >= 2 && scancode <= 53) || scancode == 57 ||(scancode >= 71 && scancode <= 83);
        }

        static char get_char(uint8_t scancode){
            //if this is useful to someone pls implement it
            return 0;
        } 

        static bool is_num(uint8_t scancode){
            return (scancode >= 2 && scancode <= 11) || (scancode >= 71 && scancode <= 73) || (scancode <= 75 && scancode >= 77) || (scancode <= 79 && scancode >= 82);
        }

        static uint8_t get_num(uint8_t scancode){
            if (scancode >= 2 && scancode <= 11) return (scancode - 1) % 10;
            if (scancode >= 71 && scancode <= 73) return (scancode - 4) % 10;
            if (scancode >= 75 && scancode <= 77) return (scancode - 1) % 10;
            if (scancode >= 79 && scancode <= 81) return (scancode - 8) % 10;
            if (scancode == 82) return 0;

            return 11;
        }
};
#endif
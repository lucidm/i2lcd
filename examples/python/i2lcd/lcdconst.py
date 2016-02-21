# -*- coding: utf-8 -*-

#Control/Data ports numbers
CPORT = 0
DPORT = 1

#Control port bits meaning
EN = 1
RW = 1 << 1
RS = 1 << 2
IRS = 1 << 3
PWR = 1 << 4
UD = 1 << 5
CCS = 1 << 6
BCS = 1 << 7

#HD4470 instructions
CLEAR_DISPLAY = 1
CURSOR_HOME = 1 << 1
ENTRY_MODE_SET = 1 << 2
DISPLAY_ONOFF = 1 << 3
CURSOR_DISPLAY_SHIFT = 1 << 4
FUNCTION_SET = 1 << 5
SET_CGRAM_ADDRESS = 1 << 6
SET_DDRAM_ADDRESS = 1 << 7

#Values of instructions above
#ENTRY MODE SET
EMS_S = 1		#1 - Shift display
EMS_ID = 1 << 1	#0 - Dec cursor, 1 - inc cursor pos
    
#DISPLAY ON OFF
DOO_B = 1		#1 - Blink ON, 0 - Blink OFF
DOO_C = 1 << 1	#1 - Cursor ON, 0 - Cursor OFF
DOO_D = 1 << 2	#1 - Display ON, 0 - Display OFF
    
    #CURSOR DISPLAY SHIFT
CDS_RL = 1 << 2	#1 - shift right, 0 - shift left
CDS_SC = 1 << 3	#1 - shift display, 0 - move cursor

#FUNCTION SET
FS_F = 1 << 2	#1 - 5x10 dots, 0 - 5x8 dots
FS_N = 1 << 3	#1 - 2 lines, 0 - 1 line
FS_DL = 1 << 4	#1 - 8 bit interface, 0 - 4 bit interface

#BUSY FLAG
BF = 1 << 7		#1 - busy, 0 - can accept instruction

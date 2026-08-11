#pragma once

// TODO | rip gw dependency
#define IMGN_KEY_UNSUPPORTED          -1
#define IMGN_KEY_UNKNOWN               0

// General keys
#define IMGN_KEY_ESCAPE                1
#define IMGN_KEY_MINUS                 2
#define IMGN_KEY_EQUAL                 3
#define IMGN_KEY_BACKSPACE             4
#define IMGN_KEY_TAB                   5
#define IMGN_KEY_LEFT_BRACKET          6
#define IMGN_KEY_RIGHT_BRACKET         7
#define IMGN_KEY_ENTER                 8
#define IMGN_KEY_LEFT_CONTROL          9
#define IMGN_KEY_RIGHT_CONTROL        10
#define IMGN_KEY_SEMICOLON            11
#define IMGN_KEY_APOSTROPHE           12
#define IMGN_KEY_GRAVE_ACCENT         13
#define IMGN_KEY_LEFT_SHIFT           14
#define IMGN_KEY_BACKSLASH            15
#define IMGN_KEY_COMMA                16
#define IMGN_KEY_PERIOD               17
#define IMGN_KEY_SLASH                18
#define IMGN_KEY_RIGHT_SHIFT          19
#define IMGN_KEY_PRINT_SCREEN         20
#define IMGN_KEY_LEFT_ALT             21
#define IMGN_KEY_RIGHT_ALT            22
#define IMGN_KEY_SPACE                23
#define IMGN_KEY_CAPS_LOCK            24
#define IMGN_KEY_NUM_LOCK             25
#define IMGN_KEY_SCROLL_LOCK          26
#define IMGN_KEY_PAUSE                27
#define IMGN_KEY_HOME                 28
#define IMGN_KEY_UP                   29
#define IMGN_KEY_PAGE_UP              30
#define IMGN_KEY_LEFT                 31
#define IMGN_KEY_RIGHT                32
#define IMGN_KEY_END                  33
#define IMGN_KEY_DOWN                 34
#define IMGN_KEY_PAGE_DOWN            35
#define IMGN_KEY_INSERT               36
#define IMGN_KEY_DELETE               37

// Letters
#define IMGN_KEY_A                    38
#define IMGN_KEY_B                    39
#define IMGN_KEY_C                    40
#define IMGN_KEY_D                    41
#define IMGN_KEY_E                    42
#define IMGN_KEY_F                    43
#define IMGN_KEY_G                    44
#define IMGN_KEY_H                    45
#define IMGN_KEY_I                    46
#define IMGN_KEY_J                    47
#define IMGN_KEY_K                    48
#define IMGN_KEY_L                    49
#define IMGN_KEY_M                    50
#define IMGN_KEY_N                    51
#define IMGN_KEY_O                    52
#define IMGN_KEY_P                    53
#define IMGN_KEY_Q                    54
#define IMGN_KEY_R                    55
#define IMGN_KEY_S                    56
#define IMGN_KEY_T                    57
#define IMGN_KEY_U                    58
#define IMGN_KEY_V                    59
#define IMGN_KEY_W                    60
#define IMGN_KEY_X                    61
#define IMGN_KEY_Y                    62
#define IMGN_KEY_Z                    63

// Number row
#define IMGN_KEY_0                    64
#define IMGN_KEY_1                    65
#define IMGN_KEY_2                    66
#define IMGN_KEY_3                    67
#define IMGN_KEY_4                    68
#define IMGN_KEY_5                    69
#define IMGN_KEY_6                    70
#define IMGN_KEY_7                    71
#define IMGN_KEY_8                    72
#define IMGN_KEY_9                    73

// Function keys supported by Gateware
#define IMGN_KEY_F1                   74
#define IMGN_KEY_F2                   75
#define IMGN_KEY_F3                   76
#define IMGN_KEY_F4                   77
#define IMGN_KEY_F5                   78
#define IMGN_KEY_F6                   79
#define IMGN_KEY_F7                   80
#define IMGN_KEY_F8                   81
#define IMGN_KEY_F9                   82
#define IMGN_KEY_F10                  83
#define IMGN_KEY_F11                  84
#define IMGN_KEY_F12                  85

// Numpad
#define IMGN_KEY_KP_ADD               86
#define IMGN_KEY_KP_SUBTRACT          87
#define IMGN_KEY_KP_MULTIPLY          88
#define IMGN_KEY_KP_DIVIDE            89
#define IMGN_KEY_KP_0                 90
#define IMGN_KEY_KP_1                 91
#define IMGN_KEY_KP_2                 92
#define IMGN_KEY_KP_3                 93
#define IMGN_KEY_KP_4                 94
#define IMGN_KEY_KP_5                 95
#define IMGN_KEY_KP_6                 96
#define IMGN_KEY_KP_7                 97
#define IMGN_KEY_KP_8                 98
#define IMGN_KEY_KP_9                 99
#define IMGN_KEY_KP_DECIMAL          100
#define IMGN_KEY_KP_ENTER            101

// Gateware calls this G_KEY_COMMAND. It is primarily meaningful on macOS.
#define IMGN_KEY_LEFT_SUPER           102

// Mouse codes supported by Gateware GInput.
#define IMGN_MOUSE_BUTTON_1           200
#define IMGN_MOUSE_BUTTON_2           201
#define IMGN_MOUSE_BUTTON_3           202
#define IMGN_MOUSE_BUTTON_LEFT        IMGN_MOUSE_BUTTON_1
#define IMGN_MOUSE_BUTTON_RIGHT       IMGN_MOUSE_BUTTON_2
#define IMGN_MOUSE_BUTTON_MIDDLE      IMGN_MOUSE_BUTTON_3

// Gateware GInput in the supplied header only exposes three mouse buttons.
#define IMGN_MOUSE_BUTTON_4           IMGN_KEY_UNSUPPORTED
#define IMGN_MOUSE_BUTTON_5           IMGN_KEY_UNSUPPORTED
#define IMGN_MOUSE_BUTTON_6           IMGN_KEY_UNSUPPORTED
#define IMGN_MOUSE_BUTTON_7           IMGN_KEY_UNSUPPORTED
#define IMGN_MOUSE_BUTTON_8           IMGN_KEY_UNSUPPORTED
#define IMGN_MOUSE_BUTTON_LAST        IMGN_MOUSE_BUTTON_3

#define IMGN_MOUSE_SCROLL_UP          203
#define IMGN_MOUSE_SCROLL_DOWN        204
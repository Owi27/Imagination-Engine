#pragma once

#define IMGN_KEY_SPACE              32
#define IMGN_KEY_APOSTROPHE         39  /* ' */
#define IMGN_KEY_COMMA              44  /* , */
#define IMGN_KEY_MINUS              45  /* - */
#define IMGN_KEY_PERIOD             46  /* . */
#define IMGN_KEY_SLASH              47  /* / */
#define IMGN_KEY_0                  48
#define IMGN_KEY_1                  49
#define IMGN_KEY_2                  50
#define IMGN_KEY_3                  51
#define IMGN_KEY_4                  52
#define IMGN_KEY_5                  53
#define IMGN_KEY_6                  54
#define IMGN_KEY_7                  55
#define IMGN_KEY_8                  56
#define IMGN_KEY_9                  57
#define IMGN_KEY_SEMICOLON          59  /* ; */
#define IMGN_KEY_EQUAL              61  /* = */
#define IMGN_KEY_A                  65
#define IMGN_KEY_B                  66
#define IMGN_KEY_C                  67
#define IMGN_KEY_D                  68
#define IMGN_KEY_E                  69
#define IMGN_KEY_F                  70
#define IMGN_KEY_G                  71
#define IMGN_KEY_H                  72
#define IMGN_KEY_I                  73
#define IMGN_KEY_J                  74
#define IMGN_KEY_K                  75
#define IMGN_KEY_L                  76
#define IMGN_KEY_M                  77
#define IMGN_KEY_N                  78
#define IMGN_KEY_O                  79
#define IMGN_KEY_P                  80
#define IMGN_KEY_Q                  81
#define IMGN_KEY_R                  82
#define IMGN_KEY_S                  83
#define IMGN_KEY_T                  84
#define IMGN_KEY_U                  85
#define IMGN_KEY_V                  86
#define IMGN_KEY_W                  87
#define IMGN_KEY_X                  88
#define IMGN_KEY_Y                  89
#define IMGN_KEY_Z                  90
#define IMGN_KEY_LEFT_BRACKET       91  /* [ */
#define IMGN_KEY_BACKSLASH          92  /* \ */
#define IMGN_KEY_RIGHT_BRACKET      93  /* ] */
#define IMGN_KEY_GRAVE_ACCENT       96  /* ` */
#define IMGN_KEY_WORLD_1            161 /* non-US #1 */
#define IMGN_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define IMGN_KEY_ESCAPE             256
#define IMGN_KEY_ENTER              257
#define IMGN_KEY_TAB                258
#define IMGN_KEY_BACKSPACE          259
#define IMGN_KEY_INSERT             260
#define IMGN_KEY_DELETE             261
#define IMGN_KEY_RIGHT              262
#define IMGN_KEY_LEFT               263
#define IMGN_KEY_DOWN               264
#define IMGN_KEY_UP                 265
#define IMGN_KEY_PAGE_UP            266
#define IMGN_KEY_PAGE_DOWN          267
#define IMGN_KEY_HOME               268
#define IMGN_KEY_END                269
#define IMGN_KEY_CAPS_LOCK          280
#define IMGN_KEY_SCROLL_LOCK        281
#define IMGN_KEY_NUM_LOCK           282
#define IMGN_KEY_PRINT_SCREEN       283
#define IMGN_KEY_PAUSE              284
#define IMGN_KEY_F1                 290
#define IMGN_KEY_F2                 291
#define IMGN_KEY_F3                 292
#define IMGN_KEY_F4                 293
#define IMGN_KEY_F5                 294
#define IMGN_KEY_F6                 295
#define IMGN_KEY_F7                 296
#define IMGN_KEY_F8                 297
#define IMGN_KEY_F9                 298
#define IMGN_KEY_F10                299
#define IMGN_KEY_F11                300
#define IMGN_KEY_F12                301
#define IMGN_KEY_F13                302
#define IMGN_KEY_F14                303
#define IMGN_KEY_F15                304
#define IMGN_KEY_F16                305
#define IMGN_KEY_F17                306
#define IMGN_KEY_F18                307
#define IMGN_KEY_F19                308
#define IMGN_KEY_F20                309
#define IMGN_KEY_F21                310
#define IMGN_KEY_F22                311
#define IMGN_KEY_F23                312
#define IMGN_KEY_F24                313
#define IMGN_KEY_F25                314
#define IMGN_KEY_KP_0               320
#define IMGN_KEY_KP_1               321
#define IMGN_KEY_KP_2               322
#define IMGN_KEY_KP_3               323
#define IMGN_KEY_KP_4               324
#define IMGN_KEY_KP_5               325
#define IMGN_KEY_KP_6               326
#define IMGN_KEY_KP_7               327
#define IMGN_KEY_KP_8               328
#define IMGN_KEY_KP_9               329
#define IMGN_KEY_KP_DECIMAL         330
#define IMGN_KEY_KP_DIVIDE          331
#define IMGN_KEY_KP_MULTIPLY        332
#define IMGN_KEY_KP_SUBTRACT        333
#define IMGN_KEY_KP_ADD             334
#define IMGN_KEY_KP_ENTER           335
#define IMGN_KEY_KP_EQUAL           336
#define IMGN_KEY_LEFT_SHIFT         340
#define IMGN_KEY_LEFT_CONTROL       341
#define IMGN_KEY_LEFT_ALT           342
#define IMGN_KEY_LEFT_SUPER         343
#define IMGN_KEY_RIGHT_SHIFT        344
#define IMGN_KEY_RIGHT_CONTROL      345
#define IMGN_KEY_RIGHT_ALT          346
#define IMGN_KEY_RIGHT_SUPER        347
#define IMGN_KEY_MENU               348

//mouse
#define IMGN_MOUSE_BUTTON_1         0
#define IMGN_MOUSE_BUTTON_2         1
#define IMGN_MOUSE_BUTTON_3         2
#define IMGN_MOUSE_BUTTON_4         3
#define IMGN_MOUSE_BUTTON_5         4
#define IMGN_MOUSE_BUTTON_6         5
#define IMGN_MOUSE_BUTTON_7         6
#define IMGN_MOUSE_BUTTON_8         7
#define IMGN_MOUSE_BUTTON_LAST      IMGN_MOUSE_BUTTON_8
#define IMGN_MOUSE_BUTTON_LEFT      IMGN_MOUSE_BUTTON_1
#define IMGN_MOUSE_BUTTON_RIGHT     IMGN_MOUSE_BUTTON_2
#define IMGN_MOUSE_BUTTON_MIDDLE    IMGN_MOUSE_BUTTON_3

#define IMGN_KEY(x) \
( \
    (x) == G_KEY_ESCAPE            ? IMGN_KEY_ESCAPE : \
    (x) == G_KEY_MINUS             ? IMGN_KEY_MINUS : \
    (x) == G_KEY_EQUALS            ? IMGN_KEY_EQUAL : \
    (x) == G_KEY_BACKSPACE         ? IMGN_KEY_BACKSPACE : \
    (x) == G_KEY_TAB               ? IMGN_KEY_TAB : \
    (x) == G_KEY_BRACKET_OPEN      ? IMGN_KEY_LEFT_BRACKET : \
    (x) == G_KEY_BRACKET_CLOSE     ? IMGN_KEY_RIGHT_BRACKET : \
    (x) == G_KEY_ENTER             ? IMGN_KEY_ENTER : \
    (x) == G_KEY_LEFTCONTROL       ? IMGN_KEY_LEFT_CONTROL : \
    (x) == G_KEY_RIGHTCONTROL      ? IMGN_KEY_RIGHT_CONTROL : \
    (x) == G_KEY_SEMICOLON         ? IMGN_KEY_SEMICOLON : \
    (x) == G_KEY_QUOTE             ? IMGN_KEY_APOSTROPHE : \
    (x) == G_KEY_TILDE             ? IMGN_KEY_GRAVE_ACCENT : \
    (x) == G_KEY_LEFTSHIFT         ? IMGN_KEY_LEFT_SHIFT : \
    (x) == G_KEY_BACKSLASH         ? IMGN_KEY_BACKSLASH : \
    (x) == G_KEY_COMMA             ? IMGN_KEY_COMMA : \
    (x) == G_KEY_PERIOD            ? IMGN_KEY_PERIOD : \
    (x) == G_KEY_FORWARDSLASH      ? IMGN_KEY_SLASH : \
    (x) == G_KEY_RIGHTSHIFT        ? IMGN_KEY_RIGHT_SHIFT : \
    (x) == G_KEY_PRINTSCREEN       ? IMGN_KEY_PRINT_SCREEN : \
    (x) == G_KEY_LEFTALT           ? IMGN_KEY_LEFT_ALT : \
    (x) == G_KEY_RIGHTALT          ? IMGN_KEY_RIGHT_ALT : \
    (x) == G_KEY_SPACE             ? IMGN_KEY_SPACE : \
    (x) == G_KEY_CAPSLOCK          ? IMGN_KEY_CAPS_LOCK : \
    (x) == G_KEY_NUMLOCK           ? IMGN_KEY_NUM_LOCK : \
    (x) == G_KEY_SCROLL_LOCK       ? IMGN_KEY_SCROLL_LOCK : \
    (x) == G_KEY_PAUSE             ? IMGN_KEY_PAUSE : \
    (x) == G_KEY_HOME              ? IMGN_KEY_HOME : \
    (x) == G_KEY_UP                ? IMGN_KEY_UP : \
    (x) == G_KEY_PAGEUP            ? IMGN_KEY_PAGE_UP : \
    (x) == G_KEY_LEFT              ? IMGN_KEY_LEFT : \
    (x) == G_KEY_RIGHT             ? IMGN_KEY_RIGHT : \
    (x) == G_KEY_END               ? IMGN_KEY_END : \
    (x) == G_KEY_DOWN              ? IMGN_KEY_DOWN : \
    (x) == G_KEY_PAGEDOWN          ? IMGN_KEY_PAGE_DOWN : \
    (x) == G_KEY_INSERT            ? IMGN_KEY_INSERT : \
    (x) == G_KEY_DELETE            ? IMGN_KEY_DELETE : \
    (x) == G_KEY_A                 ? IMGN_KEY_A : \
    (x) == G_KEY_B                 ? IMGN_KEY_B : \
    (x) == G_KEY_C                 ? IMGN_KEY_C : \
    (x) == G_KEY_D                 ? IMGN_KEY_D : \
    (x) == G_KEY_E                 ? IMGN_KEY_E : \
    (x) == G_KEY_F                 ? IMGN_KEY_F : \
    (x) == G_KEY_G                 ? IMGN_KEY_G : \
    (x) == G_KEY_H                 ? IMGN_KEY_H : \
    (x) == G_KEY_I                 ? IMGN_KEY_I : \
    (x) == G_KEY_J                 ? IMGN_KEY_J : \
    (x) == G_KEY_K                 ? IMGN_KEY_K : \
    (x) == G_KEY_L                 ? IMGN_KEY_L : \
    (x) == G_KEY_M                 ? IMGN_KEY_M : \
    (x) == G_KEY_N                 ? IMGN_KEY_N : \
    (x) == G_KEY_O                 ? IMGN_KEY_O : \
    (x) == G_KEY_P                 ? IMGN_KEY_P : \
    (x) == G_KEY_Q                 ? IMGN_KEY_Q : \
    (x) == G_KEY_R                 ? IMGN_KEY_R : \
    (x) == G_KEY_S                 ? IMGN_KEY_S : \
    (x) == G_KEY_T                 ? IMGN_KEY_T : \
    (x) == G_KEY_U                 ? IMGN_KEY_U : \
    (x) == G_KEY_V                 ? IMGN_KEY_V : \
    (x) == G_KEY_W                 ? IMGN_KEY_W : \
    (x) == G_KEY_X                 ? IMGN_KEY_X : \
    (x) == G_KEY_Y                 ? IMGN_KEY_Y : \
    (x) == G_KEY_Z                 ? IMGN_KEY_Z : \
    (x) == G_KEY_0                 ? IMGN_KEY_0 : \
    (x) == G_KEY_1                 ? IMGN_KEY_1 : \
    (x) == G_KEY_2                 ? IMGN_KEY_2 : \
    (x) == G_KEY_3                 ? IMGN_KEY_3 : \
    (x) == G_KEY_4                 ? IMGN_KEY_4 : \
    (x) == G_KEY_5                 ? IMGN_KEY_5 : \
    (x) == G_KEY_6                 ? IMGN_KEY_6 : \
    (x) == G_KEY_7                 ? IMGN_KEY_7 : \
    (x) == G_KEY_8                 ? IMGN_KEY_8 : \
    (x) == G_KEY_9                 ? IMGN_KEY_9 : \
    (x) == G_KEY_F1                ? IMGN_KEY_F1 : \
    (x) == G_KEY_F2                ? IMGN_KEY_F2 : \
    (x) == G_KEY_F3                ? IMGN_KEY_F3 : \
    (x) == G_KEY_F4                ? IMGN_KEY_F4 : \
    (x) == G_KEY_F5                ? IMGN_KEY_F5 : \
    (x) == G_KEY_F6                ? IMGN_KEY_F6 : \
    (x) == G_KEY_F7                ? IMGN_KEY_F7 : \
    (x) == G_KEY_F8                ? IMGN_KEY_F8 : \
    (x) == G_KEY_F9                ? IMGN_KEY_F9 : \
    (x) == G_KEY_F10               ? IMGN_KEY_F10 : \
    (x) == G_KEY_F11               ? IMGN_KEY_F11 : \
    (x) == G_KEY_F12               ? IMGN_KEY_F12 : \
    (x) == G_KEY_NUMPAD_ADD        ? IMGN_KEY_KP_ADD : \
    (x) == G_KEY_NUMPAD_SUBTRACT   ? IMGN_KEY_KP_SUBTRACT : \
    (x) == G_KEY_NUMPAD_MULTIPLY   ? IMGN_KEY_KP_MULTIPLY : \
    (x) == G_KEY_NUMPAD_DIVIDE     ? IMGN_KEY_KP_DIVIDE : \
    (x) == G_KEY_NUMPAD_0          ? IMGN_KEY_KP_0 : \
    (x) == G_KEY_NUMPAD_1          ? IMGN_KEY_KP_1 : \
    (x) == G_KEY_NUMPAD_2          ? IMGN_KEY_KP_2 : \
    (x) == G_KEY_NUMPAD_3          ? IMGN_KEY_KP_3 : \
    (x) == G_KEY_NUMPAD_4          ? IMGN_KEY_KP_4 : \
    (x) == G_KEY_NUMPAD_5          ? IMGN_KEY_KP_5 : \
    (x) == G_KEY_NUMPAD_6          ? IMGN_KEY_KP_6 : \
    (x) == G_KEY_NUMPAD_7          ? IMGN_KEY_KP_7 : \
    (x) == G_KEY_NUMPAD_8          ? IMGN_KEY_KP_8 : \
    (x) == G_KEY_NUMPAD_9          ? IMGN_KEY_KP_9 : \
    (x) == G_KEY_NUMPAD_PERIOD     ? IMGN_KEY_KP_DECIMAL : \
    (x) == G_KEY_NUMPAD_ENTER      ? IMGN_KEY_KP_ENTER : \
    (x) == G_KEY_COMMAND           ? IMGN_KEY_LEFT_SUPER : \
    -1 \
)

#define IMGN_MOUSE(x) \
( \
    (x) == G_BUTTON_LEFT        ? IMGN_MOUSE_BUTTON_LEFT : \
    (x) == G_BUTTON_RIGHT       ? IMGN_MOUSE_BUTTON_RIGHT : \
    (x) == G_BUTTON_MIDDLE      ? IMGN_MOUSE_BUTTON_MIDDLE : \
    -1 \
)
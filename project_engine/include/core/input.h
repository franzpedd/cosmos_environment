#pragma once

namespace Cosmos::Input
{
	typedef enum Buttoncode : unsigned short
	{
        BUTTON_LEFT = 1,
        BUTTON_MIDDLE = 2,
        BUTTON_RIGHT = 3,
        BUTTON_X1 = 4,
        BUTTON_X2 = 5
	} Buttoncode;

    typedef enum Keycode : unsigned int
    {
        KEYCODE_UNKNOWN = 0,
        KEYCODE_A = 4,
        KEYCODE_B = 5,
        KEYCODE_C = 6,
        KEYCODE_D = 7,
        KEYCODE_E = 8,
        KEYCODE_F = 9,
        KEYCODE_G = 10,
        KEYCODE_H = 11,
        KEYCODE_I = 12,
        KEYCODE_J = 13,
        KEYCODE_K = 14,
        KEYCODE_L = 15,
        KEYCODE_M = 16,
        KEYCODE_N = 17,
        KEYCODE_O = 18,
        KEYCODE_P = 19,
        KEYCODE_Q = 20,
        KEYCODE_R = 21,
        KEYCODE_S = 22,
        KEYCODE_T = 23,
        KEYCODE_U = 24,
        KEYCODE_V = 25,
        KEYCODE_W = 26,
        KEYCODE_X = 27,
        KEYCODE_Y = 28,
        KEYCODE_Z = 29,
        KEYCODE_1 = 30,
        KEYCODE_2 = 31,
        KEYCODE_3 = 32,
        KEYCODE_4 = 33,
        KEYCODE_5 = 34,
        KEYCODE_6 = 35,
        KEYCODE_7 = 36,
        KEYCODE_8 = 37,
        KEYCODE_9 = 38,
        KEYCODE_0 = 39,
        KEYCODE_RETURN = 40,
        KEYCODE_ESCAPE = 41,
        KEYCODE_BACKSPACE = 42,
        KEYCODE_TAB = 43,
        KEYCODE_SPACE = 44,
        KEYCODE_MINUS = 45,
        KEYCODE_EQUALS = 46,
        KEYCODE_LEFTBRACKET = 47,
        KEYCODE_RIGHTBRACKET = 48,
        KEYCODE_BACKSLASH = 49,
        KEYCODE_NONUSHASH = 50,
        KEYCODE_SEMICOLON = 51,
        KEYCODE_APOSTROPHE = 52,
        KEYCODE_GRAVE = 53,
        KEYCODE_COMMA = 54,
        KEYCODE_PERIOD = 55,
        KEYCODE_SLASH = 56,
        KEYCODE_CAPSLOCK = 57,
        KEYCODE_F1 = 58,
        KEYCODE_F2 = 59,
        KEYCODE_F3 = 60,
        KEYCODE_F4 = 61,
        KEYCODE_F5 = 62,
        KEYCODE_F6 = 63,
        KEYCODE_F7 = 64,
        KEYCODE_F8 = 65,
        KEYCODE_F9 = 66,
        KEYCODE_F10 = 67,
        KEYCODE_F11 = 68,
        KEYCODE_F12 = 69,
        KEYCODE_PRINTSCREEN = 70,
        KEYCODE_SCROLLLOCK = 71,
        KEYCODE_PAUSE = 72,
        KEYCODE_INSERT = 73,
        KEYCODE_HOME = 74,
        KEYCODE_PAGEUP = 75,
        KEYCODE_DELETE = 76,
        KEYCODE_END = 77,
        KEYCODE_PAGEDOWN = 78,
        KEYCODE_RIGHT = 79,
        KEYCODE_LEFT = 80,
        KEYCODE_DOWN = 81,
        KEYCODE_UP = 82,
        KEYCODE_NUMLOCKCLEAR = 83,
        KEYCODE_KP_DIVIDE = 84,
        KEYCODE_KP_MULTIPLY = 85,
        KEYCODE_KP_MINUS = 86,
        KEYCODE_KP_PLUS = 87,
        KEYCODE_KP_ENTER = 88,
        KEYCODE_KP_1 = 89,
        KEYCODE_KP_2 = 90,
        KEYCODE_KP_3 = 91,
        KEYCODE_KP_4 = 92,
        KEYCODE_KP_5 = 93,
        KEYCODE_KP_6 = 94,
        KEYCODE_KP_7 = 95,
        KEYCODE_KP_8 = 96,
        KEYCODE_KP_9 = 97,
        KEYCODE_KP_0 = 98,
        KEYCODE_KP_PERIOD = 99,
        KEYCODE_NONUSBACKSLASH = 100,
        KEYCODE_APPLICATION = 101,
        KEYCODE_POWER = 102,
        KEYCODE_KP_EQUALS = 103,
        KEYCODE_F13 = 104,
        KEYCODE_F14 = 105,
        KEYCODE_F15 = 106,
        KEYCODE_F16 = 107,
        KEYCODE_F17 = 108,
        KEYCODE_F18 = 109,
        KEYCODE_F19 = 110,
        KEYCODE_F20 = 111,
        KEYCODE_F21 = 112,
        KEYCODE_F22 = 113,
        KEYCODE_F23 = 114,
        KEYCODE_F24 = 115,
        KEYCODE_EXECUTE = 116,
        KEYCODE_HELP = 117,
        KEYCODE_MENU = 118,
        KEYCODE_SELECT = 119,
        KEYCODE_STOP = 120,
        KEYCODE_AGAIN = 121,
        KEYCODE_UNDO = 122,
        KEYCODE_CUT = 123,
        KEYCODE_COPY = 124,
        KEYCODE_PASTE = 125,
        KEYCODE_FIND = 126,
        KEYCODE_MUTE = 127,
        KEYCODE_VOLUMEUP = 128,
        KEYCODE_VOLUMEDOWN = 129,
        KEYCODE_KP_COMMA = 133,
        KEYCODE_KP_EQUALSAS400 = 134,
        KEYCODE_INTERNATIONAL1 = 135,
        KEYCODE_INTERNATIONAL2 = 136,
        KEYCODE_INTERNATIONAL3 = 137,
        KEYCODE_INTERNATIONAL4 = 138,
        KEYCODE_INTERNATIONAL5 = 139,
        KEYCODE_INTERNATIONAL6 = 140,
        KEYCODE_INTERNATIONAL7 = 141,
        KEYCODE_INTERNATIONAL8 = 142,
        KEYCODE_INTERNATIONAL9 = 143,
        KEYCODE_LANG1 = 144,
        KEYCODE_LANG2 = 145,
        KEYCODE_LANG3 = 146,
        KEYCODE_LANG4 = 147,
        KEYCODE_LANG5 = 148,
        KEYCODE_LANG6 = 149,
        KEYCODE_LANG7 = 150,
        KEYCODE_LANG8 = 151,
        KEYCODE_LANG9 = 152,

        KEYCODE_ALTERASE = 153,
        KEYCODE_SYSREQ = 154,
        KEYCODE_CANCEL = 155,
        KEYCODE_CLEAR = 156,
        KEYCODE_PRIOR = 157,
        KEYCODE_RETURN2 = 158,
        KEYCODE_SEPARATOR = 159,
        KEYCODE_OUT = 160,
        KEYCODE_OPER = 161,
        KEYCODE_CLEARAGAIN = 162,
        KEYCODE_CRSEL = 163,
        KEYCODE_EXSEL = 164,
        KEYCODE_KP_00 = 176,
        KEYCODE_KP_000 = 177,
        KEYCODE_THOUSANDSSEPARATOR = 178,
        KEYCODE_DECIMALSEPARATOR = 179,
        KEYCODE_CURRENCYUNIT = 180,
        KEYCODE_CURRENCYSUBUNIT = 181,
        KEYCODE_KP_LEFTPAREN = 182,
        KEYCODE_KP_RIGHTPAREN = 183,
        KEYCODE_KP_LEFTBRACE = 184,
        KEYCODE_KP_RIGHTBRACE = 185,
        KEYCODE_KP_TAB = 186,
        KEYCODE_KP_BACKSPACE = 187,
        KEYCODE_KP_A = 188,
        KEYCODE_KP_B = 189,
        KEYCODE_KP_C = 190,
        KEYCODE_KP_D = 191,
        KEYCODE_KP_E = 192,
        KEYCODE_KP_F = 193,
        KEYCODE_KP_XOR = 194,
        KEYCODE_KP_POWER = 195,
        KEYCODE_KP_PERCENT = 196,
        KEYCODE_KP_LESS = 197,
        KEYCODE_KP_GREATER = 198,
        KEYCODE_KP_AMPERSAND = 199,
        KEYCODE_KP_DBLAMPERSAND = 200,
        KEYCODE_KP_VERTICALBAR = 201,
        KEYCODE_KP_DBLVERTICALBAR = 202,
        KEYCODE_KP_COLON = 203,
        KEYCODE_KP_HASH = 204,
        KEYCODE_KP_SPACE = 205,
        KEYCODE_KP_AT = 206,
        KEYCODE_KP_EXCLAM = 207,
        KEYCODE_KP_MEMSTORE = 208,
        KEYCODE_KP_MEMRECALL = 209,
        KEYCODE_KP_MEMCLEAR = 210,
        KEYCODE_KP_MEMADD = 211,
        KEYCODE_KP_MEMSUBTRACT = 212,
        KEYCODE_KP_MEMMULTIPLY = 213,
        KEYCODE_KP_MEMDIVIDE = 214,
        KEYCODE_KP_PLUSMINUS = 215,
        KEYCODE_KP_CLEAR = 216,
        KEYCODE_KP_CLEARENTRY = 217,
        KEYCODE_KP_BINARY = 218,
        KEYCODE_KP_OCTAL = 219,
        KEYCODE_KP_DECIMAL = 220,
        KEYCODE_KP_HEXADECIMAL = 221,
        KEYCODE_LCTRL = 224,
        KEYCODE_LSHIFT = 225,
        KEYCODE_LALT = 226,
        KEYCODE_LGUI = 227,
        KEYCODE_RCTRL = 228,
        KEYCODE_RSHIFT = 229,
        KEYCODE_RALT = 230,
        KEYCODE_RGUI = 231,
        KEYCODE_MODE = 257,
    } Keycode;

    typedef enum Keymod : unsigned int
    {
        KEYMOD_NONE = 0x0000u,
        KEYMOD_LSHIFT = 0x0001u,
        KEYMOD_RSHIFT = 0x0002u,
        KEYMOD_LEVEL5 = 0x0004u,
        KEYMOD_LCTRL = 0x0040u,
        KEYMOD_RCTRL = 0x0080u,
        KEYMOD_LALT = 0x0100u,
        KEYMOD_RALT = 0x0200u,
        KEYMOD_LGUI = 0x0400u,
        KEYMOD_RGUI = 0x0800u,
        KEYMOD_NUM = 0x1000u,
        KEYMOD_CAPS = 0x2000u,
        KEYMOD_MODE = 0x4000u,
        KEYMOD_SCROLL = 0x8000u,
        KEYMOD_CTRL = (KEYMOD_LCTRL | KEYMOD_RCTRL),
        KEYMOD_SHIFT = (KEYMOD_LSHIFT | KEYMOD_RSHIFT),
        KEYMOD_ALT = (KEYMOD_LALT | KEYMOD_RALT),
        KEYMOD_GUI = (KEYMOD_LGUI | KEYMOD_RGUI)
    } Keymod;
}
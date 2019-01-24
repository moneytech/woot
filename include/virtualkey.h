#ifndef VIRTUALKEY_H
#define VIRTUALKEY_H

#include <types.h>

#define VK_NULL           0x00
#define VK_LEFTBUTTON     0x01
#define VK_RIGHTBUTTON    0x02
#define VK_CANCEL         0x03
#define VK_MIDDLEBUTTON   0x04
#define VK_EXTRABUTTON    0x05
#define VK_EXTRABUTTON2   0x06
#define VK_UNDEFINED07    0x07
#define VK_BACK           0x08
#define VK_TAB            0x09
#define VK_UNDEFINED0A    0x0A
#define VK_UNDEFINED0B    0x0B
#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D
#define VK_UNDEFINED0E    0x0E
#define VK_UNDEFINED0F    0x0F

#define VK_UDNEFINED10    0x10
#define VK_UNDEFINED11    0x11
#define VK_UNDEFINED12    0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14
#define VK_KANA           0x15
#define VK_UNDEFINED16    0x16
#define VK_JUNJA          0x17
#define VK_FINAL          0x18
#define VK_KANJI          0x19
#define VK_UNDEFINED1A    0x1A
#define VK_ESCAPE         0x1B
#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

#define VK_KEY0           0x30
#define VK_KEY1           0x31
#define VK_KEY2           0x32
#define VK_KEY3           0x33
#define VK_KEY4           0x34
#define VK_KEY5           0x35
#define VK_KEY6           0x36
#define VK_KEY7           0x37
#define VK_KEY8           0x38
#define VK_KEY9           0x39
#define VK_UNDEFINED3A    0x3A
#define VK_UNDEFINED3B    0x3B
#define VK_UNDEFINED3C    0x3C
#define VK_UNDEFINED3D    0x3D
#define VK_UNDEFINED3E    0x3E
#define VK_UNDEFINED3F    0x3F

#define VK_UNDEFINED40    0x40
#define VK_KEYA           0x41
#define VK_KEYB           0x42
#define VK_KEYC           0x43
#define VK_KEYD           0x44
#define VK_KEYE           0x45
#define VK_KEYF           0x46
#define VK_KEYG           0x47
#define VK_KEYH           0x48
#define VK_KEYI           0x49
#define VK_KEYJ           0x4A
#define VK_KEYK           0x4B
#define VK_KEYL           0x4C
#define VK_KEYM           0x4D
#define VK_KEYN           0x4E
#define VK_KEYO           0x4F

#define VK_KEYP           0x50
#define VK_KEYQ           0x51
#define VK_KEYR           0x52
#define VK_KEYS           0x53
#define VK_KEYT           0x54
#define VK_KEYU           0x55
#define VK_KEYV           0x56
#define VK_KEYW           0x57
#define VK_KEYX           0x58
#define VK_KEYY           0x59
#define VK_KEYZ           0x5A
#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D
#define VK_UNDEFINED5E    0x5E
#define VK_SLEEP          0x5F

#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F

#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F

#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87
#define VK_UNDEFINED88    0x88
#define VK_UNDEFINED89    0x89
#define VK_UNDEFINED8A    0x8A
#define VK_UNDEFINED8B    0x8B
#define VK_UNDEFINED8C    0x8C
#define VK_UNDEFINED8D    0x8D
#define VK_UNDEFINED8E    0x8E
#define VK_UNDEFINED8F    0x8F

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91
#define VK_OEMFJJISHO     0x92
#define VK_OEMFJMASSHOU   0x93
#define VK_OEMFJTOUROKU   0x94
#define VK_OEMFJLOYA      0x95
#define VK_OEMFJROYA      0x96
#define VK_UNDEFINED97    0x97
#define VK_UNDEFINED98    0x98
#define VK_UNDEFINED99    0x99
#define VK_UNDEFINED9A    0x9A
#define VK_UNDEFINED9B    0x9B
#define VK_UNDEFINED9C    0x9C
#define VK_UNDEFINED9D    0x9D
#define VK_UNDEFINED9E    0x9E
#define VK_UNDEFINED9F    0x9F

#define VK_LSHIFT           0xA0
#define VK_RSHIFT           0xA1
#define VK_LCONTROL         0xA2
#define VK_RCONTROL         0xA3
#define VK_LMENU            0xA4
#define VK_RMENU            0xA5
#define VK_BROWSERBACK      0xA6
#define VK_BROWSERFORWARD   0xA7
#define VK_BROWSERREFRESH   0xA8
#define VK_BROWSERSTOP      0xA9
#define VK_BROWSERSEARCH    0xAA
#define VK_BROWSERFAVORITES 0xAB
#define VK_BROWSERHOME      0xAC
#define VK_VOLUMEMUTE       0xAD
#define VK_VOLUMEDOWN       0xAE
#define VK_VOLUMEUP         0xAF

#define VK_MEDIANEXTTRACK    0xB0
#define VK_MEDIAPREVTRACK    0xB1
#define VK_MEDIASTOP         0xB2
#define VK_MEDIAPLAYPAUSE    0xB3
#define VK_LAUNCHMAIL        0xB4
#define VK_LAUNCHMEDIASELECT 0xB5
#define VK_LAUNCHAPP1        0xB6
#define VK_LAUNCHAPP2        0xB7
#define VK_UNDEFINEDB8       0xB8
#define VK_UNDEFINEDB9       0xB9
#define VK_OEM1              0xBA
#define VK_OEMPLUS           0xBB
#define VK_OEMCOMMA          0xBC
#define VK_OEMMINUS          0xBD
#define VK_OEMPERIOD         0xBE
#define VK_OEM2              0xBF

#define VK_OEM3              0xC0
#define VK_ABNTC1            0xC1
#define VK_ABNTC2            0xC2
#define VK_UNDEFINEDC3       0xC3
#define VK_UNDEFINEDC4       0xC4
#define VK_UNDEFINEDC5       0xC5
#define VK_UNDEFINEDC6       0xC6
#define VK_UNDEFINEDC7       0xC7
#define VK_UNDEFINEDC8       0xC8
#define VK_UNDEFINEDC9       0xC9
#define VK_UNDEFINEDCA       0xCA
#define VK_UNDEFINEDCB       0xCB
#define VK_UNDEFINEDCC       0xCC
#define VK_UNDEFINEDCD       0xCD
#define VK_UNDEFINEDCE       0xCE
#define VK_UNDEFINEDCF       0xCF

#define VK_UNDEFINEDD0       0xD0
#define VK_UNDEFINEDD1       0xD1
#define VK_UNDEFINEDD2       0xD2
#define VK_UNDEFINEDD3       0xD3
#define VK_UNDEFINEDD4       0xD4
#define VK_UNDEFINEDD5       0xD5
#define VK_UNDEFINEDD6       0xD6
#define VK_UNDEFINEDD7       0xD7
#define VK_UNDEFINEDD8       0xD8
#define VK_UNDEFINEDD9       0xD9
#define VK_UNDEFINEDDA       0xDA
#define VK_OEM4              0xDB
#define VK_OEM5              0xDC
#define VK_OEM6              0xDD
#define VK_OEM7              0xDE
#define VK_OEM8              0xDF

#define VK_UNDEFINEDE0       0xE0
#define VK_OEMAX             0xE1
#define VK_OEM102            0xE2
#define VK_ICOHELP           0xE3
#define VK_ICO00             0xE4
#define VK_PROCESSKEY        0xE5
#define VK_ICOCLEAR          0xE6
#define VK_PACKET            0xE7
#define VK_UNDEFINEDE8       0xE8
#define VK_OEMRESET          0xE9
#define VK_OEMJUMP           0xEA
#define VK_OEMPA1            0xEB
#define VK_OEMPA2            0xEC
#define VK_OEMPA3            0xED
#define VK_OEMWSCTRL         0xEE
#define VK_OEMCUSEL          0xEF

#define VK_OEMATTN           0xF0
#define VK_OEMFINISH         0xF1
#define VK_OEMCOPY           0xF2
#define VK_OEMAUTO           0xF3
#define VK_OEMENLW           0xF4
#define VK_OEMBACKTAB        0xF5
#define VK_ATTN              0xF6
#define VK_CRSEL             0xF7
#define VK_EXSEL             0xF8
#define VK_EREOF             0xF9
#define VK_PLAY              0xFA
#define VK_ZOOM              0xFB
#define VK_NONAME            0xFC
#define VK_PA1               0xFD
#define VK_OEMCLEAR          0xFE
#define VK_NONE              0xFF

#ifdef __cplusplus

enum class VirtualKey : uint8_t
{
    Null = 0x00, LeftButton, RightButton, Cancel, MiddleButton, ExtraButton, ExtraButton2, Undefined07,
    Back, Tab, Undefined0A, Undefined0B, Clear, Return, Undefined0E, Undefined0F,

    Undefined10, Undefined11, Undefined12, Pause, Capital, Kana, Undefined16, Junja,
    Final, Kanji, Undefined1A, Escape, Convert, NonConvert, Accept, ModeChange,

    Space, Prior, Next, End, Home, Left, Up, Right,
    Down, Select, Print, Execute, Snapshot, Insert, Delete, Help,

    Key0, Key1, Key2, Key3, Key4, Key5, Key6, Key7,
    Key8, Key9, Undefined3A, Undefined3B, Undefined3C, Undefined3D, Undefined3E, Undefined3F,

    Undefined40, KeyA, KeyB, KeyC, KeyD, KeyE, KeyF, KeyG,
    KeyH, KeyI, KeyJ, KeyK, KeyL, KeyM, KeyN, KeyO,

    KeyP, KeyQ, KeyR, KeyS, KeyT, KeyU, KeyV, KeyW,
    KeyX, KeyY, KeyZ, LWin, RWin, Apps, Undefined5E, Sleep,

    NumPad0, NumPad1, NumPad2, NumPad3, NumPad4, NumPad5, NumPad6, NumPad7,
    NumPad8, NumPad9, Multiply, Add, Separator, Subtract, Decimal, Divide,

    F1, F2, F3, F4, F5, F6, F7, F8,
    F9, F10, F11, F12, F13, F14, F15, F16,

    F17, F18, F19, F20, F21, F22, F23, F24,
    Undefined88, Undefined89, Undefined8A, Undefined8B, Undefined8C, Undefined8D, Undefined8E, Undefined8F,

    NumLock, Scroll, OEMFJJisho, OEMFJMasshou, OEMFJTouroku, OEMFJLoya, OEMFJRoya, Undefined97,
    Undefined98, Undefined99, Undefined9A, Undefined9B, Undefined9C, Undefined9D, Undefined9E, Undefined9F,

    LShift, RShift, LControl, RControl, LMenu, RMenu, BrowserBack, BrowserForward,
    BrowserRefresh, BrowserStop, BrowserSearch, BrowserFavorites, BrowserHome, VolumeMute, VolumeDown, VolumeUp,

    MediaNextTrack, MediaPrevTrack, MediaStop, MediaPlayPause, LaunchMail, LaunchMediaSelect, LaunchApp1, LaunchApp2,
    UndefinedB8, UndefinedB9, OEM1, OEMPlus, OEMComma, OEMMinus, OEMPeriod, OEM2,

    OEM3, ABNTC1, ABNTC2, UndefinedC3, UndefinedC4, UndefinedC5, UndefinedC6, UndefinedC7,
    UndefinedC8, UndefinedC9, UndefinedCA, UndefinedCB, UndefinedCC, UndefinedCD, UndefinedCE, UndefinedCF,

    UndefinedD0, UndefinedD1, UndefinedD2, UndefinedD3, UndefinedD4, UndefinedD5, UndefinedD6, UndefinedD7,
    UndefinedD8, UndefinedD9, UndefinedDA, OEM4, OEM5, OEM6, OEM7, OEM8,

    UndefinedE0, OEMAX, OEM102, ICOHelp, ICO00, ProcessKey, ICOClear, Packet,
    UndefinedE8, OEMReset, OEMJump, OEMPA1, OEMPA2, OEMPA3, OEMWsCtrl, OEMCuSel,

    OEMAttn, OEMFinish, OEMCopy, OEMAuto, OEMEnLw, OEMBackTab, Attn, CrSel,
    ExSel, ErEOF, Play, Zoom, NoName, PA1, OEMClear, None
};

#endif // __cplusplus

#endif // VIRTUALKEY_H

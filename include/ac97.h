#ifndef AC97_H
#define AC97_H

#include <audiodevice.h>
#include <types.h>

class AC97
{
    const int regMap[39] =
    {
        0x02, 0x02, 0x02,   // master volume
        0x04, 0x04, 0x04,   // aux out volume
        0x06, 0x06,         // mono volume
        0x08, 0x08,         // master tone
        0x0A,               // pc beep volume
        0x0C, 0x0C,         // phone volume
        0x0E, 0x0E, 0x0E,   // mic volume
        0x10, 0x10, 0x10,   // line in volume
        0x12, 0x12, 0x12,   // cd volume
        0x14, 0x14, 0x14,   // video volume
        0x16, 0x16, 0x16,   // aux in volume
        0x18, 0x18, 0x18,   // pcm out volume
        0x1A, 0x1A,         // record select
        0x1C, 0x1C, 0x1C,   // record gain
        0x1E, 0x1E,         // record gain mic
        0x00                // reset
    };
protected:
    AudioDevice::MixerSetting mixerSettings[39] =
    {
        { 0, 100, "Master Volume (Left)" },     // 0
        { 0, 100, "Master Volume (Right)" },    // 1
        { 0, 1, "Master Mute" },                // 2

        { 0, 100, "AUX Out Volume (Left)" },    // 3
        { 0, 100, "AUX Out Volume (Right)" },   // 4
        { 0, 1, "AUX Out Mute" },               // 5

        { 0, 100, "Mono Volume" },              // 6
        { 0, 1, "Mono Mute" },                  // 7

        { 0, 100, "Master Tone (Bass)" },       // 8
        { 0, 100, "Master Tone (Treble)" },     // 9

        { 0, 100, "PC Beep Volume" },           // 10

        { 0, 100, "Phone Volume" },             // 11
        { 0, 1, "Phone Mute" },                 // 12

        { 0, 100, "Mic Volume" },               // 13
        { 0, 1, "Mic 20dB Boost" },             // 14
        { 0, 1, "Mic Mute"},                    // 15

        { 0, 100, "Line In Volume (Left)" },    // 16
        { 0, 100, "Line In Volume (Right)" },   // 17
        { 0, 1, "Line In Mute" },               // 18

        { 0, 100, "CD Volume (Left)" },         // 19
        { 0, 100, "CD Volume (Right)" },        // 20
        { 0, 1, "CD Mute" },                    // 21

        { 0, 100, "Video Volume (Left)" },      // 22
        { 0, 100, "Video Volume (Right)" },     // 23
        { 0, 1, "Video Mute" },                 // 24

        { 0, 100, "AUX Volume (Left)" },        // 25
        { 0, 100, "AUX Volume (Right)" },       // 26
        { 0, 1, "AUX Mute" },                   // 27

        { 0, 100, "PCM Out Volume (Left)" },    // 28
        { 0, 100, "PCM Out Volume (Right)" },   // 29
        { 0, 1, "PCM Out Mute" },               // 30

        { 0, 7, "Record Select (Left)" },       // 31
        { 0, 7, "Record Select (Right)" },      // 32

        { 0, 100, "Record Gain (Left)" },       // 33
        { 0, 100, "Record Gain (Right)" },      // 34
        { 0, 1, "Record Gain Mute" },           // 35

        { 0, 100, "Record Gain Mic" },          // 36
        { 0, 100, "Record Gain Mic Mute" },     // 37

        { 0, 0, nullptr }                       // 38
    };
public:
    AC97();
    virtual int RegisterWrite(uint8_t reg, uint16_t val);
    virtual int RegisterRead(uint8_t reg);
    int ResetCODEC();
    int SetMixerSetting(int setting, int value);
    int GetMixerSetting(int setting);
    ~AC97();
};

#endif // AC97_H

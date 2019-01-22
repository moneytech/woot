#ifndef AC97_H
#define AC97_H

#include <audiodevice.h>
#include <types.h>

class AC97
{
protected:
    AudioDevice::MixerSetting mixerSettings[64] =
    {
        { 0, 100, "Master Volume (Left)" },     // 0
        { 0, 100, "Master Volume (Right)" },    // 1
        { 0, 1, "Master Mute" },                // 2

        { 0, 100, "AUX Out Volume (Left)" },    // 3
        { 0, 100, "AUX Out Volume (Right)" },   // 4
        { 0, 100, "AUX Out Mute" },             // 5

        { 0, 100, "Mono Volume" },              // 6

        { 0, 100, "Master Tone (Bass)" },       // 7
        { 0, 100, "Master Tone (Treble)" },     // 8

    };
public:
    AC97();
    ~AC97();
};

#endif // AC97_H

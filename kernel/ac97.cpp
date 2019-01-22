#include <ac97.h>
#include <errno.h>

static int mapValue(int imin, int imax, int omin, int omax, int val)
{
    return (float)(val - imin) / (imax - imin) * (omax - omin) + omin;
}

AC97::AC97()
{
}

int AC97::RegisterWrite(uint8_t reg, uint16_t val)
{
    return -ENOSYS;
}

int AC97::RegisterRead(uint8_t reg)
{
    return -ENOSYS;
}

int AC97::ResetCODEC()
{
    return RegisterWrite(0, 0);
}

int AC97::SetMixerSetting(int setting, int value)
{
    return -EINVAL;
}

int AC97::GetMixerSetting(int setting)
{
    if(setting < 0 || setting > 52)
        return -EINVAL;
    AudioDevice::MixerSetting *set = mixerSettings + setting;
    int regVal = RegisterRead(regMap[setting]);
    if(regVal < 0) return regVal;
    switch(setting)
    {
    case 0: // master left
        return mapValue(63, 0, set->MinValue, set->MaxValue, (regVal >> 8) & 63);
    case 1: // master right
        return mapValue(63, 0, set->MinValue, set->MaxValue, regVal & 63);
    case 2: // master mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 3: // aux left
        return mapValue(63, 0, set->MinValue, set->MaxValue, (regVal >> 8) & 63);
    case 4: // aux right
        return mapValue(63, 0, set->MinValue, set->MaxValue, regVal & 63);
    case 5: // aux mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 6: // mono
        return mapValue(63, 0, set->MinValue, set->MaxValue, regVal & 63);
    case 7: // mono mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 8: // master bass
        return mapValue(15, 0, set->MinValue, set->MaxValue, (regVal >> 8) & 15);
    case 9: // master treble
        return mapValue(15, 0, set->MinValue, set->MaxValue, regVal & 15);

    case 10: // beep volume
        return mapValue(15, 0, set->MinValue, set->MaxValue, (regVal >> 1) & 15);

    case 11: // phone volume
        return mapValue(31, 0, set->MinValue, set->MaxValue, regVal & 31);
    case 12: // phone mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 13: // mic volume
        return mapValue(31, 0, set->MinValue, set->MaxValue, regVal & 31);
    case 14: // mic boost
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 6) & 1);
    case 15: // mic mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 16: // line left
        return mapValue(31, 0, set->MinValue, set->MaxValue, (regVal >> 8) & 31);
    case 17: // line right
        return mapValue(31, 0, set->MinValue, set->MaxValue, regVal & 31);
    case 18: // line mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 19: // cd left
        return mapValue(31, 0, set->MinValue, set->MaxValue, (regVal >> 8) & 31);
    case 20: // cd right
        return mapValue(31, 0, set->MinValue, set->MaxValue, regVal & 31);
    case 21: // cd mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 22: // video left
        return mapValue(31, 0, set->MinValue, set->MaxValue, (regVal >> 8) & 31);
    case 23: // video right
        return mapValue(31, 0, set->MinValue, set->MaxValue, regVal & 31);
    case 24: // video mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 25: // aux left
        return mapValue(31, 0, set->MinValue, set->MaxValue, (regVal >> 8) & 31);
    case 26: // aux right
        return mapValue(31, 0, set->MinValue, set->MaxValue, regVal & 31);
    case 27: // aux mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 28: // pcm out left
        return mapValue(31, 0, set->MinValue, set->MaxValue, (regVal >> 8) & 31);
    case 29: // pcm out right
        return mapValue(31, 0, set->MinValue, set->MaxValue, regVal & 31);
    case 30: // pcm out mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 31: // record select left
        return mapValue(0, 7, set->MinValue, set->MaxValue, (regVal >> 8) & 7);
    case 32: // record select right
        return mapValue(0, 7, set->MinValue, set->MaxValue, regVal & 7);

    case 33: // record gain left
        return mapValue(15, 0, set->MinValue, set->MaxValue, (regVal >> 8) & 15);
    case 34: // record gain right
        return mapValue(15, 0, set->MinValue, set->MaxValue, regVal & 15);
    case 35: // record gain mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);

    case 36: // record gain mic
        return mapValue(15, 0, set->MinValue, set->MaxValue, regVal & 15);
    case 37: // record gain mic mute
        return mapValue(0, 1, set->MinValue, set->MaxValue, (regVal >> 15) & 1);
    }
    return -EINVAL;
}

AC97::~AC97()
{
}

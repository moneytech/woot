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
    if(setting < 0 || setting >= (sizeof(mixerSettings) / sizeof(AudioDevice::MixerSetting)))
        return -EINVAL;
    AudioDevice::MixerSetting *set = mixerSettings + setting;
    int regVal = RegisterRead(regMap[setting]);
    if(regVal < 0) return regVal;
    switch(setting)
    {
    case 0: // master left
        regVal = regVal & ~0x3F00 | mapValue(set->MinValue, set->MaxValue, 63, 0, value) << 8;
        break;
    case 1: // master right
        regVal = regVal & ~0x003F | mapValue(set->MinValue, set->MaxValue, 63, 0, value);
        break;
    case 2: // master mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 3: // aux left
        regVal = regVal & ~0x3F00 | mapValue(set->MinValue, set->MaxValue, 63, 0, value) << 8;
        break;
    case 4: // aux right
        regVal = regVal & ~0x003F | mapValue(set->MinValue, set->MaxValue, 63, 0, value);
        break;
    case 5: // aux mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 6: // mono
        regVal = regVal & ~0x003F | mapValue(set->MinValue, set->MaxValue, 63, 0, value);
        break;
    case 7: // mono mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 8: // master bass
        regVal = regVal & ~0x0F00 | mapValue(set->MinValue, set->MaxValue, 15, 0, value) << 8;
        break;
    case 9: // master treble
        regVal = regVal & ~0x000F | mapValue(set->MinValue, set->MaxValue, 15, 0, value);
        break;

    case 10: // beep volume
        regVal = regVal & ~0x001E | mapValue(set->MinValue, set->MaxValue, 15, 0, value) << 1;
        break;

    case 11: // phone
        regVal = regVal & ~0x001F | mapValue(set->MinValue, set->MaxValue, 31, 0, value);
        break;
    case 12: // phone mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 13: // mic volume
        regVal = regVal & ~0x001F | mapValue(set->MinValue, set->MaxValue, 31, 0, value);
        break;
    case 14: // mic boost
        regVal = regVal & ~0x0040 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 6;
        break;
    case 15: // mic mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 16: // line left
        regVal = regVal & ~0x1F00 | mapValue(set->MinValue, set->MaxValue, 31, 0, value) << 8;
        break;
    case 17: // line right
        regVal = regVal & ~0x001F | mapValue(set->MinValue, set->MaxValue, 31, 0, value);
        break;
    case 18: // line mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 19: // cd left
        regVal = regVal & ~0x1F00 | mapValue(set->MinValue, set->MaxValue, 31, 0, value) << 8;
        break;
    case 20: // cd right
        regVal = regVal & ~0x001F | mapValue(set->MinValue, set->MaxValue, 31, 0, value);
        break;
    case 21: // cd mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 22: // video left
        regVal = regVal & ~0x1F00 | mapValue(set->MinValue, set->MaxValue, 31, 0, value) << 8;
        break;
    case 23: // video right
        regVal = regVal & ~0x001F | mapValue(set->MinValue, set->MaxValue, 31, 0, value);
        break;
    case 24: // video mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 25: // aux left
        regVal = regVal & ~0x1F00 | mapValue(set->MinValue, set->MaxValue, 31, 0, value) << 8;
        break;
    case 26: // aux right
        regVal = regVal & ~0x001F | mapValue(set->MinValue, set->MaxValue, 31, 0, value);
        break;
    case 27: // aux mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 28: // pcm left
        regVal = regVal & ~0x1F00 | mapValue(set->MinValue, set->MaxValue, 31, 0, value) << 8;
        break;
    case 29: // pcm right
        regVal = regVal & ~0x001F | mapValue(set->MinValue, set->MaxValue, 31, 0, value);
        break;
    case 30: // pcm mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 31: // record select left
        regVal = regVal & ~0x0700 | mapValue(set->MinValue, set->MaxValue, 7, 0, value) << 8;
        break;
    case 32: // record select right
        regVal = regVal & ~0x0007 | mapValue(set->MinValue, set->MaxValue, 7, 0, value);
        break;

    case 33: // record gain left
        regVal = regVal & ~0x0F00 | mapValue(set->MinValue, set->MaxValue, 15, 0, value) << 8;
        break;
    case 34: // record gain right
        regVal = regVal & ~0x000F | mapValue(set->MinValue, set->MaxValue, 15, 0, value);
        break;
    case 35: // record gain mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    case 36: // record gain mic right
        regVal = regVal & ~0x000F | mapValue(set->MinValue, set->MaxValue, 15, 0, value);
        break;
    case 37: // record gain mic mute
        regVal = regVal & ~0x8000 | mapValue(set->MinValue, set->MaxValue, 0, 1, value) << 15;
        break;

    default:
        return -EINVAL;
        break;
    }
    return RegisterWrite(regMap[setting], regVal);
}

int AC97::GetMixerSetting(int setting)
{
    if(setting < 0 || setting >= (sizeof(mixerSettings) / sizeof(AudioDevice::MixerSetting)))
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

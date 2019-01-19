#include <audiodevice.h>
#include <cmi8738.h>
#include <stdio.h>

extern AudioDevice::MixerSetting mixerSettings[25];

extern "C" int Initialize()
{
    printf("[cmi8738] Initialize()\n");

    mixerSettings[0] = AudioDevice::MixerSetting(0, 100, "Master volume L");
    mixerSettings[1] = AudioDevice::MixerSetting(0, 100, "Master volume R");
    mixerSettings[2] = AudioDevice::MixerSetting(0, 100, "Wave volume L");
    mixerSettings[3] = AudioDevice::MixerSetting(0, 100, "Wave volume R");
    mixerSettings[4] = AudioDevice::MixerSetting(0, 100, "MIDI volume L");
    mixerSettings[5] = AudioDevice::MixerSetting(0, 100, "MIDI volume R");
    mixerSettings[6] = AudioDevice::MixerSetting(0, 100, "CD volume L");
    mixerSettings[7] = AudioDevice::MixerSetting(0, 100, "CD volume R");
    mixerSettings[8] = AudioDevice::MixerSetting(0, 100, "Line-in volume L");
    mixerSettings[9] = AudioDevice::MixerSetting(0, 100, "Line-in volume R");
    mixerSettings[10] = AudioDevice::MixerSetting(0, 100, "Mic. volume");
    mixerSettings[11] = AudioDevice::MixerSetting(0, 100, "PC spk volume");

    mixerSettings[12] = AudioDevice::MixerSetting(0, 1, "Line L mute");
    mixerSettings[13] = AudioDevice::MixerSetting(0, 1, "Line R mute");
    mixerSettings[14] = AudioDevice::MixerSetting(0, 1, "CD L mute");
    mixerSettings[15] = AudioDevice::MixerSetting(0, 1, "CD R mute");
    mixerSettings[16] = AudioDevice::MixerSetting(0, 1, "Mic mute");

    mixerSettings[17] = AudioDevice::MixerSetting(0, 1, "FM L record L");
    mixerSettings[18] = AudioDevice::MixerSetting(0, 1, "FM R record L");
    mixerSettings[19] = AudioDevice::MixerSetting(0, 1, "Line L record L");
    mixerSettings[20] = AudioDevice::MixerSetting(0, 1, "Line R record L");
    mixerSettings[21] = AudioDevice::MixerSetting(0, 1, "CD L record L");
    mixerSettings[22] = AudioDevice::MixerSetting(0, 1, "CD R record L");
    mixerSettings[23] = AudioDevice::MixerSetting(0, 1, "Mic record L");

    mixerSettings[24] = AudioDevice::MixerSetting(0, 1, "FM L record R");
    mixerSettings[25] = AudioDevice::MixerSetting(0, 1, "FM R record R");
    mixerSettings[26] = AudioDevice::MixerSetting(0, 1, "Line L record R");
    mixerSettings[27] = AudioDevice::MixerSetting(0, 1, "Line R record R");
    mixerSettings[28] = AudioDevice::MixerSetting(0, 1, "CD L record R");
    mixerSettings[29] = AudioDevice::MixerSetting(0, 1, "CD R record R");
    mixerSettings[30] = AudioDevice::MixerSetting(0, 1, "Mic record R");

    mixerSettings[31] = AudioDevice::MixerSetting(0, 0, nullptr);

    CMI8738::Initialize();
    return 0;
}

extern "C" void Cleanup()
{
    printf("[cmi8738] Cleanup()\n");
    CMI8738::Cleanup();
}

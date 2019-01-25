#include <woot/audio.h>
#include <sys/syscall.h>

int auOpenDevice(int id, int rate, int channels, int bits, int samples)
{
    return syscall5(SYS_audio_open, id, rate, channels, bits, samples);
}

int auCloseDevice(int id)
{
    return syscall1(SYS_audio_close, id);
}

int auGetFrameSize(int id)
{
    return syscall1(SYS_audio_get_frame_size, id);
}

int auWriteDevice(int id, void *buffer)
{
    return syscall2(SYS_audio_write, id, (long)buffer);
}

int auStartPlayback(int id)
{
    return syscall1(SYS_audio_start_playback, id);
}

int auStopPlayback(int id)
{
    return syscall1(SYS_audio_stop_playback, id);
}

int auGetBufferCount(int id)
{
    return syscall1(SYS_audio_get_buffer_count, id);
}

int auGetDeviceVendor(int id, char *buffer, int bufSize)
{
    return syscall3(SYS_audio_get_device_vendor, id, (long)buffer, bufSize);
}

int auGetDeviceModel(int id, char *buffer, int bufSize)
{
    return syscall3(SYS_audio_get_device_model, id, (long)buffer, bufSize);
}

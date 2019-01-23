#ifndef AUDIO_H
#define AUDIO_H

int auOpenDevice(int id, int rate, int channels, int bits, int samples);
int auCloseDevice(int id);
int auGetFrameSize(int id);
int auWriteDevice(int id, void *buffer);
int auStartPlayback(int id);
int auStopPlayback(int id);
int auGetBufferCount(int id);
int auGetDeviceVendor(int id, char *buffer, int bufSize);
int auGetDeviceModel(int id, char *buffer, int bufSize);

#endif // AUDIO_H

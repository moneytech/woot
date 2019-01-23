#ifndef AUDIO_H
#define AUDIO_H

int auOpenDevice(int id, int rate, int channels, int bits, int samples);
int auCloseDevice(int id);
int auGetFrameSize(int id);
int auWriteDevice(int id, void *buffer);
int auStartPlayback(int id);
int auStopPlayback(int id);
int auGetBufferCount(int id);

#endif // AUDIO_H

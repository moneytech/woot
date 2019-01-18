#include <audiodevice.h>

List<AudioDevice *> AudioDevice::devices;
Mutex AudioDevice::lock;

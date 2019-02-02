#ifndef TIME_H
#define TIME_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int tmSleep(int ms);
unsigned long long tmGetTicks();
unsigned long long tmGetTickFreq();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TIME_H

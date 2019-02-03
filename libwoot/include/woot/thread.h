#ifndef THREAD_H
#define THREAD_H

int thrGetCurrentID(); // get current thread id
int thrCreate(void *entry); // create new thread
int thrDelete(int id); // stop and delete (abort) specified thread
int thrSuspend(int id); // suspend specified thread
int thrResume(int id); // resume specified thread (if suspended or at interruptible sleep)
int thrSleep(int id, int ms); // make thread go to sleep for ms milliseconds (negative value means interruptible sleep)


#endif // THREAD_H

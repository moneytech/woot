#ifndef PROCESS_H
#define PROCESS_H

#include <list.h>
#include <sequencer.h>
#include <types.h>

class DEntry;
class Mutex;
class Thread;

class Process
{
    static Sequencer<pid_t> id;
    static List<Process *> *processList;
    static Mutex *listLock;
    List<Thread *> *threads;
    Mutex *lock;
public:
    pid_t ID;
    char *Name;
    uintptr_t AddressSpace;
    uid_t UID, EUID;
    gid_t GID, EGID;
    DEntry *CurrentDirectory;

    static void Initialize();
    static uintptr_t NewAddressSpace();
    static void Cleanup();

    Process(const char *name, Thread *mainThread, uintptr_t addressSpace);
    bool AddThread(Thread *thread);
    bool RemoveThread(Thread *thread);
    ~Process();
};

#endif // PROCESS_H

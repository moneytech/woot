#ifndef PROCESS_H
#define PROCESS_H

#include <elf.h>
#include <list.h>
#include <mutex.h>
#include <sequencer.h>
#include <types.h>

class DEntry;
class ELF;
class Semaphore;
class Thread;

class Process
{
    static Sequencer<pid_t> id;
    static List<Process *> processList;
    static Mutex listLock;
    static uintptr_t kernelAddressSpace;
    List<Thread *> threads;
    Mutex lock;

    static int processEntryPoint(const char *filename);
public:
    pid_t ID;
    char *Name;
    uintptr_t AddressSpace;
    uid_t UID, EUID;
    gid_t GID, EGID;
    DEntry *CurrentDirectory;
    List<ELF *> Images;

    static void Initialize();
    static Process *Create(const char *filename, Semaphore *finished);
    static Process *GetCurrent();
    static DEntry *GetCurrentDir();
    static uintptr_t NewAddressSpace();
    static void Cleanup();

    Process(const char *name, Thread *mainThread, uintptr_t addressSpace);
    bool Start();
    bool AddThread(Thread *thread);
    bool RemoveThread(Thread *thread);
    Elf32_Sym *FindSymbol(const char *name);
    ~Process();
};

#endif // PROCESS_H

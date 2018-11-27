#ifndef PROCESS_H
#define PROCESS_H

#include <elf.h>
#include <list.h>
#include <mutex.h>
#include <sequencer.h>
#include <types.h>

class DEntry;
class ELF;
class File;
class Semaphore;
class Thread;

#define MAX_FILE_DESCRIPTORS 128

class Process
{
    static Sequencer<pid_t> id;
    static List<Process *> processList;
    static Mutex listLock;
    static uintptr_t kernelAddressSpace;
    Mutex lock;

    static uintptr_t buildUserStack(uintptr_t stackPtr, const char *cmdLine, int envCount, const char *envVars[], ELF *elf, uintptr_t retAddr, uintptr_t basePointer);
    static int processEntryPoint(const char *cmdline);
public:
    pid_t ID;
    char *Name;
    uintptr_t AddressSpace;
    List<ELF *> Images;
    uid_t UID, EUID;
    gid_t GID, EGID;
    DEntry *CurrentDirectory;
    uintptr_t UserStackPtr;
    File *FileDescriptors[MAX_FILE_DESCRIPTORS];

    // used for brk() syscall
    Mutex MemoryLock;
    uintptr_t MinBrk;
    uintptr_t MaxBrk;
    uintptr_t CurrentBrk;
    uintptr_t MappedBrk;

    List<Thread *> Threads;
    bool SelfDestruct;

    static void Initialize();
    static Process *Create(const char *filename, Semaphore *finished);
    static Process *GetCurrent();
    static DEntry *GetCurrentDir();
    static uintptr_t NewAddressSpace();
    static void Cleanup();
    static void Dump();

    Process(const char *name, Thread *mainThread, uintptr_t addressSpace, bool SelfDestruct);
    bool Start();
    bool AddThread(Thread *thread);
    bool RemoveThread(Thread *thread);
    bool AddELF(ELF *elf);
    ELF *GetELF(const char *name);
    Elf32_Sym *FindSymbol(const char *name, ELF *skip, ELF **elf);
    bool ApplyRelocations();
    int Open(const char *filename, int flags);
    int Close(int fd);
    File *GetFileDescriptor(int fd);
    ~Process();
};

#endif // PROCESS_H

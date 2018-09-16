#ifndef INTS_H
#define INTS_H

#include <types.h>

class Ints
{
public:
#pragma pack(push, 1)
struct State
{
    union //sESP
    {
        dword ESP;
        word SP;
    };
    struct //sSS
    {
        word SS;
        word hSS;
    };
    struct //sGS
    {
        word GS;
        word hGS;
    };
    struct //sFS
    {
        word FS;
        word hFS;
    };
    struct //sES
    {
        word ES;
        word hES;
    };
    struct //sDS
    {
        word DS;
        word hDS;
    };
    union //sEDI
    {
        dword EDI;
        word DI;
    };
    union //sESI
    {
        dword ESI;
        word SI;
    };
    union //sEBP
    {
        dword EBP;
        word BP;
    };
    union //sEBX
    {
        dword EBX;
        word BX;
        struct
        {
            byte BL;
            byte BH;
        };
    };
    union //sEDX
    {
        dword EDX;
        word DX;
        struct
        {
            byte DL;
            byte DH;
        };
    };
    union //sECX
    {
        dword ECX;
        word CX;
        struct
        {
            byte CL;
            byte CH;
        };
    };
    union //sEAX
    {
        dword EAX;
        word AX;
        struct
        {
            byte AL;
            byte AH;
        };
    };
    dword InterruptNumber;
    dword ErrorCode;

    union //sEIP
    {
        dword EIP;
        word IP;
    };
    struct //sCS
    {
        word CS;
        word hCS;
    };
    union //sEFLAGS
    {
        dword EFLAGS;
        word FLAGS;
    };
    union //sUserESP
    {
        dword UserESP;
        word UserSP;
    };
    union //sUserSS
    {
        word UserSS;
        word hUserSS;
    };
};
#pragma pack(pop)
private:
    typedef bool (*HandlerCallback)(State *state, void *context);
public:
    struct Handler
    {
        Handler *Next; // set to 0
        HandlerCallback Callback;
        void *Context;
    };
private:
    static Handler *Handlers[];
public:
    static void CommonHandler(State *state);
    static void RegisterHandler(int intNo, Handler *handler);
    static void UnRegisterHandler(int intNo, Handler *handler);
};

// returns true if interrupt was properly handled



#endif // INTS_H

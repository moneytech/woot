#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <list.h>
#include <mutex.h>
#include <pixmap.h>
#include <sequencer.h>
#include <types.h>

class FrameBuffer
{
    static Sequencer<int> ids;
    static List<FrameBuffer *> fbs;
    static Mutex listLock;
    Mutex lock;
public:
    struct ModeInfo
    {
        int Width, Height;
        size_t Pitch;
        PixMap::PixelFormat Format;

        ModeInfo();
        ModeInfo(int width, int height, size_t pitch, PixMap::PixelFormat format);
        /*int BPP;
        int AlphaShift, RedShift, GreenShift, BlueShift;
        int AlphaBits, RedBits, GreenBits, BlueBits;*/
    };
    /*union Color
    {
        uint32_t Value;
        struct { uint8_t A, R, G, B; };

        Color();
        Color(const Color &src);
        Color(uint8_t r, uint8_t g, uint8_t b);
        Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
        Color(uint32_t value);
        static Color FromFloatRGB(float r, float g, float b);
        uint32_t ToValue(ModeInfo &mode);
        static Color FromValue(ModeInfo &mode, uint32_t value);
    };*/
protected:
    int ID;
    /*union
    {
        void *Pixels;
        uint8_t *PixelBytes;
    };*/
public:
    PixMap *Pixels;

    static int Add(FrameBuffer *fb);
    static FrameBuffer *GetByID(int id, bool lock);
    static int Remove(FrameBuffer *fb);

    FrameBuffer();
    //ModeInfo GetMode();
    int Lock();
    void *GetPixels();
    void UnLock();

    virtual int GetModeCount();
    virtual int GetModes(ModeInfo *buffer, size_t maxModes);
    virtual int SetMode(ModeInfo mode);
    virtual ~FrameBuffer();
};

#endif // FRAMEBUFFER_H

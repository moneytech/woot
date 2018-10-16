#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <list.h>
#include <mutex.h>
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
        int Width, Height, BPP;
        size_t Pitch;
        int AlphaShift, RedShift, GreenShift, BlueShift;
        int AlphaBits, RedBits, GreenBits, BlueBits;
    };
    union Color
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
    };
protected:
    int ID;
    union
    {
        void *Pixels;
        uint8_t *PixelBytes;
    };
    ModeInfo Mode;
public:
    static int Add(FrameBuffer *fb);
    static FrameBuffer *GetByID(int id, bool lock);
    static int Remove(FrameBuffer *fb);

    FrameBuffer();
    int SetMode(ModeInfo mode);
    ModeInfo GetMode();
    int Lock();
    void *GetPixels();
    void UnLock();

    virtual void SetPixel(int x, int y, Color c);
    virtual Color GetPixel(int x, int y);
    virtual void Clear(Color c);
    virtual void FillRectangle(int x, int y, int w, int h, Color c);
    virtual ~FrameBuffer();
};

#endif // FRAMEBUFFER_H

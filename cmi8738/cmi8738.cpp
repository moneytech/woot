#include <cmi8738.h>
#include <cpu.h>
#include <errno.h>
#include <irqs.h>
#include <pci.h>
#include <mutex.h>
#include <paging.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdefs.h>
#include <time.h>

#define CM_REG_FUNCTRL0		0x00
#define CM_RST_CH1          0x00080000
#define CM_RST_CH0          0x00040000
#define CM_CHEN1            0x00020000	/* ch1: enable */
#define CM_CHEN0            0x00010000	/* ch0: enable */
#define CM_PAUSE1           0x00000008	/* ch1: pause */
#define CM_PAUSE0           0x00000004	/* ch0: pause */
#define CM_CHADC1           0x00000002	/* ch1, 0:playback, 1:record */
#define CM_CHADC0           0x00000001	/* ch0, 0:playback, 1:record */

#define CM_REG_FUNCTRL1		0x04
#define CM_DSFC_MASK		0x0000E000	/* channel 1 (DAC?) sampling frequency */
#define CM_DSFC_SHIFT		13
#define CM_ASFC_MASK		0x00001C00	/* channel 0 (ADC?) sampling frequency */
#define CM_ASFC_SHIFT		10
#define CM_SPDF_1           0x00000200	/* SPDIF IN/OUT at channel B */
#define CM_SPDF_0           0x00000100	/* SPDIF OUT only channel A */
#define CM_SPDFLOOP         0x00000080	/* ext. SPDIF/IN -> OUT loopback */
#define CM_SPDO2DAC         0x00000040	/* SPDIF/OUT can be heard from internal DAC */
#define CM_INTRM            0x00000020	/* master control block (MCB) interrupt enabled */
#define CM_BREQ             0x00000010	/* bus master enabled */
#define CM_VOICE_EN         0x00000008	/* legacy voice (SB16,FM) */
#define CM_UART_EN          0x00000004	/* legacy UART */
#define CM_JYSTK_EN         0x00000002	/* legacy joystick */
#define CM_ZVPORT           0x00000001	/* ZVPORT */

#define CM_REG_CHFORMAT		0x08

#define CM_REG_INT_HLDCLR	0x0C
#define CM_CHIP_MASK2		0xff000000
#define CM_CHIP_8768		0x20000000
#define CM_CHIP_055         0x08000000
#define CM_CHIP_039         0x04000000
#define CM_CHIP_039_6CH		0x01000000
#define CM_UNKNOWN_INT_EN	0x00080000	/* ? */
#define CM_TDMA_INT_EN		0x00040000
#define CM_CH1_INT_EN		0x00020000
#define CM_CH0_INT_EN		0x00010000

#define CM_REG_INT_STATUS	0x10
#define CM_INTR             0x80000000
#define CM_VCO              0x08000000	/* Voice Control? CMI8738 */
#define CM_MCBINT           0x04000000	/* Master Control Block abort cond.? */
#define CM_UARTINT          0x00010000
#define CM_LTDMAINT         0x00008000
#define CM_HTDMAINT         0x00004000
#define CM_XDO46            0x00000080	/* Modell 033? Direct programming EEPROM (read data register) */
#define CM_LHBTOG           0x00000040	/* High/Low status from DMA ctrl register */
#define CM_LEG_HDMA         0x00000020	/* Legacy is in High DMA channel */
#define CM_LEG_STEREO		0x00000010	/* Legacy is in Stereo mode */
#define CM_CH1BUSY          0x00000008
#define CM_CH0BUSY          0x00000004
#define CM_CHINT1           0x00000002
#define CM_CHINT0           0x00000001

#define CM_REG_MISC_CTRL	0x18
#define CM_PWD              0x80000000	/* power down */
#define CM_RESET            0x40000000
#define CM_SFIL_MASK		0x30000000	/* filter control at front end DAC, model 037? */
#define CM_VMGAIN           0x10000000	/* analog master amp +6dB, model 039? */
#define CM_TXVX             0x08000000	/* model 037? */
#define CM_N4SPK3D          0x04000000	/* copy front to rear */
#define CM_SPDO5V           0x02000000	/* 5V spdif output (1 = 0.5v (coax)) */
#define CM_SPDIF48K         0x01000000	/* write */
#define CM_SPATUS48K		0x01000000	/* read */
#define CM_ENDBDAC          0x00800000	/* enable double dac */
#define CM_XCHGDAC          0x00400000	/* 0: front=ch0, 1: front=ch1 */
#define CM_SPD32SEL         0x00200000	/* 0: 16bit SPDIF, 1: 32bit */
#define CM_SPDFLOOPI		0x00100000	/* int. SPDIF-OUT -> int. IN */
#define CM_FM_EN            0x00080000	/* enable legacy FM */
#define CM_AC3EN2           0x00040000	/* enable AC3: model 039 */
#define CM_ENWRASID         0x00010000	/* choose writable internal SUBID (audio) */
#define CM_VIDWPDSB         0x00010000	/* model 037? */
#define CM_SPDF_AC97		0x00008000	/* 0: SPDIF/OUT 44.1K, 1: 48K */
#define CM_MASK_EN          0x00004000	/* activate channel mask on legacy DMA */
#define CM_ENWRMSID         0x00002000	/* choose writable internal SUBID (modem) */
#define CM_VIDWPPRT         0x00002000	/* model 037? */
#define CM_SFILENB          0x00001000	/* filter stepping at front end DAC, model 037? */
#define CM_MMODE_MASK		0x00000E00	/* model DAA interface mode */
#define CM_SPDIF_SELECT2	0x00000100	/* for model > 039 ? */
#define CM_ENCENTER         0x00000080
#define CM_FLINKON          0x00000040	/* force modem link detection on, model 037 */
#define CM_MUTECH1          0x00000040	/* mute PCI ch1 to DAC */
#define CM_FLINKOFF         0x00000020	/* force modem link detection off, model 037 */
#define CM_MIDSMP           0x00000010	/* 1/2 interpolation at front end DAC */
#define CM_UPDDMA_MASK		0x0000000C	/* TDMA position update notification */
#define CM_UPDDMA_2048		0x00000000
#define CM_UPDDMA_1024		0x00000004
#define CM_UPDDMA_512		0x00000008
#define CM_UPDDMA_256		0x0000000C
#define CM_TWAIT_MASK		0x00000003	/* model 037 */
#define CM_TWAIT1           0x00000002	/* FM i/o cycle, 0: 48, 1: 64 PCICLKs */
#define CM_TWAIT0           0x00000001	/* i/o cycle, 0: 4, 1: 6 PCICLKs */

#define CM_REG_SB16_DATA	0x22
#define CM_REG_SB16_ADDR	0x23

#define CM_REG_CH0_FRAME1	0x80	/* write: base address */
#define CM_REG_CH0_FRAME2	0x84	/* read: current address */
#define CM_REG_CH1_FRAME1	0x88	/* 0-15: count of samples at bus master; buffer size */
#define CM_REG_CH1_FRAME2	0x8C	/* 16-31: count of samples at codec; fragment size */

static bool testAndSet(bool *lock)
{
    bool init = *lock;
    *lock = true;
    return init;
}

bool CMI8738::interrupt(Ints::State *state, void *context)
{
    CMI8738 *dev = (CMI8738 *)context;
    uint32_t status = _inl(dev->base + CM_REG_INT_STATUS);

    if(!(status & CM_INTR))
        return false; // not our interrupt

    // clear interrupts
    uint32_t mask = 0;
    if(status & CM_CHINT0)
        mask |= CM_CH0_INT_EN;
    if(status & CM_CHINT1)
        mask |= CM_CH1_INT_EN;
    cpuIOClrBitsL(dev->base + CM_REG_INT_HLDCLR, mask);
    cpuIOSetBitsL(dev->base + CM_REG_INT_HLDCLR, mask);

    ++dev->playedBuffer;
    dev->bufSem->Signal(state);

    return true;
}

void CMI8738::mixerWrite(byte idx, byte val)
{
    _outb(base + CM_REG_SB16_ADDR, idx);
    _outb(base + CM_REG_SB16_DATA, val);
}

byte CMI8738::mixerRead(byte idx)
{
    _outb(base + CM_REG_SB16_ADDR, idx);
    return _inb(base + CM_REG_SB16_DATA);
}

void CMI8738::mixerReset()
{
    mixerWrite(0x00, 0x00);
}

void CMI8738::chanReset(int ch)
{
    uint32_t mask = ch ? CM_RST_CH1 : CM_RST_CH1;
    cpuIOSetBitsL(base + CM_REG_FUNCTRL0, mask);
    cpuIOClrBitsL(base + CM_REG_FUNCTRL0, mask);
    Time::Sleep(1, false);
}

void CMI8738::bmDspReset()
{
    cpuIOSetBitsL(base + CM_REG_MISC_CTRL, CM_RESET);
    cpuIOClrBitsL(base + CM_REG_MISC_CTRL, CM_RESET);
    Time::Sleep(1, false);
}

void CMI8738::fullReset()
{
    bmDspReset();
    chanReset(0);
    chanReset(1);
    mixerReset();
}

uint32_t CMI8738::getSFSel(int playbackRate)
{
    switch(playbackRate)
    {
    case 5512:
        return 0;
    case 8000:
        return 4;
    case 11025:
        return 1;
    case 16000:
        return 5;
    case 22050:
        return 2;
    case 32000:
        return 6;
    case 44100:
        return 3;
    case 48000:
        return 7;
    }
    return ~0;
}

uint32_t CMI8738::getFmt(int playbackChannels, int playbackBits)
{
    if(playbackChannels == 1)
    {
        if(playbackBits == 8)
            return 0;
        else if(playbackBits == 16)
            return 2;
        return ~0;
    }
    else if(playbackChannels == 2)
    {
        if(playbackBits == 8)
            return 1;
        else if(playbackBits == 16)
            return 3;
    }
    return ~0;
}

void CMI8738::Initialize()
{
    PCI::Lock->Acquire(0, false);
    for(PCI::Device *pciDev : *PCI::Devices)
    {
        if(pciDev->VendorID != 0x13F6 || pciDev->DeviceID != 0x0111)
            continue;

        printf("[cmi8738] Found compatible device at PCI:%d.%d.%d\n",
               PCI_ADDR_BUS(pciDev->Address),
               PCI_ADDR_DEV(pciDev->Address),
               PCI_ADDR_FUNC(pciDev->Address));

        // enable PCI BusMaster and IO address space accesses
        PCI::WriteConfigWord(pciDev->Address | 0x04, PCI::ReadConfigWord(pciDev->Address | 0x04) | 0x0005);

        PCI::Config cfg;
        PCI::ReadConfigData(&cfg, pciDev->Address);

        CMI8738 *device = new CMI8738(cfg.Header.Default.BAR[0] & ~3, cfg.Header.Default.InterruptLine);
        printf("          base: %#.4x irq: %u\n", device->base, device->irq);
        AudioDevice::Add(device);
    }

    PCI::Lock->Release();
}

void CMI8738::Cleanup()
{

}

CMI8738::CMI8738(uint16_t base, uint8_t irq) :
    base(base), irq(irq),
    interruptHandler { nullptr, interrupt, this },
    bufSem(new Semaphore(1))
{
    IRQs::RegisterHandler(irq, &interruptHandler);
    IRQs::Enable(irq);

    fullReset();
}

const char *CMI8738::GetVendor()
{
    return "C-Media";
}

const char *CMI8738::GetModel()
{
    return "CMI8738";
}

int CMI8738::Open(int rate, int channels, int bits, int samples)
{
    sf = getSFSel(rate);
    fmt = getFmt(channels, bits);
    if(sf == ~0 || fmt == ~0)
        return -EINVAL; // format not supported
    this->samples = samples;

    // make sure that device is not busy
    if(testAndSet(&opened))
        return -EBUSY;

    fullReset();

    bufferSize = 2 * samples * (bits / 8) * channels;
    buffer = (byte *)Paging::AllocDMA(bufferSize);
    bufferPhAddr = Paging::GetPhysicalAddress(Paging::GetAddressSpace(), (uintptr_t)buffer);

    for(uint i = 0; i < bufferSize; ++i)
        buffer[i] = i;

    return 0;
}

int CMI8738::GetFrameSize()
{
    return bufferSize / 2;
}

int CMI8738::Start()
{
    if(!opened) return -EBUSY;

    playedBuffer = 0;

    _outl(base + CM_REG_FUNCTRL1, (sf << CM_ASFC_SHIFT) | CM_BREQ);
    cpuIOClrBitsL(base + CM_REG_FUNCTRL0, CM_CHADC0 | CM_PAUSE0 | CM_CHEN0); // ch0 -> playback, ch0 not paused, ch0 disable
    _outl(base + CM_REG_CHFORMAT, (_inl(base + CM_REG_CHFORMAT) & ~0x03) | fmt); // set sample format

    _outl(base + CM_REG_CH0_FRAME1, bufferPhAddr);
    _outw(base + CM_REG_CH0_FRAME2, 2 * samples - 1);
    _outw(base + CM_REG_CH0_FRAME2 + 2, samples - 1);

    cpuIOSetBitsL(base + CM_REG_INT_HLDCLR, CM_CH0_INT_EN); // enable interrupts
    cpuIOSetBitsL(base + CM_REG_FUNCTRL0, CM_CHEN0); // start channel 0

    return 0;
}

int CMI8738::Stop()
{
    if(!opened) return -EBUSY;
    cpuIOClrBitsL(base + CM_REG_INT_HLDCLR, CM_CH0_INT_EN); // disable interrupts
    cpuIOClrBitsL(base + CM_REG_FUNCTRL0, CM_CHEN0); // ch0 disable
    bufSem->Signal(nullptr);
    return 0;
}

int CMI8738::Pause()
{
    if(!opened) return -EBUSY;
    cpuIOSetBitsL(base + CM_REG_FUNCTRL0, CM_PAUSE0);
    return 0;
}

int CMI8738::Resume()
{
    if(!opened) return -EBUSY;
    cpuIOClrBitsL(base + CM_REG_FUNCTRL0, CM_PAUSE0);
    return 0;
}

int CMI8738::Write(void *buffer)
{
    if(!opened) return -EBUSY;
    bool ints = cpuAreInterruptsEnabled();
    if(!bufSem->Wait(5000, false, true))
    {
        cpuRestoreInterrupts(ints);
        return -EBUSY;
    };
    byte *dst = this->buffer + (playedBuffer & 1) * bufferSize / 2;
    memcpy(dst, buffer, bufferSize / 2);
    cpuRestoreInterrupts(ints);
    return 0;
}

void CMI8738::Close()
{
    opened = false;
}

CMI8738::~CMI8738()
{
    bool ints = cpuDisableInterrupts();
    IRQs::UnRegisterHandler(irq, &interruptHandler);
    IRQs::TryDisable(irq);
    cpuRestoreInterrupts(ints);

    Paging::FreeDMA(buffer, bufferSize);
    delete bufSem;
}

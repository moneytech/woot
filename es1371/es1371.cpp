#include <cpu.h>
#include <errno.h>
#include <es1371.h>
#include <ints.h>
#include <irqs.h>
#include <mutex.h>
#include <paging.h>
#include <pci.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>

#define REG_CONTROL             0x00
#define REG_STATUS              0x04
#define REG_UART                0x08
#define REG_PAGE                0x0C
#define REG_SRC                 0x10
#define REG_CODEC               0x14
#define REG_LEGACY              0x18
#define REG_SERIAL              0x20
#define REG_DAC1_SAMPLE_COUNT   0x24
#define REG_DAC2_SAMPLE_COUNT   0x28
#define REG_ADC_SAMPLE_COUNT    0x2C

#define CONTROL_DAC1_EN (1 << 6)
#define CONTROL_DAC2_EN (1 << 5)
#define CONTROL_ADC_EN  (1 << 4)

#define STATUS_INTR (1 << 31)
#define STATUS_DAC1 (1 << 2)
#define STATUS_DAC2 (1 << 1)
#define STATUS_ADC  (1 << 0)

#define SRC_RAM_WE      (1 << 24)
#define SRC_RAM_BUSY    (1 << 23)
#define SRC_DISABLE     (1 << 22)
#define SRC_DIS_P1      (1 << 21)
#define SRC_DIS_P2      (1 << 20)
#define SRC_DIS_R1      (1 << 19)

#define CODEC_RDY	(1 << 31)
#define CODEC_WIP	(1 << 30)
#define CODEC_MAGIC (1 << 26)
#define CODEC_PIRD  (1 << 23)

#define SERIAL_P1_LOOP_SEL  (1 << 13)
#define SERIAL_R1_INTR_EN   (1 << 10)
#define SERIAL_P2_INTR_EN   (1 << 9)
#define SERIAL_P1_INTR_EN   (1 << 8)
#define SERIAL_R1_S_EB      (1 << 5)
#define SERIAL_R1_S_MB      (1 << 4)
#define SERIAL_P2_S_EB      (1 << 3)
#define SERIAL_P2_S_MB      (1 << 2)
#define SERIAL_P1_S_EB      (1 << 1)
#define SERIAL_P1_S_MB      (1 << 0)

#define SRC_REG_DAC1		0x70
#define SRC_REG_DAC2		0x74
#define SRC_REG_ADC         0x78
#define SRC_REG_VOL_ADC     0x6c
#define SRC_REG_VOL_DAC1	0x7c
#define SRC_REG_VOL_DAC2    0x7e

#define SRC_REG_TRUNC_N     0x00
#define SRC_REG_INT_REGS	0x01
#define SRC_REG_ACCUM_FRAC	0x02
#define SRC_REG_VFREQ_FRAC	0x03

#define BUFFERS 3 // at least 3 buffers are requires so es1371 won't crackle

static bool testAndSet(bool *lock)
{
    bool init = *lock;
    *lock = true;
    return init;
}

bool ES1371::interrupt(Ints::State *state, void *context)
{
    ES1371 *dev = (ES1371 *)context;
    uint32_t status = _inl(dev->base + REG_STATUS);
    if(!(status & STATUS_INTR))
        return false;

    uint32_t sctrl = _inl(dev->base + REG_SERIAL);
    if(status & STATUS_DAC1)
        sctrl &= ~SERIAL_P1_INTR_EN;
    if(status & STATUS_DAC2)
        sctrl &= ~SERIAL_P2_INTR_EN;
    if(status & STATUS_ADC)
        sctrl &= ~SERIAL_R1_INTR_EN;
    _outl(dev->base + REG_SERIAL, sctrl);
    if(status & STATUS_DAC1)
        sctrl |= SERIAL_P1_INTR_EN;
    if(status & STATUS_DAC2)
        sctrl |= SERIAL_P2_INTR_EN;
    if(status & STATUS_ADC)
        sctrl |= SERIAL_R1_INTR_EN;
    _outl(dev->base + REG_SERIAL, sctrl);

    ++dev->playedBuffer;
    dev->bufSem->Signal(state);

    return true;
}

void ES1371::codecReset()
{
    cpuIOSetBitsL(base + REG_CONTROL, 1 << 14);
    Time::Sleep(1, false);
    cpuIOClrBitsL(base + REG_CONTROL, 1 << 14);
    Time::Sleep(1, false);
}

int ES1371::codecWrite(uint8_t reg, uint16_t val)
{
    int retry = 100;
    while(_inl(base + REG_CODEC) & CODEC_WIP && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    _outl(base + REG_CODEC, (((uint32_t)reg) & 0x7F) << 16 | val);
    return 0;
}

int ES1371::codecRead(uint8_t reg)
{
    int retry = 100;
    while(_inl(base + REG_CODEC) & CODEC_WIP && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    _outl(base + REG_CODEC, 0x00800000 | (((uint32_t)reg) & 0x7F) << 16);
    retry = 100;
    while(!(_inl(base + REG_CODEC) & CODEC_RDY) && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    return _inl(base + REG_CODEC) & 0xFFFF;
}

int ES1371::srcReset()
{
    int retry = 100;
    while(_inl(base + REG_SRC) & SRC_RAM_BUSY && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    _outl(base + REG_SRC, SRC_DISABLE);

    srcWrite(SRC_REG_DAC1 + SRC_REG_TRUNC_N, 16 << 4);
    srcWrite(SRC_REG_DAC1 + SRC_REG_INT_REGS, 16 << 10);
    srcWrite(SRC_REG_DAC1 + SRC_REG_ACCUM_FRAC, 0);
    srcWrite(SRC_REG_DAC1 + SRC_REG_VFREQ_FRAC, 0);

    srcWrite(SRC_REG_DAC2 + SRC_REG_TRUNC_N, 16 << 4);
    srcWrite(SRC_REG_DAC2 + SRC_REG_INT_REGS, 16 << 10);
    srcWrite(SRC_REG_DAC2 + SRC_REG_ACCUM_FRAC, 0);
    srcWrite(SRC_REG_DAC2 + SRC_REG_VFREQ_FRAC, 0);

    srcWrite(SRC_REG_VOL_DAC1 + 0, 1 << 12);
    srcWrite(SRC_REG_VOL_DAC1 + 1, 1 << 12);

    srcWrite(SRC_REG_VOL_DAC2 + 0, 1 << 12);
    srcWrite(SRC_REG_VOL_DAC2 + 1, 1 << 12);

    srcWrite(SRC_REG_VOL_ADC + 0, 1 << 12);
    srcWrite(SRC_REG_VOL_ADC + 1, 1 << 12);

    retry = 100;
    while(_inl(base + REG_SRC) & SRC_RAM_BUSY && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    _outl(base + REG_SRC, 0);
    return 0;
}

int ES1371::srcWrite(uint8_t reg, uint16_t val)
{
    uint32_t r;
    int retry = 100;
    while((r = _inl(base + REG_SRC)) & SRC_RAM_BUSY && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    r &= SRC_DISABLE | SRC_DIS_P1 | SRC_DIS_P2 | SRC_DIS_R1;
    _outl(base + REG_SRC, SRC_RAM_WE | r | ((uint32_t)reg) << 25 | val);
    return 0;
}

int ES1371::srcRead(uint8_t reg)
{
    uint32_t r;
    int retry = 100;
    while((r = _inl(base + REG_SRC)) & SRC_RAM_BUSY && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;

    r &= SRC_DISABLE | SRC_DIS_P2 | SRC_DIS_P1 | SRC_DIS_R1;;
    _outl(base + REG_SRC, ((uint32_t)reg) << 25 | r);

    retry = 100;
    while((r = _inl(base + REG_SRC)) & SRC_RAM_BUSY && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;

    return _inl(base + REG_SRC) & 0xFFFF;
}

int ES1371::srcSetRate(int base, int rate)
{
    if(base == SRC_REG_ADC)
    {
        printf("[es1371] ADC sample rate change not implemented\n");
        return -ENOSYS;
    }

    uint32_t r;
    uint16_t r2;

    uint freq = (rate << 15) / 3000;

    // disable channel
    int retry = 100;
    while((r = _inl(this->base + REG_SRC)) & SRC_RAM_BUSY && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    r |= (base == SRC_REG_DAC1 ? SRC_DIS_P1 : SRC_DIS_P2);
    _outl(this->base + REG_SRC, r);

    // set frequency
    int res = srcRead(base + SRC_REG_INT_REGS);
    if(res < 0) return res;
    r2 = res;
    res = srcWrite(base + SRC_REG_INT_REGS, (r2 & 0xFF) | ((freq >> 5) & 0xFC00));
    if(res) return res;
    res = srcWrite(base + SRC_REG_VFREQ_FRAC, freq & 0x7FFF);
    if(res) return res;

    // reenable channel
    retry = 100;
    while((r = _inl(this->base + REG_SRC)) & SRC_RAM_BUSY && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    r &= ~(base == SRC_REG_DAC1 ? SRC_DIS_P1 : SRC_DIS_P2);
    _outl(this->base + REG_SRC, r);
    return 0;
}

void ES1371::Initialize()
{
    PCI::Lock->Acquire(0, false);
    for(PCI::Device *pciDev : *PCI::Devices)
    {
        if(pciDev->VendorID != 0x1274 || pciDev->DeviceID != 0x1371)
            continue;

        printf("[es1371] Found compatible device at PCI:%d.%d.%d\n",
               PCI_ADDR_BUS(pciDev->Address),
               PCI_ADDR_DEV(pciDev->Address),
               PCI_ADDR_FUNC(pciDev->Address));

        // enable PCI BusMaster and IO address space accesses
        PCI::WriteConfigWord(pciDev->Address | 0x04, PCI::ReadConfigWord(pciDev->Address | 0x04) | 0x0005);

        PCI::Config cfg;
        PCI::ReadConfigData(&cfg, pciDev->Address);

        ES1371 *device = new ES1371(cfg.Header.Default.BAR[0] & ~3, cfg.Header.Default.InterruptLine);
        printf("          base: %#.4x irq: %u\n", device->base, device->irq);
        AudioDevice::Add(device);
    }

    PCI::Lock->Release();
}

void ES1371::Cleanup()
{
}

ES1371::ES1371(uint16_t base, uint8_t irq) :
    base(base), irq(irq),
    interruptHandler { nullptr, interrupt, this },
    bufSem(new Semaphore(2))
{
    for(int i = 0; i < 0x10; ++i)
    {
        _outl(base + REG_PAGE, i);
        for(int j = 0; j < 0x10; j += 4)
            _outl(base + 0x30 + j, 0);
    }

    srcReset();
    codecReset();

    IRQs::RegisterHandler(irq, &interruptHandler);
    IRQs::Enable(irq);
}

const char *ES1371::GetVendor()
{
    return "Ensoniq";
}

const char *ES1371::GetModel()
{
    return "ES1371";
}

int ES1371::Open(int rate, int channels, int bits, int samples)
{
    if(rate < 4000 || rate > 48000 || channels < 1 || channels > 2 || (bits != 8 && bits != 16))
        return -EINVAL;

    // set playback rate
    int res = srcSetRate(SRC_REG_DAC2, rate);
    if(res) return res;

    // set mixer levels
    res = codecWrite(0x02, 0x0000); // master 0dB
    if(res) return res;
    res = codecWrite(0x18, 0x0808); // pcm 0dB
    if(res) return res;

    // make sure that device is not busy
    if(testAndSet(&opened))
        return -EBUSY;

    bufferSize = BUFFERS * samples * (bits / 8) * channels;
    buffer = (byte *)Paging::AllocDMA(bufferSize);
    bufferPhAddr = Paging::GetPhysicalAddress(Paging::GetAddressSpace(), (uintptr_t)buffer);

    _outl(base + REG_PAGE, 0x0C);
    _outl(base + 0x38, bufferPhAddr);
    _outl(base + 0x3C, bufferSize / sizeof(dword) - 1); // dword count

    _outl(base + REG_DAC2_SAMPLE_COUNT, samples - 1);

    int mode = (bits == 8 ? 0x00 : SERIAL_P2_S_EB) | (channels == 1 ? 0x00 : SERIAL_P2_S_MB);
    _outl(base + REG_SERIAL, SERIAL_P2_INTR_EN | mode);

    // some buzzing for testing
    for(uint i = 0; i < bufferSize; ++i)
        buffer[i] = i;

    playedBuffer = 0;
    writtenBuffer = 0;
    bufSem->Reset(BUFFERS - 1);

    return 0;
}

int ES1371::GetFrameSize()
{
    if(!opened) return -EBUSY;
    return bufferSize / BUFFERS;
}

int ES1371::Start()
{
    if(!opened) return -EBUSY;
    cpuIOSetBitsL(base + REG_CONTROL, CONTROL_DAC2_EN);
    return 0;
}

int ES1371::Stop()
{
    if(!opened) return -EBUSY;
    for(int i = 0; i < (BUFFERS - 2); ++i)
        bufSem->Wait(1000, false, false);
    cpuIOClrBitsL(base + REG_CONTROL, CONTROL_DAC2_EN);

    playedBuffer = 0;
    writtenBuffer = 0;
    bufSem->Reset(BUFFERS - 1);

    return 0;
}

int ES1371::Write(void *buffer)
{
    if(!opened) return -EBUSY;
    bool ints = cpuAreInterruptsEnabled();
    if(!bufSem->Wait(5000, false, true))
    {
        cpuRestoreInterrupts(ints);
        return -EBUSY;
    };
    byte *dst = this->buffer + (writtenBuffer % BUFFERS) * bufferSize / BUFFERS;
    memcpy(dst, buffer, bufferSize / BUFFERS);
    ++writtenBuffer;
    cpuRestoreInterrupts(ints);
    return 0;
}

void ES1371::Close()
{
    opened = false;
}

ES1371::~ES1371()
{
    bool ints = cpuDisableInterrupts();
    IRQs::UnRegisterHandler(irq, &interruptHandler);
    IRQs::TryDisable(irq);
    cpuRestoreInterrupts(ints);
    Paging::FreeDMA(buffer);
    delete bufSem;
}

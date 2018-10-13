#include <elf.h>
#include <file.h>
#include <process.h>
#include <stdio.h>
#include <string.h>

ELF::ELF(Elf32_Ehdr *ehdr, byte *phdrData, byte *shdrData, Elf32_Shdr *symtabShdr, byte *symtab, Elf32_Shdr *strtabShdr, byte *strtab) :
    ehdr(ehdr), phdrData(phdrData), shdrData(shdrData), symtabShdr(symtabShdr), symtab(symtab), strtabShdr(strtabShdr), strtab(strtab)
{
}

void ELF::Initialize(const char *kernelFile)
{
    // we need to load headers since GRUB and QEMU loader (maybe others as well) do not load them
    File *f = File::Open(kernelFile, O_RDONLY);
    if(!f)
    {
        printf("[elf] Couldn't find '%s' kernel file\n", kernelFile);
        return;
    }
    Elf32_Ehdr *ehdr = new Elf32_Ehdr;
    if(f->Read(ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        printf("[elf] Couldn't load kernel ELF header\n", kernelFile);
        delete ehdr;
        delete f;
        return;
    }
    if(ehdr->e_ident[0] != 127 || ehdr->e_ident[1] != 'E' || ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F')
    {
        printf("[elf] Invalid kernel ELF header magic\n", kernelFile);
        delete ehdr;
        delete f;
        return;
    }

    // load program headers
    if(f->Seek(ehdr->e_phoff, SEEK_SET) != ehdr->e_phoff)
    {
        printf("[elf] Couldn't seek to kernel program headers\n", kernelFile);
        delete ehdr;
        delete f;
        return;
    }
    size_t phSize = ehdr->e_phentsize * ehdr->e_phnum;
    byte *phdrData = new byte[phSize];
    if(f->Read(phdrData, phSize) != phSize)
    {
        printf("[elf] Couldn't load kernel program headers\n", kernelFile);
        delete[] phdrData;
        delete ehdr;
        delete f;
        return;
    }

    // load section headers
    if(f->Seek(ehdr->e_shoff, SEEK_SET) != ehdr->e_shoff)
    {
        printf("[elf] Couldn't seek to kernel section headers\n", kernelFile);
        delete[] phdrData;
        delete ehdr;
        delete f;
        return;
    }
    size_t shSize = ehdr->e_shentsize * ehdr->e_shnum;
    byte *shdrData = new byte[shSize];
    if(f->Read(shdrData, shSize) != shSize)
    {
        printf("[elf] Couldn't load kernel section headers\n", kernelFile);
        delete[] shdrData;
        delete[] phdrData;
        delete ehdr;
        delete f;
        return;
    }

    // load symbol table
    Elf32_Shdr *symtabShdr = nullptr;
    byte *symtab = nullptr;
    for(uint i = 0; i < ehdr->e_shnum; ++i)
    {
        Elf32_Shdr *shdr = (Elf32_Shdr *)(shdrData + i * ehdr->e_shentsize);
        if(shdr->sh_type != SHT_SYMTAB)
            continue;
        if(f->Seek(shdr->sh_offset, SEEK_SET) != shdr->sh_offset)
        {
            printf("[elf] Couldn't seek to kernel symbol table\n", kernelFile);
            break;
        }
        symtabShdr = shdr;
        symtab = new byte[shdr->sh_size];
        if(f->Read(symtab, shdr->sh_size) != shdr->sh_size)
        {
            printf("[elf] Couldn't load kernel symbol table\n", kernelFile);
            symtabShdr = nullptr;
            delete[] symtab;
            symtab = nullptr;
            break;
        }
        break;
    }

    // load string table
    Elf32_Shdr *strtabShdr = nullptr;
    byte *strtab = nullptr;
    for(uint i = 0; i < ehdr->e_shnum; ++i)
    {
        Elf32_Shdr *shdr = (Elf32_Shdr *)(shdrData + i * ehdr->e_shentsize);
        if(shdr->sh_type != SHT_STRTAB)
            continue;
        if(f->Seek(shdr->sh_offset, SEEK_SET) != shdr->sh_offset)
        {
            printf("[elf] Couldn't seek to kernel string table\n", kernelFile);
            break;
        }
        strtabShdr = shdr;
        strtab = new byte[shdr->sh_size];
        if(f->Read(strtab, shdr->sh_size) != shdr->sh_size)
        {
            printf("[elf] Couldn't load kernel string table\n", kernelFile);
            strtabShdr = nullptr;
            delete[] strtab;
            strtab = nullptr;
            break;
        }
        break;
    }

    Process *proc = Process::GetCurrent();
    proc->Image = new ELF(ehdr, phdrData, shdrData, symtabShdr, symtab, strtabShdr, strtab);
    delete f;
}

ELF *ELF::Load(char *filename)
{
    return nullptr;
}

Elf32_Sym *ELF::FindSymbol(const char *name)
{
    if(!symtabShdr || !symtab || !strtabShdr || !strtab)
        return nullptr;

    for(uint coffs = 0; coffs < symtabShdr->sh_size; coffs += symtabShdr->sh_entsize)
    {
        Elf32_Sym *sym = (Elf32_Sym *)(symtab + coffs);
        if(!sym->st_shndx || !sym->st_name)
            continue;
        if(!strcmp(name, (const char *)(strtab + sym->st_name)))
            return sym;
    }
    return nullptr;
}

ELF::~ELF()
{
    if(shdrData) delete[] shdrData;
    if(phdrData) delete[] phdrData;
    if(ehdr) delete ehdr;
}

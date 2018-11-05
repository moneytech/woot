#include <elf.h>
#include <file.h>
#include <paging.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdefs.h>

ELF::ELF(Elf32_Ehdr *ehdr, byte *phdrData, byte *shdrData, Elf32_Shdr *symtabShdr, byte *symtab, Elf32_Shdr *strtabShdr, byte *strtab, bool user) :
    ehdr(ehdr), phdrData(phdrData), shdrData(shdrData), symtabShdr(symtabShdr), symtab(symtab), strtabShdr(strtabShdr), strtab(strtab), user(user),
    EntryPoint((int (*)())ehdr->e_entry)
{
}

void ELF::Initialize(const char *kernelFile)
{
    ELF *elf = Load(nullptr, kernelFile, true, true);
    if(!elf) return;
    Process *proc = Process::GetCurrent();
    proc->Images.Append(elf);
}

ELF *ELF::Load(DEntry *dentry, const char *filename, bool user, bool onlyHeaders)
{
    File *f = dentry ? File::Open(dentry, filename, O_RDONLY) : File::Open(filename, O_RDONLY);
    if(!f)
    {
        printf("[elf] Couldn't find '%s' file\n", filename);
        return nullptr;
    }
    Elf32_Ehdr *ehdr = new Elf32_Ehdr;
    if(f->Read(ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        printf("[elf] Couldn't load ELF header\n", filename);
        delete ehdr;
        delete f;
        return nullptr;
    }
    if(ehdr->e_ident[0] != 127 || ehdr->e_ident[1] != 'E' || ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F')
    {
        printf("[elf] Invalid ELF header magic\n", filename);
        delete ehdr;
        delete f;
        return nullptr;
    }
    // load program headers
    if(f->Seek(ehdr->e_phoff, SEEK_SET) != ehdr->e_phoff)
    {
        printf("[elf] Couldn't seek to program headers\n", filename);
        delete ehdr;
        delete f;
        return nullptr;
    }
    size_t phSize = ehdr->e_phentsize * ehdr->e_phnum;
    byte *phdrData = new byte[phSize];
    if(f->Read(phdrData, phSize) != phSize)
    {
        printf("[elf] Couldn't load program headers\n", filename);
        delete[] phdrData;
        delete ehdr;
        delete f;
        return nullptr;
    }

    // load section headers
    if(f->Seek(ehdr->e_shoff, SEEK_SET) != ehdr->e_shoff)
    {
        printf("[elf] Couldn't seek to section headers\n", filename);
        delete[] phdrData;
        delete ehdr;
        delete f;
        return nullptr;
    }
    size_t shSize = ehdr->e_shentsize * ehdr->e_shnum;
    byte *shdrData = new byte[shSize];
    if(f->Read(shdrData, shSize) != shSize)
    {
        printf("[elf] Couldn't load section headers\n", filename);
        delete[] shdrData;
        delete[] phdrData;
        delete ehdr;
        delete f;
        return nullptr;
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
            printf("[elf] Couldn't seek to symbol table\n", filename);
            break;
        }
        symtabShdr = shdr;
        symtab = new byte[shdr->sh_size];
        if(f->Read(symtab, shdr->sh_size) != shdr->sh_size)
        {
            printf("[elf] Couldn't load symbol table\n", filename);
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
            printf("[elf] Couldn't seek to string table\n", filename);
            break;
        }
        strtabShdr = shdr;
        strtab = new byte[shdr->sh_size];
        if(f->Read(strtab, shdr->sh_size) != shdr->sh_size)
        {
            printf("[elf] Couldn't load string table\n", filename);
            strtabShdr = nullptr;
            delete[] strtab;
            strtab = nullptr;
            break;
        }
        break;
    }

    ELF *elf = new ELF(ehdr, phdrData, shdrData, symtabShdr, symtab, strtabShdr, strtab, user);
    Process *proc = Process::GetCurrent();

    if(!onlyHeaders && proc)
    {
        elf->process = proc;
        elf->releaseData = true;

        // load the data
        for(uint i = 0; i < ehdr->e_phnum; ++i)
        {
            Elf32_Phdr *phdr = (Elf32_Phdr *)(phdrData + ehdr->e_phentsize * i);
            if(phdr->p_type != PT_LOAD)
                continue;
            if(f->Seek(phdr->p_offset, SEEK_SET) != phdr->p_offset)
            {
                printf("[elf] Couldn't seek to data of program header %d in file '%s'\n", i, filename);
                delete elf;
                delete f;
                return nullptr;
            }

            size_t pageCount = align(phdr->p_memsz, PAGE_SIZE) / PAGE_SIZE;
            for(uint i = 0; i < pageCount; ++i)
            {
                uintptr_t va = phdr->p_vaddr + i * PAGE_SIZE;
                if(user && va >= KERNEL_BASE)
                {   // user elf can't map any kernel memory
                    printf("[elf] Invalid user address %p in file '%s'\n", va, filename);
                    delete elf;
                    delete f;
                    return nullptr;
                }
                uintptr_t pa = Paging::GetPhysicalAddress(proc->AddressSpace, va);
                if(pa != ~0)
                {
                    printf("[elf] Address conflict at %p in file '%s'\n", va, filename);
                    delete elf;
                    delete f;
                    return nullptr;
                }
                pa = Paging::AllocPage();
                if(pa == ~0)
                {
                    printf("[elf] Couldn't allocate memory for data in file '%s'\n", filename);
                    delete elf;
                    delete f;
                    return nullptr;
                }
                if(!Paging::MapPage(proc->AddressSpace, va, pa, false, user, true))
                {
                    printf("[elf] Couldn't map memory for data in file '%s'\n", filename);
                    delete elf;
                    delete f;
                    return nullptr;
                }
                elf->endPtr = max(elf->endPtr, va + PAGE_SIZE);
            }
            byte *buffer = (byte *)phdr->p_vaddr;
            memset(buffer, 0, phdr->p_memsz);
            if(f->Read(buffer, phdr->p_filesz) != phdr->p_filesz)
            {
                printf("[elf] Couldn't read data of program header %d in file '%s'\n", i, filename);
                delete elf;
                delete f;
                return nullptr;
            }
        }
        delete f;

        uintptr_t baseDelta = 0;

        auto getSHdr = [ehdr, shdrData](int i) -> Elf32_Shdr *
        {
            return (Elf32_Shdr *)(shdrData + i * ehdr->e_shentsize);
        };

        // resolve symbols and apply relocations
        for(uint i = 0; i < ehdr->e_shnum; ++i)
        {
            Elf32_Shdr *shdr = getSHdr(i);
            if(shdr->sh_type != SHT_REL) // no SHT_RELA on x86
                continue;

            byte *reltab = (byte *)(shdr->sh_addr + baseDelta);
            Elf32_Sym *_symtab = (Elf32_Sym *)(getSHdr(shdr->sh_link)->sh_addr + baseDelta);
            char *_strtab = (char *)(getSHdr(getSHdr(shdr->sh_link)->sh_link)->sh_addr + baseDelta);

            for(uint coffs = 0; coffs < shdr->sh_size; coffs += shdr->sh_entsize)
            {
                Elf32_Rel *rel = (Elf32_Rel *)(reltab + coffs);
                uint symIdx = ELF32_R_SYM(rel->r_info);
                uint rType = ELF32_R_TYPE(rel->r_info);
                Elf32_Sym *symbol = _symtab + symIdx;
                Elf32_Sym *fSymbol = nullptr;

                char *name = _strtab + symbol->st_name;
                if(name[0])
                {
                    fSymbol = proc->FindSymbol(name);
                    if(!fSymbol)
                    {
                        fSymbol = elf->FindSymbol(name);
                        if(!fSymbol)
                        {
                            printf("[elf] Unresolved symbol '%s' in '%s'\n", name, filename);
                            delete elf;
                            return nullptr;
                        }
                    }
                }

                uintptr_t *val = (uintptr_t *)(rel->r_offset + baseDelta);
                uintptr_t A = *val;
                uintptr_t B = baseDelta;
                uintptr_t P = rel->r_offset + baseDelta;
                uintptr_t S = fSymbol ? fSymbol->st_value : symbol->st_value;

                //printf("%s: rel: %d ", elf->name, rType);
                //printf("sym: %s S: %.8x A: %.8x P: %.8x\n", symbol->st_name ? name : "<no symbol>", S, A, P);

                switch(rType)
                {
                case R_386_32:
                    *val = S + A;
                    break;
                case R_386_PC32:
                    *val = S + A - P;
                    break;
                case R_386_COPY:
                    memcpy(val, (void *)S, symbol->st_size);
                    break;
                case R_386_GLOB_DAT:
                case R_386_JMP_SLOT:
                    *val = S;
                    break;
                case R_386_RELATIVE:
                    *val = B + A;
                    break;
                default:
                    printf("[elf] Unsupported relocation type: %d in '%s'\n", rType, filename);
                    delete elf;
                    return nullptr;
                }
            }
        }
    } else delete f;


    if(!user)
    {
        Elf32_Sym *cleanupSym = elf->FindSymbol("Cleanup");
        elf->CleanupProc = (void (*)())(cleanupSym ? cleanupSym->st_value : 0);
    }
    if(proc)
        proc->Images.Append(elf);
    return elf;
}

Elf32_Sym *ELF::FindSymbol(const char *name)
{
    auto getSHdr = [this](int i) -> Elf32_Shdr *{ return (Elf32_Shdr *)(shdrData + i * ehdr->e_shentsize); };
    for(uint i = 0; i < ehdr->e_shnum; ++i)
    {
        Elf32_Shdr *shdr = getSHdr(i);
        if(shdr->sh_type != SHT_DYNSYM && shdr->sh_type != SHT_SYMTAB)
            continue;
        if(!shdr->sh_addr)
            continue;
        char *strtab = (char *)(getSHdr(shdr->sh_link)->sh_addr + baseDelta);
        byte *symtab = (byte *)(shdr->sh_addr + baseDelta);
        for(uint coffs = 0; coffs < shdr->sh_size; coffs += shdr->sh_entsize)
        {
            Elf32_Sym *sym = (Elf32_Sym *)(symtab + coffs);
            //int type = ELF32_ST_TYPE(sym->st_info);
            if(!sym->st_shndx || !sym->st_name)
                continue;
            if(!strcmp(name, strtab + sym->st_name))
                return sym;
        }
    }

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

uintptr_t ELF::GetEndPtr()
{
    return endPtr;
}

ELF::~ELF()
{
    if(process)
    {
        for(uint i = 0; i < ehdr->e_phnum; ++i)
        {
            Elf32_Phdr *phdr = (Elf32_Phdr *)(phdrData + ehdr->e_phentsize * i);
            if(phdr->p_type != PT_LOAD)
                continue;
            size_t pageCount = align(phdr->p_memsz, PAGE_SIZE) / PAGE_SIZE;
            for(uint i = 0; i < pageCount; ++i)
            {
                uintptr_t va = phdr->p_vaddr + i * PAGE_SIZE;
                uintptr_t pa = Paging::GetPhysicalAddress(process->AddressSpace, va);
                if(pa == ~0)
                    continue;
                Paging::UnMapPage(process->AddressSpace, va, false);
                Paging::FreePage(pa);
            }
        }
    }

    if(strtab) delete[] strtab;
    if(symtab) delete[] symtab;
    if(shdrData) delete[] shdrData;
    if(phdrData) delete[] phdrData;
    if(ehdr) delete ehdr;
}

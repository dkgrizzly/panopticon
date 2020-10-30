/* ELF Loader
 */

#include <string.h>
#include <stdio.h>
#include "elf.h"
#include "log.h"
#include "ff.h"

/* What's our architecture code we're expecting? */
#define ARCH_CODE EM_RISCV
#define ELF_SYM_PREFIX "_"

/* Finds a given symbol in a relocated ELF symbol table */
static int find_sym(char *name, struct elf_sym_t* table, int tablelen) {
  int i;
  for (i=0; i<tablelen; i++) {
    if (!strcmp((char*)table[i].name, name))
      return i;
  }
  return -1;
}

/* Round a size to an even page count */
//int roundpages(int size) {
//  size = (size + (PAGESIZE-1)) & ~(PAGESIZE-1);
//  return size / PAGESIZE;
//}

/* Pass in a file descriptor from the virtual file system, and the
   result will be NULL if the file cannot be loaded, or a pointer to
   the loaded and relocated executable otherwise. The second variable
   will be set to the entry point. */
/* There's a lot of shit in here that's not documented or very poorly
   documented by Intel.. I hope that this works for future compilers. */
int elf_load(const char * fn, elf_prog_t * out) {
  uint8_t      *img, *imgout;
  int      sz, i, j, sect;
  struct elf_hdr_t  *hdr;
  struct elf_shdr_t  *shdrs, *symtabhdr;
  struct elf_sym_t  *symtab;
  int      symtabsize;
  struct elf_rel_t  *reltab;
  struct elf_rela_t  *relatab;
  int      reltabsize;
  char      *stringtab;
  uint32_t      vma;
  UINT    BytesReadIn;
  FIL     FileDescriptor;
  FRESULT FileError;

  /* Load the file: needs to change to just load headers */
  if ((FileError = f_open(&FileDescriptor, fn, FA_READ)) != FR_OK) {
    ELOG("elf_load: can't open input file '%s', %d\n", fn, FileError);
    return -1;
  }
  sz = f_size(&FileDescriptor);
  VLOG("Loading ELF file of size %d\n", sz);

  /* We load the raw ELF file at 8MB */
  img = (uint8_t *)0x0C800000;
  f_read(&FileDescriptor, img, sz, &BytesReadIn);
  f_close(&FileDescriptor);

  /* Header is at the front */
  hdr = (struct elf_hdr_t *)(img+0);
  if (hdr->ident[0] != 0x7f || strncmp(hdr->ident+1, "ELF", 3)) {
    ELOG("elf_load: file is not a valid ELF file\n");
    hdr->ident[4] = 0;
    ELOG("   hdr->ident is %d/%s\n", hdr->ident[0], hdr->ident+1);
    goto error1;
  }
  if (hdr->ident[4] != 1 || hdr->ident[5] != 1) {
    ELOG("elf_load: invalid architecture flags in ELF file\n");
    goto error1;
  }
  if (hdr->machine != ARCH_CODE) {
    ELOG("elf_load: unknown architecture %02x in ELF file\n", hdr->machine);
    goto error1;
  }

  /* Print some debug info */
  VLOG("File size is %d bytes\n", sz);
  VLOG("  entry point  %08lx\n", hdr->entry);
  VLOG("  ph offset    %08lx\n", hdr->phoff);
  VLOG("  sh offset    %08lx\n", hdr->shoff);
  VLOG("  flags        %08lx\n", hdr->flags);
  VLOG("  ehsize       %08x\n", hdr->ehsize);
  VLOG("  phentsize    %08x\n", hdr->phentsize);
  VLOG("  phnum        %08x\n", hdr->phnum);
  VLOG("  shentsize    %08x\n", hdr->shentsize);
  VLOG("  shnum        %08x\n", hdr->shnum);
  VLOG("  shstrndx     %08x\n", hdr->shstrndx);

  /* Locate the string table; elf files ought to have
     two string tables, one for section names and one for object
     string names. We'll look for the latter. */
  shdrs = (struct elf_shdr_t *)(img + hdr->shoff);
  stringtab = NULL;
  for (i=0; i<hdr->shnum; i++) {
    if (shdrs[i].type == SHT_STRTAB && i != hdr->shstrndx) {
      stringtab = (char*)(img + shdrs[i].offset);
    }
  }
  if (!stringtab) {
    ELOG("elf_load: ELF contains no object string table\n");
    goto error1;
  }

  /* Locate the symbol table */
  symtabhdr = NULL;
  for (i=0; i<hdr->shnum; i++) {
    if (shdrs[i].type == SHT_SYMTAB || shdrs[i].type == SHT_DYNSYM) {
      symtabhdr = shdrs+i;
      break;
    }
  }
  if (!symtabhdr) {
    ELOG("elf_load: ELF contains no symbol table\n");
    goto error1;
  }
  symtab = (struct elf_sym_t *)(img + symtabhdr->offset);
  symtabsize = symtabhdr->size / sizeof(struct elf_sym_t);

  /* Relocate symtab entries for quick access */
  for (i=0; i<symtabsize; i++)
    symtab[i].name = (uint32_t)(stringtab + symtab[i].name);

  /* Build the final memory image */
  sz = 0;
  for (i=0; i<hdr->shnum; i++) {
    if (shdrs[i].flags & SHF_ALLOC) {
      shdrs[i].addr = sz;
      sz += shdrs[i].size;
      if (shdrs[i].addralign && (shdrs[i].addr % shdrs[i].addralign)) {
        uint32_t orig = shdrs[i].addr;
        shdrs[i].addr = (shdrs[i].addr + shdrs[i].addralign)
          & ~(shdrs[i].addralign-1);
        sz += shdrs[i].addr - orig;
      }
    }
  }
  VLOG("Final image is %d bytes\n", sz);

  /* Executable Binary image gets loaded to base of RAM */
  out->data = imgout = (void *)0x0C000000;
  out->size = sz;
  vma = (uint32_t)imgout;
  for (i=0; i<hdr->shnum; i++) {
    if (shdrs[i].flags & SHF_ALLOC) {
      if (shdrs[i].type == SHT_NOBITS) {
        VLOG("  setting %d bytes of zeros at %08x\n",
          shdrs[i].size, shdrs[i].addr);
        memset(imgout+shdrs[i].addr, 0, shdrs[i].size);
      }
      else {
        VLOG("  copying %d bytes from %08x to %08x\n",
          shdrs[i].size, shdrs[i].offset, shdrs[i].addr);
        memcpy(imgout+shdrs[i].addr,
          img+shdrs[i].offset,
          shdrs[i].size);
      }
    }
  }

#if 0
  /* Go through and patch in any symbols that are undefined */
  for (i=1; i<symtabsize; i++) {
    export_sym_t * sym;

    /* VLOG(" symbol '%s': value %04lx, size %04lx, info %02x, other %02x, shndx %04lx\n",
      (const char *)(symtab[i].name),
      symtab[i].value, symtab[i].size,
      symtab[i].info,
      symtab[i].other,
      symtab[i].shndx); */
    if (symtab[i].shndx != SHN_UNDEF || ELF32_ST_TYPE(symtab[i].info) == STT_SECTION) {
      // VLOG(" symbol '%s': skipping\n", (const char *)(symtab[i].name));
      continue;
    }

    /* Find the symbol in our exports */
    sym = export_lookup((const char *)(symtab[i].name + ELF_SYM_PREFIX_LEN));
    if (!sym) {
      ELOG(" symbol '%s' is undefined\n", (const char *)(symtab[i].name));
      goto error3;
    }

    /* Patch it in */
    VLOG(" symbol '%s' patched to 0x%lx\n",
      (const char *)(symtab[i].name),
      sym->ptr);
    symtab[i].value = sym->ptr;
  }
#endif

  /* Process the relocations */
  reltab = NULL; relatab = NULL;
  for (i=0; i<hdr->shnum; i++) {
    if (shdrs[i].type != SHT_REL && shdrs[i].type != SHT_RELA) continue;

    sect = shdrs[i].info;
    VLOG("Relocating (%s) on section %d\n", shdrs[i].type == SHT_REL ? "SHT_REL" : "SHT_RELA", sect);

    switch (shdrs[i].type) {
    case SHT_RELA:
      relatab = (struct elf_rela_t *)(img + shdrs[i].offset);
      reltabsize = shdrs[i].size / sizeof(struct elf_rela_t);

      for (j=0; j<reltabsize; j++) {
        int sym;

        // XXX Does non-sh ever use RELA?
        if (ELF32_R_TYPE(relatab[j].info) != R_SH_DIR32) {
          ELOG("elf_load: ELF contains unknown RELA type %02x\n",
            ELF32_R_TYPE(relatab[j].info));
          goto error3;
        }

        sym = ELF32_R_SYM(relatab[j].info);
        if (symtab[sym].shndx == SHN_UNDEF) {
          VLOG("  Writing undefined RELA %08x(%08lx+%08lx -> %08x\n",
            symtab[sym].value + relatab[j].addend,
            symtab[sym].value,
            relatab[j].addend,
            vma + shdrs[sect].addr + relatab[j].offset);
          *((uint32_t *)(imgout
            + shdrs[sect].addr
            + relatab[j].offset))
            =    symtab[sym].value
              + relatab[j].addend;
        } else {
          VLOG("  Writing RELA %08x(%08x+%08x+%08x+%08x -> %08x\n",
            vma + shdrs[symtab[sym].shndx].addr + symtab[sym].value + relatab[j].addend,
            vma, shdrs[symtab[sym].shndx].addr, symtab[sym].value, relatab[j].addend,
            vma + shdrs[sect].addr + relatab[j].offset);
          *((uint32_t*)(imgout
            + shdrs[sect].addr    /* assuming 1 == .text */
            + relatab[j].offset))
            +=    vma
              + shdrs[symtab[sym].shndx].addr
              + symtab[sym].value
              + relatab[j].addend;
        }
      }
      break;

    case SHT_REL:
      reltab = (struct elf_rel_t *)(img + shdrs[i].offset);
      reltabsize = shdrs[i].size / sizeof(struct elf_rel_t);

      for (j=0; j<reltabsize; j++) {
        int sym, info, pcrel;

        // XXX Does non-ia32 ever use REL?
        info = ELF32_R_TYPE(reltab[j].info);
        if (info != R_386_32 && info != R_386_PC32) {
          ELOG("elf_load: ELF contains unknown REL type %02x\n", info);
          goto error3;
        }
        pcrel = (info == R_386_PC32);

        sym = ELF32_R_SYM(reltab[j].info);
        if (symtab[sym].shndx == SHN_UNDEF) {
          uint32_t value = symtab[sym].value;
          if (sect == 1 && j < 5) {
            VLOG("  Writing undefined %s %08x -> %08x",
              pcrel ? "PCREL" : "ABSREL",
              value,
              vma + shdrs[sect].addr + reltab[j].offset);
          }
          if (pcrel)
            value -= vma + shdrs[sect].addr + reltab[j].offset;

          *((uint32_t *)(imgout
            + shdrs[sect].addr
            + reltab[j].offset))
            += value;

          if (sect == 1 && j < 5) {
            VLOG("(%08x)\n", *((uint32_t *)(imgout + shdrs[sect].addr + reltab[j].offset)));
          }
        } else {
          uint32_t value = vma + shdrs[symtab[sym].shndx].addr
            + symtab[sym].value;
          if (sect == 1 && j < 5) {
            VLOG("  Writing %s %08x(%08x+%08x+%08x -> %08x",
              pcrel ? "PCREL" : "ABSREL",
              value,
              vma, shdrs[symtab[sym].shndx].addr, symtab[sym].value,
              vma + shdrs[sect].addr + reltab[j].offset);
          }
          if (pcrel)
            value -= vma + shdrs[sect].addr + reltab[j].offset;

          *((uint32_t*)(imgout
            + shdrs[sect].addr
            + reltab[j].offset))
            += value;

          if (sect == 1 && j < 5) {
            VLOG("(%08x)\n", *((uint32_t *)(imgout + shdrs[sect].addr + reltab[j].offset)));
          }
        }
      }
      break;

    }
  }

#if 0
  if (reltab == NULL && relatab == NULL) {
    ELOG("elf_load warning: found no REL(A) sections; did you forget -r?\n");
  }

  /* Look for the program entry points and deal with that */
  {
    int mainsym;

    mainsym = find_sym(ELF_SYM_PREFIX "start", symtab, symtabsize);
    if (mainsym < 0) {
      ELOG("elf_load: ELF contains no start()\n");
      goto error3;
    }

    out->start = (void (*)(void *))(vma + shdrs[symtab[mainsym].shndx].addr
      + symtab[mainsym].value);
  }
#endif

  out->start = (void (*)(void *))0x0c000000;

//  mm_pfree((void *)img, imgpages);
  VLOG("elf_load final ELF stats: memory image at %p, size %08lx\n\tentry pt %p\n", out->data, out->size, out->start);

  /* If the target CPU has I-Cache, it should be invalidated for the region just loaded. */

  return 0;

error3:
//  mm_pfree((void *)out->data, imgoutpages);

error1:
//  mm_pfree((void *)img, imgpages);
  return -1;
}

/* Free a loaded ELF program */
//void elf_free(elf_prog_t *prog) {
//  mm_pfree((void *)prog->data, prog->size/PAGESIZE);
//}

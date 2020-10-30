/* ELF Headers
 */

#ifndef __ELF_H
#define __ELF_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <stdint.h>
#include <sys/queue.h>

/* ELF file header */
struct elf_hdr_t {
  uint8_t     ident[16];  /* For elf32-shl, 0x7f+"ELF"+1+1 */
  uint16_t    type;       /* 0x02 for ET_EXEC */
  uint16_t    machine;    /* 0xf3 for elf32-riscv */
  uint32_t    version;
  uint32_t    entry;      /* Entry point */
  uint32_t    phoff;      /* Program header offset */
  uint32_t    shoff;      /* Section header offset */
  uint32_t    flags;      /* Processor flags */
  uint16_t    ehsize;     /* ELF header size in bytes */
  uint16_t    phentsize;  /* Program header entry size */
  uint16_t    phnum;      /* Program header entry count */
  uint16_t    shentsize;  /* Section header entry size */
  uint16_t    shnum;      /* Section header entry count */
  uint16_t    shstrndx;   /* String table section index */
};

/* ELF architecture types */
#define EM_RISCV  243    /* RISC-V */

/* Section header types */
#define SHT_NULL                0  /* Inactive */
#define SHT_PROGBITS            1  /* Program code/data */
#define SHT_SYMTAB              2  /* Full symbol table */
#define SHT_STRTAB              3  /* String table */
#define SHT_RELA                4  /* Relocation table, with addends */
#define SHT_HASH                5  /* Sym tab hashtable */
#define SHT_DYNAMIC             6  /* Dynamic linking info */
#define SHT_NOTE                7  /* Notes */
#define SHT_NOBITS              8  /* Occupies no space in the file */
#define SHT_REL                 9  /* Relocation table, no addends */
#define SHT_SHLIB              10  /* Invalid.. hehe */
#define SHT_DYNSYM             11  /* Dynamic-only sym tab */
#define SHT_LOPROC     0x70000000  /* Processor specific */
#define SHT_HIPROC     0x7fffffff
#define SHT_LOUSER     0x80000000  /* Program specific */
#define SHT_HIUSER     0xffffffff

/* Section header flags */
#define SHF_WRITE               1  /* Writable data */
#define SHF_ALLOC               2  /* Resident */
#define SHF_EXECINSTR           4  /* Executable instructions */
#define SHF_MASKPROC   0xf0000000  /* Processor specific */

/* Special section indeces */
#define SHN_UNDEF               0  /* Undefined, missing, irrelevant */
#define SHN_ABS            0xfff1  /* Absolute values */

/* Section header */
struct elf_shdr_t {
  uint32_t    name;    /* Index into string table */
  uint32_t    type;    /* See constants above */
  uint32_t    flags;
  uint32_t    addr;    /* In-memory offset */
  uint32_t    offset;    /* On-disk offset */
  uint32_t    size;    /* Size (if SHT_NOBITS, zero file len */
  uint32_t    link;    /* See below */
  uint32_t    info;    /* See below */
  uint32_t    addralign;  /* Alignment constraints */
  uint32_t    entsize;  /* Fixed-size table entry sizes */
};
/* Link and info fields:

switch (sh_type) {
  case SHT_DYNAMIC:
    link = section header index of the string table used by
      the entries in this section
    info = 0
  case SHT_HASH:
    ilnk = section header index of the string table to which
      this info applies
    info = 0
  case SHT_REL, SHT_RELA:
    link = section header index of associated symbol table
    info = section header index of section to which reloc applies
  case SHT_SYMTAB, SHT_DYNSYM:
    link = section header index of associated string table
    info = one greater than the symbol table index of the last
      local symbol (binding STB_LOCAL)
}

*/

#define STB_LOCAL  0
#define STB_GLOBAL  1
#define STB_WEAK  2

#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC  2
#define STT_SECTION  3
#define STT_FILE  4

/* Symbol table entry */
struct elf_sym_t {
  uint32_t    name;    /* Index into stringtab */
  uint32_t    value;
  uint32_t    size;
  uint8_t     info;    /* type == info & 0x0f */
  uint8_t     other;
  uint16_t    shndx;    /* Section index */
};
#define ELF32_ST_BIND(info)  ((info) >> 4)
#define ELF32_ST_TYPE(info)  ((info) & 0xf)

/* Relocation-A Entries */
struct elf_rela_t {
  uint32_t    offset;    /* Offset within section */
  uint32_t    info;      /* Symbol and type */
  int32_t     addend;    /* "A" constant */
};

/* Relocation Entries */
struct elf_rel_t {
  uint32_t    offset;    /* Offset within section */
  uint32_t    info;      /* Symbol and type */
};

/* Relocation-related defs */
#define R_SH_DIR32      1
#define R_386_32        1
#define R_386_PC32      2
#define ELF32_R_SYM(i)  ((i) >> 8)
#define ELF32_R_TYPE(i) ((uint8_t)(i))


/* Kernel-specific definition of a loaded ELF binary */
typedef struct elf_prog {
  void     *data;    /* Data block containing the program */
  uint32_t  size;    /* Memory image size (rounded up to page size) */
  int       argc;    /* Arguments */
  char    **argv;

  /* Program entry point */
  void  (*start)(void * pshell);

  /* Program filename */
  char  fn[256];
} elf_prog_t;

/* Load an ELF binary and return the relevant data in an elf_prog_t structure. */
int elf_load(const char *fn, elf_prog_t * out);

/* Free a loaded ELF program */
//void elf_free(elf_prog_t *prog);

__END_DECLS

#endif  /* __ELF_H */


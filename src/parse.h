#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <stdbool.h>

#include <sys/types.h>

struct elf_header{    //elf_header

  Elf64_Ehdr ehdr;    /* elf header */
  ssize_t ehdr_size;    /* size of the elf header */
};

struct phdr_table{    //phdrtab

  Elf64_Phdr *phdr_table;   /* program header table */
  uint8_t phdr_entries;    /* no of program headers */
};

struct DIE{


  Elf64_Addr addr;
  char *function_name;
};

struct SYMTAB{

  uint8_t symtab_index;    /* index of symtab section */
  uint32_t symtab_size;    /* size of symtab section */
  uint8_t symtab_entries;
  off_t symtab_offset;
  Elf64_Sym *symtab;    /* dynamic symbol table , an array of Elf64_Sym structures*/
  struct STRTAB *strtab;    /* pointer to symbol tables string table */
};

struct DBGINFO{

  bool is_exec;
  uint8_t debug_index;    /* index of debug section */
  uint32_t debug_size;    /* size of debug section */

  //  uint8_t debug_entries;    /* no of entries in debug section */

  off_t debug_offset;    /* offset of debug section */
  //we might need some additional structs to parse debug info and write a tree data structure
  char *debug_content;    /* not yet parsed data of debug section */
};

struct STRTAB{    //for sections with type strtab 

  uint8_t strtab_index;    /* index of section which holds section header name string table */
  uint32_t strtab_size;    /* string table length */ 
  uint8_t strtab_entries;     /* no of entries in string table, no of section names, or no of 
                               * section names except null section. this is same as shdr_entries - 1
                               */
  off_t strtab_offset;    /* string table offset from begining of the file */
  char *strtab_buffer;
  char **strtab_content;   /* string table data */
};

struct shdr_table{    //shdrtab

  Elf64_Shdr *shdr_table;   /* section header table */
 
  /*
   * structures containing each usefull section information
   */
  struct STRTAB shstrtab;   /* section header table string table */
  struct STRTAB dynstr;    /* dynamic section string table */
  struct STRTAB strtab;    /* string table */
  struct DBGINFO dbginfo;   /* debug info */
  struct SYMTAB dynsym;    /* dynamic symbol table */
  struct SYMTAB symtab;    /* symbol table */

  // we dont need a pointer to section header's string table because its already a member

  uint8_t shdr_entries;  /* no of section headers with that null section*/
};

typedef struct {    //data

  char *filename;   /* filename of the elf binary */
  FILE *fh;   /* file structure to keep read data */

  /*
   * elf header / header table structures
   */
  struct elf_header elf_header;
  struct phdr_table phdrtab;
  struct shdr_table shdrtab;

} ElfData;

//write some defines to indicate errors. then return that value instead of exit function.

#define NOT_FOUND   404
#define NOT_ELF     405


#define STRING_NO
#define PARSE

// intitial functions
FILE *open_file(char *filename);
int assign_headers(ElfData *data);
int check_elf(FILE *fh);
void free_all(ElfData *data);
int check_elf_type(Elf64_Ehdr *ehdr);

// get information functions
char **get_strings(struct shdr_table *shdr, int entries);
int get_section_index(struct shdr_table *shdr, char *section_name);

// program header table functions

// section header table functions
int parse_shstrtab(ElfData *data);  /* this is also an intial  function. this function is necessary to parse other information */


// string tables functions
int parse_string_table(struct shdr_table *shdr, struct STRTAB *strtab, FILE *fh, char *section_name);
int parse_dynstr(struct shdr_table *shdr, FILE *fh);
int parse_strtab(struct shdr_table *shdr, FILE *fh);

// symbol table functions
int parse_symbol_table(struct shdr_table *shdr, struct SYMTAB *symtab, FILE *fh, const char *section_name);
int parse_dynsym(struct shdr_table *shdr, FILE *fh);
int parse_symtab(struct shdr_table *shdr, FILE *fh);

#endif

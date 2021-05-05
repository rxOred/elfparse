#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

#include <sys/types.h>

struct elf_header{    //elf_header

  Elf64_Ehdr ehdr;    /* elf header */
  ssize_t ehdr_size;    /* size of the elf header */
};

struct phdr_table{    //phdrtab

  Elf64_Phdr *phdr_table;   /* program header table */
  uint8_t no_of_entries;    /* no of program headers */
};
struct DIE{


  Elf64_Addr addr;
  char *function_name;
};

struct DBGINFO{

  uint8_t debug_index;    /* index of debug section */
  uint32_t debug_size;    /* size of debug section */
  uint8_t debug_entries;    /* no of entries in debug section */
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
  struct STRTAB shstrtab;   /* string table structure */
  struct DBGINFO dbginfo;

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

#define STRING_NO
#define PARSE

FILE *open_file(char *filename);
void assign_headers(ElfData *data);
bool check_elf(FILE *fh);
void exit_code(char *msg);

#endif

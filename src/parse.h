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
  int no_of_entries;    /* no of program headers */
};

struct STRTAB{    //for sections with type strtab 

  int strtab_index;    /* index of section which holds section header name string table */
  int strtab_size;    /* string table length */ 
  int strtab_entries;     /* no of entries in string table, no of strings */
  off_t strtab_offset;    /* string table offset from begining of the file */
  char **strtab_content;   /* string table data */
};

struct shdr_table{    //shdrtab

  Elf64_Shdr *shdr_table;   /* section header table */
 
  /*
   * structures containing each usefull section information
   */
  struct STRTAB shstrtab;   /* string table structure */

  int shdr_entries;  /* no of section headers */
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

FILE *open_file(char *filename);
void assign_headers(ElfData *data);
bool check_elf(FILE *fh);
void exit_code(char *msg);

#endif

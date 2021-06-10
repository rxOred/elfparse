#ifndef PARSE_H
#define PARSE_H

#include <bits/stdint-uintn.h>
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
  uint32_t phdr_size;
  off_t phdr_offset;
};

struct COMMENT{

  uint8_t comment_index;
  uint32_t comment_size;
  uint32_t comment_entries;
  off_t comment_offset;
  char *buffer;
};

struct RELOCATION {

  uint8_t relocation_index;
  uint32_t relocation_size;
  uint32_t relocation_entries;
  off_t relocation_offset;
  Elf64_Rela *relocation;
};

struct DYNAMIC {    /* dyanamic section contains an array of Elf64_Dyn structures  */

  uint8_t dynamic_index;
  uint32_t dynamic_size;
  uint32_t dynamic_entries;
  off_t dynamic_offset;
  Elf64_Dyn *dyn;
};

struct NOTETAB{

  uint8_t notetab_index;      /* index of notetab section from section header table */
  uint32_t notetab_size;      /* size of notetab section */
  uint32_t notetab_entries;     /* number of entries in notetab section */
  off_t notetab_offset;    /* offset of the notetab section from begining of the elf binary */
  Elf64_Nhdr *notetab;    /* notetab sectiom contents, an array of type Elf64_Nhdr */
};

struct SYMTAB{

  uint8_t symtab_index;    /* index of symtab section */
  uint32_t symtab_size;    /* size of symtab section */
  uint32_t symtab_entries;
  off_t symtab_offset;
  Elf64_Sym *symtab;    /* dynamic symbol table , an array of Elf64_Sym structures*/
  struct STRTAB *strtab;    /* pointer to symbol tables string table */
};

struct STRTAB{    //for sections with type strtab 

  uint8_t strtab_index;    /* index of section which holds section header name string table */
  uint32_t strtab_size;    /* string table length */ 
  uint32_t strtab_entries;     /* no of entries in string table, no of section names, or no of 
                               * section names except null section. this is same as shdr_entries - 1
                               */
  off_t strtab_offset;    /* string table offset from begining of the file */
  char *strtab_buffer;
  /*
   * content and buffer does not points to same chunk of memory therefore buffer can be freed. its useless because we have contents.
   */
  char **strtab_content;   /* string table data */ 
};

struct shdr_table{    //shdrtab

  Elf64_Shdr *shdr_table;   /* section header table */
  uint32_t shdr_entries;  /* no of section headers with that null section*/
  uint32_t shdr_size;   /* size of section header table */
  off_t shdr_offset;   /* section header table offset */
     
  /*
   * structures containing each usefull section information
   */
  struct STRTAB shstrtab;   /* section header table string table */
  struct STRTAB dynstr;    /* dynamic section string table */
  struct STRTAB strtab;    /* string table */
  struct SYMTAB dynsym;    /* dynamic symbol table */
  struct SYMTAB symtab;    /* symbol table */
  struct NOTETAB note_abi_tag;    /* some additional note sections */
  struct NOTETAB note_gnu_property;
  struct NOTETAB note_gnu_build_id;
  struct DYNAMIC dynamic;
  struct RELOCATION rela_debug_info;
  struct RELOCATION rela_debug_arranges;
  struct RELOCATION rela_debug_line;
  struct RELOCATION rela_rodata;
  struct RELOCATION rela_text;
  struct COMMENT comment;
  
  /* TODO make a dynamic array and make sections appendable. */
  /* TODO write fucntions to append those arrays */
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

#define ___STRENT
#define ___PARSE

// intitial functions
FILE *open_file(const char *filename);
int assign_headers(ElfData *data);
int check_elf(FILE *fh);
void free_all(ElfData *data);

// get information functions
char **get_strings(struct shdr_table *shdr, int entries);
int get_section_index(struct shdr_table *shdr, const char *section_name);
int get_no_of_strings(char *buffer, int size);

// program header table functions

// section header table functions
int parse_shstrtab(ElfData *data);  /* this is also an intial  function. this function is necessary to parse other information */
uint32_t get_section_link(struct shdr_table *shdr, const char *sectionname);
uint64_t get_section_address_align(struct shdr_table *shdr, const char *sectionname);
uint32_t get_section_index_to_shstr(struct shdr_table *shdr, const char *sectionname);
uint32_t get_section_type(struct shdr_table *shdr, const char *sectionname);
uint32_t get_section_info(struct shdr_table *shdr, const char *sectionname);
uint64_t get_section_flags(struct shdr_table *shdr, const char *sectionname);
uint64_t get_section_size(struct shdr_table *shdr, const char *sectionname);
Elf64_Addr get_section_address(struct shdr_table *shdr, const char *sectionname);
uint64_t get_section_entrysize(struct shdr_table *shdr, const char *sectionname);
off_t get_section_offset(struct shdr_table *shdr, const char *sectionname);

// string tables functions
int parse_string_table(struct shdr_table *shdr, struct STRTAB *strtab, FILE *fh, char *section_name);
int parse_dynstr(struct shdr_table *shdr, FILE *fh);
int parse_strtab(struct shdr_table *shdr, FILE *fh);
void free_strtab(struct STRTAB *str);

// symbol table functions
int parse_symbol_table(struct shdr_table *shdr, struct SYMTAB *symtab, FILE *fh, const char *section_name);
int parse_dynsym(struct shdr_table *shdr, FILE *fh);
int parse_symtab(struct shdr_table *shdr, FILE *fh);
int get_symbol_index(struct SYMTAB *symtab, const char *symname);
int get_symbol_info(struct SYMTAB *symtab, const char *symname);
uint64_t get_symbol_size(struct SYMTAB *symtab, const char *symname);
Elf64_Addr get_symbol_value(struct SYMTAB *symtab, const char *symname);
int get_symbol_visibility(struct SYMTAB *symtab, const char *symname);
uint16_t get_symbol_section(struct SYMTAB *symtab, const char *symname);
void free_symtab(struct SYMTAB *sym);

//note
int parse_notetab(struct shdr_table *shdr, struct NOTETAB *note, FILE *fh, const char *sectionname);
int parse_note_gnu_build_id(struct shdr_table *shdr, FILE *fh);
int parse_note_gnu_property(struct shdr_table *shdr, FILE *fh);
int parse_note_abi_tag(struct shdr_table *shdr, FILE *fh);
Elf64_Word get_note_type(struct NOTETAB *note, int index);
Elf64_Word get_note_name_len(struct NOTETAB *note, int index);
Elf64_Word get_note_descriptor_len(struct NOTETAB *note, int index);
void free_note(struct NOTETAB *note);

//dynamic
int parse_dynamic(struct shdr_table *shdr, struct DYNAMIC *dyn, FILE *fh);
char **get_lib_list(struct shdr_table *shdr, struct DYNAMIC *dyn);
char **get_lib_search_path(struct shdr_table *shdr, struct DYNAMIC *dyn);
Elf64_Sxword get_dynamic_tag(struct DYNAMIC *dyn, int index);
Elf64_Addr get_dynamic_tag_data(struct DYNAMIC *dyn, int index);
void free_dynamic(struct DYNAMIC *dyn);


//relocation
int parse_relocation(struct shdr_table *shdr, struct RELOCATION *relocation, FILE *fh, const char *sectionname);
int parse_rela_rodata(struct shdr_table *shdr, FILE *fh);
int parse_rela_debug_info(struct shdr_table *shdr, FILE *fh);
int parse_rela_debug_arranges(struct shdr_table *shdr, FILE *fh);
int parse_rela_debug_line(struct shdr_table *shdr, FILE *fh);
int parse_rela_text(struct shdr_table *shdr, FILE *fh);
int free_relocation(struct RELOCATION *relocation);

#endif

//remove these headers
//
//#include "parse.h"
//
//make this a full fucking elf parsing, code injection, binary patching library
//
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

struct DBGINFO{

  uint8_t debug_index;    /* index of debug section */
  uint32_t debug_size;    /* size of debug section */
  uint8_t debug_entries;    /* no of entries in debug section */
  off_t debug_offset;    /* offset of debug section */
  //we might need some additional structs to parse debug info and write a tree data structure
  uint8_t *debug_content;    /* not yet parsed data of debug section */
};
struct STRTAB{    //for sections with type strtab 

  uint8_t strtab_index;    /* index of section which holds section header name string table */
  uint32_t strtab_size;    /* string table length */ 
  uint8_t strtab_entries;     /* no of entries in string table, no of strings */
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

  uint8_t shdr_entries;  /* no of section headers */
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

void free_all(ElfData *data){

  /* 
   * shstrtab contains details of section header string table. memeber 
   * strtab_content contains char * pointing to heap where name of every 
   * section is stored 
   */
  if(data->shdrtab.shstrtab.strtab_buffer)
    free(data->shdrtab.shstrtab.strtab_buffer);

  /* here we are cleaning char *, whcih are stored in heap */
  if(data->shdrtab.shstrtab.strtab_content)
    free(data->shdrtab.shstrtab.strtab_content);

  /* here we are cleaning up shdr_table (type :Elf64_shdr *) which pointes to an array of Elf64_shdr structures */
  if(data->shdrtab.shdr_table)
    free(data->shdrtab.shdr_table);

  /* here we are cleaning up phdr_table (type :Elf64_phdr *) which points to an array of Elf64_phdr structures */
  if(data->phdrtab.phdr_table)
    free(data->phdrtab.phdr_table);
}

void exit_code(char *msg, int code){

  puts(msg);
  exit(code);
}

int get_section_index(struct shdr_table *shdr, char *section_name){

  int i;
  for (i = 0; i < shdr->shstrtab.strtab_entries; i++){

    if(shdr->shdr_table[i].sh_name < shdr->shstrtab.strtab_size){
    
      char *buf = &shdr->shstrtab.strtab_buffer[shdr->shdr_table[i].sh_name];
      if((buf != NULL))

        if(strcmp(buf, section_name) == 0){
        
          printf("%s\n", buf);
          return i;
      }
    }
  }
  return -1;
}

void parse_dwarf_info(struct shdr_table *shdr, FILE *fh){

  /* 
   * we use get section index function to get section header index of debug_info section
   * this function can be used for any section header
   */
  int index = get_section_index(shdr, ".symtab");

  shdr->dbginfo.debug_index = index;
  
  /* 
   * we then use that index value to access section header table values. 
   * we assign debug offset to sh_offset of .debug_info section header structure
   */
  shdr->dbginfo.debug_offset = shdr->shdr_table[index].sh_offset;

  /*
   * we seek to that location of our binary file
   */
  fseek(fh, shdr->dbginfo.debug_offset, SEEK_SET);

  /*
   * we use same index to get size of the section so that we can read it to our buffer
   */
  shdr->dbginfo.debug_size = shdr->shdr_table[index].sh_size;

  printf("%d\n", shdr->dbginfo.debug_size);

  /* 
   * we use previous retrieved size to allocate chunk of memory in head so we can read section contents to it
   */
  shdr->dbginfo.debug_content = calloc(sizeof(uint8_t), shdr->dbginfo.debug_size);
  if(!shdr->dbginfo.debug_content) exit_code("calloc failed", EXIT_FAILURE);

  if(fread(shdr->dbginfo.debug_content, 1, shdr->dbginfo.debug_size, fh) < shdr->dbginfo.debug_size)
    exit_code("fread failed", EXIT_FAILURE);

  for (int i = 0; i < (int) shdr->dbginfo.debug_size; i++)
    printf("%c",shdr->dbginfo.debug_content[i]);
  
  putchar('\n');
}

char **parse_strings(struct shdr_table *shdr, int entries){

  char **parsed_string = calloc(sizeof(char *), entries);
  if(!parsed_string) return NULL;
  
  for (int i = 0; i < shdr->shstrtab.strtab_entries; i++){

    char *buf = &shdr->shstrtab.strtab_buffer[shdr->shdr_table[i].sh_name];
    parsed_string[i] = buf;
  }

#ifndef PARSE

  int entry_i = 0;
  for (int i = 0; i < size; ++i){

    if(buffer[i] != '\0'){

      parsed_string[entry_i] = &buffer[i];
      while(buffer[i] != '\0') ++i; 
      ++entry_i;
    }
    else
      while(buffer[i] == '\0') i++;
  }
#endif
  return parsed_string;
}

void print_section_names(ElfData *data){

  /*get the section header table index to get the string table. ehdr.e_shstrndx containts the index of shstrtab table*/
  data->shdrtab.shstrtab.strtab_index = data->elf_header.ehdr.e_shstrndx;

  /*use section header index to access access corresponding section;'s shdr structure and use sh_offset to get the offset of the string table */
  data->shdrtab.shstrtab.strtab_offset = data->shdrtab.shdr_table[data->shdrtab.shstrtab.strtab_index].sh_offset;
  
  /* seek to that offset */
  fseek(data->fh, data->shdrtab.shstrtab.strtab_offset, SEEK_SET);
  
  /* set size of the  string table using sh_size */
  data->shdrtab.shstrtab.strtab_size = data->shdrtab.shdr_table[data->shdrtab.shstrtab.strtab_index].sh_size;

  /* allocate a buffer with size sh_size, then read shstrtab into that buffer */
  data->shdrtab.shstrtab.strtab_buffer = malloc(data->shdrtab.shstrtab.strtab_size);
  if(!data->shdrtab.shstrtab.strtab_buffer) exit_code("malloc failed", EXIT_FAILURE);
  
  if(fread(data->shdrtab.shstrtab.strtab_buffer, 1, data->shdrtab.shstrtab.strtab_size, data->fh) < (unsigned long)data->shdrtab.shstrtab.strtab_size) 
    exit_code("fread failed", EXIT_FAILURE);

  /* 
   * after reading, we need tp get the exact number of strings in that table. this is also possible using something like 
   * e_shnum, which gives us number of sections. number of sections == no of strings in shstrtab
   */

#ifdef STRING_NO
  data->shdrtab.shstrtab.strtab_entries = data->shdrtab.shdr_entries;
#else
  data->shdrtab.shstrtab.strtab_entries = get_no_of_strings(data->shdrtab.shstrtab.strtab_buffer, data->shdrtab.shstrtab.strtab_size); 
#endif

  printf("%d\t%d\n", data->shdrtab.shstrtab.strtab_size, data->shdrtab.shstrtab.strtab_entries);

  /* read to (char *) buffer , sh_size of bytes from current position (section .strtab offset)) */
  data->shdrtab.shstrtab.strtab_content = parse_strings(&data->shdrtab, data->shdrtab.shstrtab.strtab_entries); 

  for (int i = 0; i < data->shdrtab.shstrtab.strtab_entries; i++){

    printf("%d %s\n", i, data->shdrtab.shstrtab.strtab_content[i]);
  }

}

bool check_elf(FILE *fh){

  uint8_t buf[4];
  
  fread(buf, sizeof(uint8_t), 3, fh);
  
  if(buf[0] == EI_MAG0 && buf[1] == EI_MAG1 && buf[2] == EI_MAG2 && buf[3] == EI_MAG3) return true;

  return false;
}

void assign_headers(ElfData *data){

  //there is some error with check_elf function
  //if(!check_elf(data->fh)) exit_code("file is not an elf binary");
  
  /* assigning ehdr with data read from the binary */
  data->elf_header.ehdr_size = sizeof(Elf64_Ehdr);
  if(fread(&data->elf_header.ehdr, data->elf_header.ehdr_size, 1, data->fh) < 1) 
    exit_code("unable to read file", EXIT_FAILURE);
  

  /* assigning phdr with data read from binary using ehdr->e_phoff */ 
  data->phdrtab.phdr_entries = data->elf_header.ehdr.e_phnum;
  
  data->phdrtab.phdr_table = calloc(data->phdrtab.phdr_entries, sizeof(Elf64_Phdr));
  if(!data->phdrtab.phdr_table) exit_code("calloc failed", EXIT_FAILURE);

  fseek(data->fh, data->elf_header.ehdr.e_phoff, SEEK_SET); //seeking file pointer to position where program header table begins

  if(fread(data->phdrtab.phdr_table, sizeof(Elf64_Phdr), data->phdrtab.phdr_entries, data->fh) < (unsigned long)data->phdrtab.phdr_entries) //reading from that location to allocated space in heap
    exit_code("could not read program header table", EXIT_FAILURE);


  /* assigning shdr with data read from binary using ehdr->e_shoff */
  data->shdrtab.shdr_entries = data->elf_header.ehdr.e_shnum;

  printf("total entries %d\n", data->shdrtab.shdr_entries);
  data->shdrtab.shdr_table = calloc(data->shdrtab.shdr_entries, sizeof(Elf64_Shdr));
  if(!data->shdrtab.shdr_table) exit_code("calloc failed", EXIT_FAILURE);

  fseek(data->fh, data->elf_header.ehdr.e_shoff, SEEK_SET);

  if(fread(data->shdrtab.shdr_table, sizeof(Elf64_Shdr), data->shdrtab.shdr_entries, data->fh) < (unsigned long)data->shdrtab.shdr_entries)
    exit_code("could not read section header table", EXIT_FAILURE);
}

FILE *open_file(char *filename){

  FILE *local = fopen(filename, "rb");
  if(!local)
    exit_code("coud not open binary file", EXIT_FAILURE);

  return local;
}

int main(void){

  ElfData data;
  data.filename = "dbg";
  printf("%s\n", data.filename);

  data.fh = open_file(data.filename);
  assign_headers(&data);

  print_section_names(&data);
  parse_dwarf_info(&data.shdrtab, data.fh);

  free_all(&data);
  return 0;
}

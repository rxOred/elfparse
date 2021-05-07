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

struct SYMTAB{

  uint8_t symtab_index;
  uint32_t symtab_size;
  uint8_t symtab_entries;
  off_t symtab_offset;
  Elf64_Sym *symtab;    /* dynamic symbol table */
  struct STRTAB *strtab;    /* pointer to symbol tables string table */
};

struct DBGINFO{

  uint8_t debug_index;    /* index of debug section */
  uint32_t debug_size;    /* size of debug section */
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
  struct STRTAB dynstr;
  struct DBGINFO dbginfo;
  struct SYMTAB dynsym;

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


#define NOT_FOUND   404
#define NOT_ELF     405
#define NOT_EXEC    406
#define NOT_REL     407

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

  //free debug info
  //free symtab

}

int get_section_index(struct shdr_table *shdr, char *section_name){

  int i;
  for (i = 0; i < shdr->shstrtab.strtab_entries; i++){

    if(shdr->shdr_table[i].sh_name < shdr->shstrtab.strtab_size){
    
      char *buf = &shdr->shstrtab.strtab_buffer[shdr->shdr_table[i].sh_name];
      if((buf != NULL))

        if(strcmp(buf, section_name) == 0){
          return i;
      }
    }
  }
  return NOT_FOUND;
}

/*
 * parses dwarf info section and stores information is struct DEBUGINFO dbginfo structure.
 * members of this struct can be used to parse a dwarf tree in future
 */
int parse_dwarf_info(struct shdr_table *shdr, FILE *fh){

  /* 
   * we use get section index function to get section header index of debug_info section
   * this function can be used for any section header
   */
  int index = get_section_index(shdr, ".debug_info");
  if(index == NOT_FOUND) return NOT_FOUND;

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

  /* 
   * we use previous retrieved size to allocate chunk of memory in head so we can read section contents to it
   */
  shdr->dbginfo.debug_content = calloc(sizeof(uint8_t), shdr->dbginfo.debug_size);
  if(!shdr->dbginfo.debug_content){

    perror("parser : calloc failed");
    return -1;
  }

  if(fread(shdr->dbginfo.debug_content, 1, shdr->dbginfo.debug_size, fh) < shdr->dbginfo.debug_size){

    perror("parser fread failed");
    return -1;
  }

  /*for (int i = 0; i < (int) shdr->dbginfo.debug_size; i++)
    printf("%c",shdr->dbginfo.debug_content[i]);
  
  putchar('\n');*/
  return 0;
}

/*
 * string table
 */
int parse_strtab(struct shdr_table *shdr, struct STRTAB *strtab, FILE *fh, char *section_name){

  strtab->strtab_index = get_section_index(shdr, section_name);
  if(strtab->strtab_index == (uint8_t)NOT_FOUND) return NOT_FOUND;

  strtab->strtab_entries = (uint8_t) UN; //temp
  
  strtab->strtab_offset = shdr->shdr_table[strtab->strtab_index].sh_offset;
  strtab->strtab_size = shdr->shdr_table[strtab->strtab_index].sh_offset;

  fseek(fh, strtab->strtab_offset, SEEK_SET);

  strtab->strtab_buffer = calloc(sizeof(char), strtab->strtab_size);
  if(!strtab->strtab_buffer){

    perror("parser: calloc failed");
    return -1;
  }
  if(fread(strtab->strtab_buffer, sizeof(char), strtab->strtab_size, fh) < strtab->strtab_size){

    perror("parser fread failed");
    return -1;
  }

  return 0;
}

int parse_dynstr(struct shdr_table *shdr, FILE *fh){

  int ret = parse_strtab(shdr, &shdr->dynstr, fh, ".dynsym");
  if(ret == NOT_FOUND){

    puts("section .dynsym not found");
    return -1;
  }else if(ret == -1) return -1;

  return 0;
}
/*
 * assigns dyanamic symbol table
 */
int parse_dynsym(struct shdr_table *shdr, FILE *fh){

  shdr->dynsym.symtab_index = get_section_index(shdr, ".dynsym");
  if(shdr->dynsym.symtab_index == (uint8_t)NOT_FOUND) return NOT_FOUND; 

  shdr->dynsym.symtab_entries = shdr->shdr_table[shdr->dynsym.symtab_index].sh_entsize;
  shdr->dynsym.symtab_size = shdr->shdr_table[shdr->dynsym.symtab_index].sh_size;
  shdr->dynsym.symtab_offset = shdr->shdr_table[shdr->dynsym.symtab_index].sh_offset;
  
  fseek(fh, shdr->dynsym.symtab_offset, SEEK_SET);
  if(fread(shdr->dynsym.symtab, 1, shdr->dynsym.symtab_size, fh) < shdr->dynsym.symtab_size){

    perror("parser: ");
    return -1;
  }
  return 0;
}

char **get_strings(struct shdr_table *shdr, int entries){

  char **parsed_string = calloc(sizeof(char *), entries);
  if(!parsed_string) return NULL;
  
  for (int i = 0; i < shdr->shstrtab.strtab_entries; i++){

    char *buf = &shdr->shstrtab.strtab_buffer[shdr->shdr_table[i].sh_name];
    parsed_string[i] = buf;
  }

  return parsed_string;
}

/*
 * this function uses shdr table to parse section names. it stores parsed strings in shstrtab 
 * structure's strtab_content memeber. this memeber can be used when printing section names and so on
 * get_section_index function uses values assigned by this function to track sections.
 */
int parse_shstrtab(ElfData *data){ //should have used struct shdr_table *

  /*get the section header table index to get the string table. ehdr.e_shstrndx containts the index of shstrtab table*/
  data->shdrtab.shstrtab.strtab_index = data->elf_header.ehdr.e_shstrndx;

  /*use section header index to access access corresponding section;'s shdr structure and use sh_offset to get the offset of the string table */
  data->shdrtab.shstrtab.strtab_offset = data->shdrtab.shdr_table[data->shdrtab.shstrtab.strtab_index].sh_offset;
  
  /* seek to that offset */
  fseek(data->fh, data->shdrtab.shstrtab.strtab_offset, SEEK_SET);
  
  /* set size of the  string table using sh_size */
  data->shdrtab.shstrtab.strtab_size = data->shdrtab.shdr_table[data->shdrtab.shstrtab.strtab_index].sh_size;

  /* allocate a buffer with size sh_size, then read shstrtab into that buffer */
  data->shdrtab.shstrtab.strtab_buffer = calloc(data->shdrtab.shstrtab.strtab_size, sizeof(char));
  if(!data->shdrtab.shstrtab.strtab_buffer){
  
    perror("parser malloc failed");
    return -1;
  }
  
  if(fread(data->shdrtab.shstrtab.strtab_buffer, 1, data->shdrtab.shstrtab.strtab_size, data->fh) < (unsigned long)data->shdrtab.shstrtab.strtab_size){

    perror("fread failed");
    return -1;
  }
  /* 
   * after reading, we need tp get the exact number of strings in that table. this is also possible using something like 
   * e_shnum, which gives us number of sections. number of sections == no of strings in shstrtab
   */

#ifdef STRING_NO
  data->shdrtab.shstrtab.strtab_entries = data->shdrtab.shdr_entries;
#else
  data->shdrtab.shstrtab.strtab_entries = get_no_of_strings(data->shdrtab.shstrtab.strtab_buffer, data->shdrtab.shstrtab.strtab_size); 
#endif
 
  /* read to (char *) buffer , sh_size of bytes from current position (section .strtab offset)) */
  data->shdrtab.shstrtab.strtab_content = get_strings(&data->shdrtab, data->shdrtab.shstrtab.strtab_entries);

  return 0;
}

int check_elf(FILE *fh){

  uint8_t buf[4];
 
  fseek(fh, 0, SEEK_SET);
  if(fread(buf, sizeof(uint8_t), 4, fh) < 4){
    
    perror("check elf");
    return -1;
  };
  
  if(buf[0] != EI_MAG0 || buf[1] != EI_MAG1 || buf[2] != EI_MAG2 || buf[3] != EI_MAG3) return 0;

  return NOT_ELF;
}

int check_elf_type(Elf64_Ehdr *ehdr){

  return ehdr->e_type;
}

/*
 * this function reads from the elf file and assign ehdr header and shdr, phdr tables
 */
int assign_headers(ElfData *data){

  int ret = check_elf(data->fh);
  if(ret == -1) return -1;
  else if(ret == NOT_ELF) {

    puts("file is not an elf binary"); 
    return -1;
  }
  
  /* assigning ehdr with data read from the binary */
  data->elf_header.ehdr_size = sizeof(Elf64_Ehdr);
  if(fread(&data->elf_header.ehdr, data->elf_header.ehdr_size, 1, data->fh) < 1){
  
    perror("unable to read file");
    return -1;
  }
  
  /* assigning phdr with data read from binary using ehdr->e_phoff */ 
  int elf_type = check_elf_type(&data->elf_header.ehdr);
  if(elf_type == ET_DYN || elf_type == ET_EXEC){

    data->phdrtab.phdr_entries = data->elf_header.ehdr.e_phnum;
  
    data->phdrtab.phdr_table = calloc(data->phdrtab.phdr_entries, sizeof(Elf64_Phdr));
    if(!data->phdrtab.phdr_table) {
   
      perror("calloc failed");
      return -1;
    }
    fseek(data->fh, data->elf_header.ehdr.e_phoff, SEEK_SET); //seeking file pointer to position where program header table begins

    if(fread(data->phdrtab.phdr_table, sizeof(Elf64_Phdr), data->phdrtab.phdr_entries, data->fh) < (unsigned long)data->phdrtab.phdr_entries){ //reading from that location to allocated space in heap
      perror("could not read program header table");
      return -1;
    }
  }
  else{
  
    puts("binary file is not an executable file");
  }

  /* assigning shdr with data read from binary using ehdr->e_shoff */
  data->shdrtab.shdr_entries = data->elf_header.ehdr.e_shnum;

  data->shdrtab.shdr_table = calloc(data->shdrtab.shdr_entries, sizeof(Elf64_Shdr));
  if(!data->shdrtab.shdr_table) {

    perror("calloc failed");
    return -1;
  }

  fseek(data->fh, data->elf_header.ehdr.e_shoff, SEEK_SET);

  if(fread(data->shdrtab.shdr_table, sizeof(Elf64_Shdr), data->shdrtab.shdr_entries, data->fh) < (unsigned long)data->shdrtab.shdr_entries){
  
    perror("could not read section header table");
    return -1;
  }

  return 0;
}

FILE *open_file(char *filename){

  FILE *local = fopen(filename, "rb");
  if(!local){

    perror("coud not open binary file");
    return NULL;
  }

  return local;
}

int main(void){

  ElfData data;
  data.filename = "dbg";
  printf("filename : %s\n", data.filename);

  data.fh = open_file(data.filename);
  assign_headers(&data);
  printf("total entries %d\n", data.shdrtab.shdr_entries);

  parse_shstrtab(&data);
  printf("size of shstrtab :%d\t no of shstrtab entries :%d\n", data.shdrtab.shstrtab.strtab_size, data.shdrtab.shstrtab.strtab_entries);
  for (int i = 0; i < data.shdrtab.shstrtab.strtab_entries; i++){

    printf("%d\t%s\n", i, data.shdrtab.shstrtab.strtab_content[i]);
  }
  parse_dynsym(&data.shdrtab, data.fh);
  parse_dynstr(&data.shdrtab, data.fh);
  parse_dwarf_info(&data.shdrtab, data.fh);

  free_all(&data);
  return 0;
}

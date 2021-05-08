#include "parse.h"

/*
 * this section of library need a loaded process to work with. for example, if you want to parse dwarf information,
 * you will have to implement a debugger or some kind of application which exec the elf binary, which means,
 * only shared and exec files are compatible of these features
 */
 
 

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
#include "parse.h"
/*
 * string table
 */
int parse_string_table(struct shdr_table *shdr, struct STRTAB *strtab, FILE *fh, char *section_name){

  strtab->strtab_index = get_section_index(shdr, section_name);
  if(strtab->strtab_index == (uint8_t)NOT_FOUND) return NOT_FOUND;

  strtab->strtab_entries = (uint8_t) UN; //temp
  
  strtab->strtab_offset = shdr->shdr_table[strtab->strtab_index].sh_offset;
  strtab->strtab_size = shdr->shdr_table[strtab->strtab_index].sh_offset;

  fseek(fh, strtab->strtab_offset, SEEK_SET);

  strtab->strtab_buffer = calloc(strtab->strtab_size, sizeof(char));
  if(!strtab->strtab_buffer){

    perror("parser: calloc failed");
    return -1;
  }
  if(fread(strtab->strtab_buffer, sizeof(char), strtab->strtab_size, fh) < strtab->strtab_size){

    perror("parser fread failed");
    return -1;
  }

  strtab->strtab_content = get_strings(strtab, strtab->strtab_entries);
  
  return 0;
}

/*
 * for dynamic section's string table
 */
int parse_dynstr(struct shdr_table *shdr, FILE *fh){

  int ret = parse_string_table(shdr, &shdr->dynstr, fh, ".dynstr");
  if(ret == NOT_FOUND){

    puts("section .dynstr not found");
    return -1;
  }else if(ret == -1) return -1;

  shdr->dynsym.strtab = &shdr->dynstr;
  return 0;
}

/*
 * for main string table
 */
int parse_strtab(struct shdr_table *shdr, FILE *fh){

  int ret = parse_string_table(shdr, &shdr->strtab, fh, ".strtab");
  if(ret == NOT_FOUND){

    puts("section .strtab not found");
    return -1;
  }else if(ret == -1) return -1;

  /*
   * pointing / linking symbol table and newly parsed string table
   */
  shdr->symtab.strtab = &shdr->strtab;
}
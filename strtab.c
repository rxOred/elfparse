#include "parse.h"
#include <stdlib.h>
#include <assert.h>

void free_strtab(struct STRTAB *str){

  if(str->strtab_buffer)
    free(str->strtab_buffer);

  if(str->strtab_content)
    free(str->strtab_content);
}

/*
 * string table
 */
char **get_strtab_contents(struct SYMTAB *symtab, char *buffer, int entries){

  assert(symtab != NULL);
  assert(buffer != NULL);

  if(!buffer){

    fprintf(stderr, "buffer is empty. could not parse contents");
  }

  char **parsed_strings = calloc(sizeof(char *), entries);
  if(!parsed_strings){

    perror("memory allocation failed");
    return NULL;
  }

  for (int i = 0; i < entries; i++){

    char *buf = &buffer[symtab->symtab[i].st_name];
    parsed_strings[i] = buf;
  }

  return parsed_strings;
}

int parse_string_table(struct shdr_table *shdr, struct STRTAB *strtab, FILE *fh, char *section_name){

  assert(fh != NULL);
  assert(shdr != NULL);
  assert(strtab != NULL);
  assert(section_name);

  strtab->strtab_index = get_section_index(shdr, section_name);
  if(strtab->strtab_index == (uint8_t)NOT_FOUND) return NOT_FOUND;

  strtab->strtab_offset = shdr->shdr_table[strtab->strtab_index].sh_offset;
  strtab->strtab_size = shdr->shdr_table[strtab->strtab_index].sh_size;

  fseek(fh, strtab->strtab_offset, SEEK_SET);

  strtab->strtab_buffer = malloc(strtab->strtab_size);
  if(!strtab->strtab_buffer){

    perror("memory allocation failed");
    return -1;
  }

  memset(strtab->strtab_buffer, 0, strtab->strtab_size);

  if(fread(strtab->strtab_buffer, sizeof(char), strtab->strtab_size, fh) < strtab->strtab_size){

    perror("failed to read file");
    return -1;
  }

  strtab->strtab_entries = get_no_of_strings(strtab->strtab_buffer, strtab->strtab_size);

  return 0;
}

/*
 * for dynamic section's string table
 */
int parse_dynstr(struct shdr_table *shdr, FILE *fh){

  assert(fh != NULL);
  assert(shdr != NULL);
  if(!shdr->dynsym.strtab)
    shdr->dynsym.strtab = &shdr->dynstr;

  shdr->dynstr.strtab_buffer = NULL;
  shdr->dynstr.strtab_content = NULL;

  int ret = parse_string_table(shdr, &shdr->dynstr, fh, ".dynstr");
  if(ret == NOT_FOUND){

    fprintf(stderr, "section .dynstr not found\n");
    return -1;
  }else if(ret == -1) return -1;

  shdr->strtab.strtab_content = get_strtab_contents(&shdr->dynsym, shdr->dynstr.strtab_buffer, shdr->dynstr.strtab_entries);
  if(!shdr->dynstr.strtab_content)
    return -1;

  return 0;
}

/*
 * for main string table
 */
int parse_strtab(struct shdr_table *shdr, FILE *fh){

  assert(fh != NULL);
  assert(shdr != NULL);
  if(!shdr->symtab.strtab)
    shdr->symtab.strtab = &shdr->strtab;

  shdr->strtab.strtab_buffer = NULL;
  shdr->strtab.strtab_content = NULL;

  int ret = parse_string_table(shdr, &shdr->strtab, fh, ".strtab");
  if(ret == NOT_FOUND){

    fprintf(stderr, "section .strtab not found\n");
    return -1;
  }else if(ret == -1) return -1;

  shdr->strtab.strtab_content = get_strtab_contents(&shdr->symtab, shdr->strtab.strtab_buffer, shdr->strtab.strtab_entries);
  if(!shdr->strtab.strtab_content)
    return -1;

  return 0;
}

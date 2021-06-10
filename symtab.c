#include "parse.h"
#include <stdio.h>
#include <stdlib.h>

void free_symtab(struct SYMTAB *sym){

  if(sym->symtab)
    free(sym->symtab);
}

int get_symbol_index(struct SYMTAB *symtab, const char *symname){

  for(int i = 0; i < symtab->symtab_entries; i++){

    if(strcmp(symtab->strtab->strtab_content[symtab->symtab[i].st_name], symname) == 0)
      return i;
  }
  return NOT_FOUND;
}

int get_symbol_info(struct SYMTAB *symtab, const char *symname){

  int index = get_symbol_index(symtab, symname);
  if(index == NOT_FOUND){

    fprintf(stderr, "symbol %s not found\n", symname);
    return -1;
  }
  return symtab->symtab[index].st_info;
}

uint64_t get_symbol_size(struct SYMTAB *symtab, const char *symname){

  int index = get_symbol_index(symtab, symname);
  if(index == NOT_FOUND){

    fprintf(stderr, "symbol %s not found\n", symname);
    return -1;
  }
  return symtab->symtab[index].st_size;
}

Elf64_Addr get_symbol_value(struct SYMTAB *symtab, const char *symname){

  int index = get_symbol_index(symtab, symname);
  if(index == NOT_FOUND){

    fprintf(stderr, "symbol %s not found\n", symname);
    return -1;
  }
  return symtab->symtab[index].st_value;
}

int get_symbol_visibility(struct SYMTAB *symtab, const char *symname){

  int index = get_symbol_index(symtab, symname);
  if(index == NOT_FOUND){

    fprintf(stderr, "symbol %s not found\n", symname);
    return -1;
  }
  return symtab->symtab[index].st_other;
}

uint16_t get_symbol_section(struct SYMTAB *symtab, const char *symname){

  int index = get_symbol_index(symtab, symname);
  if(index == NOT_FOUND){

    fprintf(stderr, "symbol %s not found\n", symname);
    return -1;
  }
  return symtab->symtab[index].st_shndx;
}

int parse_symbol_table(struct shdr_table *shdr, struct SYMTAB *symtab, FILE *fh, const char *section_name){

  symtab->symtab_index = get_section_index(shdr, section_name);
  if(symtab->symtab_index == (uint8_t)NOT_FOUND) return NOT_FOUND;

  symtab->symtab_size = shdr->shdr_table[symtab->symtab_index].sh_size;
  symtab->symtab_entries = (symtab->symtab_size / shdr->shdr_table[symtab->symtab_index].sh_entsize);
  symtab->symtab_offset = shdr->shdr_table[symtab->symtab_index].sh_offset;

  //use malloc and memset
  symtab->symtab = malloc(symtab->symtab_size);
  if(!symtab->symtab){

    perror("memory allocation failed");
    return -1;
  }

  memset(symtab->symtab, 0, symtab->symtab_size);

  fseek(fh, symtab->symtab_offset, SEEK_SET);
  if(fread(symtab->symtab, 1, symtab->symtab_size, fh) < symtab->symtab_size){

    perror("error reading file");
    return -1;
  }

  return 0;
}

int parse_dynsym(struct shdr_table *shdr, FILE *fh){

  shdr->dynsym.symtab = NULL;
  int ret = parse_symbol_table(shdr, &shdr->dynsym, fh, ".dynsym");
  if(ret == NOT_FOUND){
  
    fprintf(stderr, "Section .dynsym not found\n");
    return -1;
  }
  else if(ret == -1) return -1;

  return 0;
}

int parse_symtab(struct shdr_table *shdr, FILE *fh){

  shdr->symtab.symtab = NULL;
  int ret = parse_symbol_table(shdr, &shdr->symtab, fh, ".symtab");
  if(ret == NOT_FOUND){

    fprintf(stderr, "section .symtab not found\n");
    return -1;
  }
  else if(ret == -1) return -1;

  return 0;
}

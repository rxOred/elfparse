#include "parse.h"

int parse_symbol_table(struct shdr_table *shdr, struct SYMTAB *symtab, FILE *fh, const char *section_name){

  symtab->symtab_index = get_section_index(shdr, section_name);
  if(symtab->symtab_index = (uint8_t)NOT_FOUND) return NOT_FOUND;

  symtab->symtab_entries = shdr->shdr_table[symtab->symtab_index].sh_entsize;
  symtab->symtab_size = shdr->shdr_table[symtab->symtab_index].sh_size;
  symtab->symtab_offset = shdr->shdr_table[symtab->symtab_index].sh_offset;
 
  symtab->symtab = calloc(symtab->symtab_entries, sizeof(Elf64_Sym));
  if(!symtab->symtab){

    perror("calloc failed");
    return -1;
  }

  fseek(fh, symtab->symtab_offset, SEEK_SET);
  if(fread(symtab->symtab, 1, symtab->symtab_size, fh) < symtab->symtab_size){

    perror("parser: ");
    return -1;
  }
  return 0;

}

int parse_dynsym(struct shdr_table *shdr, FILE *fh){

  int ret = parse_symbol_table(shdr, &shdr->dynsym, fh, ".dynsym");
  if(ret == NOT_FOUND){
  
    puts("Section not found");
    return -1;
  }
  else if(ret == -1) return -1;

  return 0;
}

int parse_symtab(struct shdr_table *shdr, FILE *fh){

  int ret = parse_symbol_table(shdr, &shdr->symtab, fh, ".symtab");
  if(ret == NOT_FOUND){

    puts("section .symtab not found");
    return -1;
  }else if(ret == -1) return -1;

  return 0;
}

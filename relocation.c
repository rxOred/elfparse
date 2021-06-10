#include <elf.h>
#include <stdlib.h>
#include "parse.h"

void free_relocation(struct RELOCATION *relocation){

  if(relocation->relocation)
    free(relocation->relocation);
}

/* retrieve address relocation should be affected */
Elf64_Addr get_relocation_offset(struct RELOCATION *relocation, int index){

  return relocation->relocation[index].r_offset;
}

/*  retrieve symbol table index with respect to which the relocation must be made */
Elf64_Word get_relocation_info(struct RELOCATION *relocation, int index){ //can be used when recreating symbols and stuff

  return relocation->relocation[index].r_info;
}

Elf64_Sword get_relocation_addend(struct RELOCATION *relocation, int index){

  return relocation->relocation[index].r_addend;
}

int parse_relocation(struct shdr_table *shdr, struct RELOCATION *relocation, FILE *fh, const char *sectionname){

  relocation->relocation_index = get_section_index(shdr, sectionname);
  if(relocation->relocation_index < 0)
    return -1;
  
  relocation->relocation_offset = shdr->shdr_table[relocation->relocation_index].sh_offset;
  relocation->relocation_size = shdr->shdr_table[relocation->relocation_index].sh_size;
  relocation->relocation_entries = relocation->relocation_size / sizeof(Elf64_Dyn);

  if(fseek(fh, relocation->relocation_offset, SEEK_SET) < 0) return -1;

  relocation->relocation = malloc(relocation->relocation_size);
  if(!relocation->relocation){

    perror("memory allocation error");
    return -1;
  }

  memset(relocation->relocation, 0, relocation->relocation_size);

  if(fread(relocation->relocation, 1, relocation->relocation_size, fh) < 0){

    perror("failed to read file");
    return -1;
  }

  return 0;
}

int parse_rela_debug_info(struct shdr_table *shdr, FILE *fh){

  shdr->rela_debug_info.relocation = NULL;
  int ret = parse_relocation(shdr, &shdr->rela_debug_info, fh, ".rela.debug_info");
  if(ret == NOT_FOUND){

    fprintf(stderr, "Section .rela.debug_info not found");
    return -1;
  }

  else if(ret == -1) return -1;

  return 0;
}

int parse_rela_debug_arranges(struct shdr_table *shdr, FILE *fh){

  shdr->rela_debug_arranges.relocation = NULL;
  int ret = parse_relocation(shdr, &shdr->rela_debug_arranges, fh, ".rela.debug_arranges");
  if(ret == NOT_FOUND){

    fprintf(stderr, "Section .rela.debug_arranges not found");
    return -1;
  }

  else if(ret == -1) return -1;

  return 0;
}

int parse_rela_debug_line(struct shdr_table *shdr, FILE *fh){

  shdr->rela_debug_line.relocation = NULL;
  int ret = parse_relocation(shdr, &shdr->rela_debug_line, fh, ".rela.debug_line");
  if(ret == NOT_FOUND){

    fprintf(stderr, "Section .rela.debug_line not found");
    return -1;
  }

  else if(ret == -1) return -1;

  return 0;
}

int parse_rela_rodata(struct shdr_table *shdr, FILE *fh){

  shdr->rela_rodata.relocation = NULL;
  int ret = parse_relocation(shdr, &shdr->rela_rodata, fh, ".rela.rodata");
  if(ret == NOT_FOUND){

    fprintf(stderr, "Section .rela.debug_rodata not found");
    return -1;
  }

  else if(ret == -1) return -1;

  return 0;
}

int parse_rela_text(struct shdr_table *shdr, FILE *fh){

  shdr->rela_text.relocation = NULL;
  int ret = parse_relocation(shdr, &shdr->rela_text, fh, ".rela.text");
  if(ret == NOT_FOUND){

    fprintf(stderr, "Section .rela.text not found");
    return -1;
  }

  else if(ret == -1) return -1;

  return 0;
}

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include "parse.h"

void free_dynamic(struct DYNAMIC *dyn){

  if(dyn->dyn != NULL)
    free(dyn->dyn);
}

Elf64_Addr get_dynamic_tag_data(struct DYNAMIC *dyn, int index){

  switch (dyn->dyn[index].d_tag) {
  
    case DT_NEEDED:
    case DT_PLTRELSZ:
    case DT_RELASZ:
    case DT_RELAENT:
    case DT_STRSZ:
    case DT_SYMENT:
    case DT_SONAME:
    case DT_RELSZ:
    case DT_RELENT:
    case DT_PLTREL:
      return dyn->dyn[index].d_un.d_val;
  
    case DT_PLTGOT:
    case DT_HASH:
    case DT_STRTAB:
    case DT_SYMTAB:
    case DT_RELA:
    case DT_INIT:
    case DT_FINI:
    case DT_REL:
      return dyn->dyn[index].d_un.d_ptr;
  }

  return -1;
}

Elf64_Sxword get_dynamic_tag(struct DYNAMIC *dyn, int index){

  return dyn->dyn[index].d_tag;
}

/* library search path is an env variable which specifies directories that executable search for shared libraries "LD_LIBRARY_PATH" */
char **get_lib_search_path(struct shdr_table *shdr, struct DYNAMIC *dyn){

  char **buffer = NULL;
  int no_of_ent = 0;
  int j = 0;
  for(int i = 0; i < dyn->dynamic_entries; i++){

    if(dyn->dyn[i].d_tag == DT_RUNPATH){    // or DT_RPATH, but deprecated

      ++no_of_ent;
      buffer = realloc(buffer, sizeof(char *) * no_of_ent);
      if(!buffer){

        perror("failed to allocate memory");
        return NULL;
      }

      buffer[j] = &shdr->dynstr.strtab_buffer[dyn->dyn[i].d_un.d_val];
      ++j;
    }
  }

  return buffer;
}

char **get_lib_list(struct shdr_table *shdr, struct DYNAMIC *dyn){

  char **buffer = NULL;
  int no_of_ent = 0;
  int j = 0;
  for(int i = 0; i < dyn->dynamic_entries; i++){

    if(dyn->dyn[i].d_tag == DT_NEEDED || dyn->dyn[i].d_tag == DT_SONAME){

      ++no_of_ent;
      buffer = realloc(buffer, sizeof(char *) * no_of_ent);
      if(!buffer){

        perror("failed to allocate memory");
        return NULL;
      }
      /* "DT_NEEDED String table offset to name of a needed library" - elf specification (no i lied, its man elf :))
       *  So, I assume what it meant by offset is index and string table to be .dynstr
       */
     
      int index = shdr->shdr_table[dyn->dynamic_index].sh_link;    // sh_link of dynamic section specifies an index to section header table. that index refers to string table which dynamic section  entries of TYPE DT_NEEDED and DT_NEEDED uses to store its strings
      if(index == shdr->dynstr.strtab_index){

        buffer[j] = &shdr->dynstr.strtab_buffer[dyn->dyn[i].d_un.d_val];
        ++j;
      }
      else if(index == shdr->strtab.strtab_index){

        buffer[j] = &shdr->strtab.strtab_buffer[dyn->dyn[i].d_un.d_val];
        ++j;
      }
    }
  }
  return buffer;
}

int parse_dynamic(struct shdr_table *shdr, struct DYNAMIC *dyn, FILE *fh){

  dyn->dynamic_index = get_section_index(shdr, ".dynamic");
  if(dyn->dynamic_index < 0)
    return -1;
  
  dyn->dynamic_offset = shdr->shdr_table[dyn->dynamic_index].sh_offset;
  dyn->dynamic_size = shdr->shdr_table[dyn->dynamic_index].sh_size;
  dyn->dynamic_entries = dyn->dynamic_size / sizeof(Elf64_Dyn);

  if(fseek(fh, dyn->dynamic_offset, SEEK_SET) < 0) return -1;

  dyn->dyn = malloc(dyn->dynamic_size);
  if(!dyn->dyn){

    perror("memory allocation error");
    return -1;
  }

  memset(dyn->dyn, 0, dyn->dynamic_size);

  if(fread(dyn->dyn, 1, dyn->dynamic_size, fh) < 0){

    perror("failed to read file");
    return -1;
  }

  return 0;
}

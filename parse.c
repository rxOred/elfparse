#include "parse.h"
#include <stdio.h>

void free_all(ElfData *data){

  /* 
   * shstrtab contains details of section header string table. memeber 
   * strtab_content contains char * pointing to heap where name of every 
   * section is stored 
   */
  if(data->shdrtab.shstrtab.strtab_buffer != NULL)
    free(data->shdrtab.shstrtab.strtab_buffer);

  /* cleaning array of character pointers which points to section header string table */
  if(data->shdrtab.shstrtab.strtab_content != NULL)
    free(data->shdrtab.shstrtab.strtab_content);

  /* cleaning memory where section header table is stored */
  if(data->shdrtab.shdr_table != NULL)
    free(data->shdrtab.shdr_table);

  /* cleaning memort where program header table is stored */
  if(data->phdrtab.phdr_table != NULL)
    free(data->phdrtab.phdr_table);

  free_note(&data->shdrtab.note_abi_tag);
  free_note(&data->shdrtab.note_gnu_build_id);
  free_note(&data->shdrtab.note_gnu_property);
  
  free_symtab(&data->shdrtab.symtab);
  free_symtab(&data->shdrtab.dynsym);

  free_strtab(&data->shdrtab.strtab);
  free_strtab(&data->shdrtab.dynstr);

  free_dynamic(&data->shdrtab.dynamic);

  free_relocation(&data->shdrtab.rela_debug_arranges);
  free_relocation(&data->shdrtab.rela_debug_line);
  free_relocation(&data->shdrtab.rela_rodata);
  free_relocation(&data->shdrtab.rela_text);
  free_relocation(&data->shdrtab.rela_debug_info);

  fclose(data->fh);
}

int get_no_of_strings(char *buffer, int size){

  int no_of_strings = 0;
  for(int i = 0; i < size; i++){

    if(buffer[i] == '\0' || buffer[i] == '\n'){

      no_of_strings++;
    }
  }
 return no_of_strings;
}

int get_section_index(struct shdr_table *shdr, const char *section_name){

  int i;
  for (i = 0; i < shdr->shstrtab.strtab_entries; i++){

    if(shdr->shdr_table[i].sh_name < shdr->shstrtab.strtab_size){
    
      char *buf = &shdr->shstrtab.strtab_buffer[shdr->shdr_table[i].sh_name];
      if((buf))

        if(strcmp(buf, section_name) == 0){
          return i;
      }
    }
  }
  return NOT_FOUND;
}

char **get_strings(struct shdr_table *shdr, int entries){

  char **parsed_string = malloc(sizeof(char *) * entries);
  if(!parsed_string){

    perror("memory allocation failed");
    return NULL;
  }

  memset(parsed_string, 0, sizeof(char *) * entries);

  for (int i = 0; i < entries; i++){

    char *buf = &shdr->shstrtab.strtab_buffer[shdr->shdr_table[i].sh_name];
    parsed_string[i] = buf;
  }
  
  return parsed_string;
}

uint32_t get_section_link(struct shdr_table *shdr, const char *sectionname){

  int index = get_section_index(shdr, sectionname);
  if(index == NOT_FOUND) return NOT_FOUND;

  return shdr->shdr_table[index].sh_link;
}

uint64_t get_section_address_align(struct shdr_table *shdr, const char *sectionname){

  int index = get_section_index(shdr, sectionname);
  if(index == NOT_FOUND) return NOT_FOUND;

  return shdr->shdr_table[index].sh_addralign;
}

uint32_t get_section_index_to_shstr(struct shdr_table *shdr, const char *sectionname){

  int index = get_section_index(shdr, sectionname);
  if(index == NOT_FOUND) return NOT_FOUND;

  return shdr->shdr_table[index].sh_name;
}

uint32_t get_section_type(struct shdr_table *shdr, const char *sectionname){

  int index = get_section_index(shdr, sectionname);
  if(index == NOT_FOUND) return NOT_FOUND;

  return shdr->shdr_table[index].sh_type;
}

uint32_t get_section_info(struct shdr_table *shdr, const char *sectionname){

  int index = get_section_index(shdr, sectionname);
  if(index == NOT_FOUND) return NOT_FOUND;

  return shdr->shdr_table[index].sh_info;
}

uint64_t get_section_flags(struct shdr_table *shdr, const char *sectionname){

  int index = get_section_index(shdr, sectionname);
  if(index == NOT_FOUND) return NOT_FOUND;

  return shdr->shdr_table[index].sh_flags;
}

Elf64_Addr get_section_address(struct shdr_table *shdr, const char *sectionname){

  int index = get_section_index(shdr, sectionname);
  if(index == NOT_FOUND) return NOT_FOUND;

  return shdr->shdr_table[index].sh_addr;
}

off_t get_section_offset(struct shdr_table *shdr, const char *sectionname){

  int index = get_section_index(shdr, sectionname);
  if(index == NOT_FOUND) return NOT_FOUND;

  return shdr->shdr_table[index].sh_offset;
}

uint64_t get_section_entrysize(struct shdr_table *shdr, const char *sectionname){

  int index = get_section_index(shdr, sectionname);
  if(index == NOT_FOUND) return NOT_FOUND;

  return shdr->shdr_table[index].sh_entsize;
}

uint64_t get_section_size(struct shdr_table *shdr, const char *sectionname){

  int index = get_section_index(shdr, sectionname);
  if(index == NOT_FOUND) return NOT_FOUND;

  return shdr->shdr_table[index].sh_size;
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
  data->shdrtab.shstrtab.strtab_buffer = malloc(data->shdrtab.shstrtab.strtab_size * sizeof(char));  //do not free this, used to get section names..
  if(!data->shdrtab.shstrtab.strtab_buffer){
  
    perror("memory allocation failed");
    return -1;
  }

  memset(data->shdrtab.shstrtab.strtab_buffer, 0, data->shdrtab.shstrtab.strtab_size * sizeof(char));
  
  if(fread(data->shdrtab.shstrtab.strtab_buffer, 1, data->shdrtab.shstrtab.strtab_size, data->fh) < (unsigned long)data->shdrtab.shstrtab.strtab_size){

    perror("failed read file");
    return -1;
  }
  /* 
   * after reading, we need tp get the exact number of strings in that table. this is also possible using something like 
   * e_shnum, which gives us number of sections. number of sections == no of strings in shstrtab
   */

#ifdef ___STRENT
  data->shdrtab.shstrtab.strtab_entries = data->shdrtab.shdr_entries;
#else
  data->shdrtab.shstrtab.strtab_entries = get_no_of_strings(data->shdrtab.shstrtab.strtab_buffer, data->shdrtab.shstrtab.strtab_size); 
#endif
 
  /* read to (char *) buffer , sh_size of bytes from current position (section .strtab offset)) */
  data->shdrtab.shstrtab.strtab_content = get_strings(&data->shdrtab, data->shdrtab.shstrtab.strtab_entries);

  return 0;
}

int check_elf(FILE *fh){

  uint8_t buf[5];
 
  fseek(fh, 0, SEEK_SET);
  if(fread(buf, sizeof(uint8_t), 5, fh) < 4){
    
    perror("failed to read file");
    return -1;
  };
  
  if((buf[0] == 0x7f) && (buf[1] == 'E') && (buf[2] == 'L') && (buf[3] == 'F'))
    return 0;
  else{

    fprintf(stderr, "not an elf binary\n");
    return -1;
  }
  return 0;
}

void init_ptr(struct shdr_table *shdr){

  /* initialize all not-yet-used pointers to null */
  shdr->shstrtab.strtab_buffer = NULL;
  shdr->shstrtab.strtab_content = NULL;

  shdr->dynstr.strtab_buffer = NULL;
  shdr->dynstr.strtab_content = NULL;

  shdr->strtab.strtab_buffer = NULL;
  shdr->strtab.strtab_content = NULL;

  shdr->dynsym.symtab = NULL;
  
  shdr->symtab.symtab = NULL;

  shdr->note_abi_tag.notetab = NULL;

  shdr->note_gnu_build_id.notetab = NULL;

  shdr->note_gnu_property.notetab = NULL;

  shdr->dynamic.dyn = NULL;

  shdr->rela_debug_line.relocation = NULL;
  shdr->rela_debug_info.relocation = NULL;
  shdr->rela_debug_arranges.relocation = NULL;
  shdr->rela_rodata.relocation = NULL;
  shdr->rela_text.relocation = NULL;
  
  shdr->comment.buffer = NULL;
}

/*
 * this function reads from the elf file and assign ehdr header and shdr, phdr tables
 */
int assign_headers(ElfData *data){

  if(check_elf(data->fh) == -1) return -1;;
  fseek(data->fh, 0, SEEK_SET);

  /* assigning ehdr with data read from the binary */
  data->elf_header.ehdr_size = sizeof(Elf64_Ehdr);
  if(fread(&data->elf_header.ehdr, data->elf_header.ehdr_size, 1, data->fh) < 1){
  
    perror("failed to read file: ");
    return -1;
  }
  
  /* assigning phdr with data read from binary using ehdr->e_phoff */ 
  if((data->elf_header.ehdr.e_type == ET_EXEC) || (data->elf_header.ehdr.e_type == ET_DYN)){

    data->phdrtab.phdr_entries = data->elf_header.ehdr.e_phnum;
    data->phdrtab.phdr_size = data->phdrtab.phdr_entries * sizeof(Elf64_Phdr);
    data->phdrtab.phdr_offset = data->elf_header.ehdr.e_phoff;
  
    data->phdrtab.phdr_table = malloc(data->phdrtab.phdr_size);
    if(!data->phdrtab.phdr_table) {
   
      perror("memory allocation failed: ");
      return -1;
    }

    memset(data->phdrtab.phdr_table, 0, data->phdrtab.phdr_size);

    fseek(data->fh, data->phdrtab.phdr_offset, SEEK_SET); //seeking file pointer to position where program header table begins

    if(fread(data->phdrtab.phdr_table, sizeof(Elf64_Phdr), data->phdrtab.phdr_entries, data->fh) < (unsigned long)data->phdrtab.phdr_entries){ //reading from that location to allocated space in heap
      perror("failed to read data\n");
      return -1;
    }
  }
  else{
  
    fprintf(stderr, "binary file is not an executable file\n");
  }

  /* assigning shdr with data read from binary using ehdr->e_shoff */
  data->shdrtab.shdr_entries = data->elf_header.ehdr.e_shnum;
  data->shdrtab.shdr_size = data->shdrtab.shdr_entries * sizeof(Elf64_Shdr);
  data->shdrtab.shdr_offset = data->elf_header.ehdr.e_shoff;

  data->shdrtab.shdr_table = malloc(data->shdrtab.shdr_size); //do not free this memory. used througout the code
  if(!data->shdrtab.shdr_table) {

    perror("memory allocation failed: ");
    return -1;
  }

  memset(data->shdrtab.shdr_table, 0, data->shdrtab.shdr_size);

  fseek(data->fh, data->shdrtab.shdr_offset, SEEK_SET);

  if(fread(data->shdrtab.shdr_table, sizeof(Elf64_Shdr), data->shdrtab.shdr_entries, data->fh) < (unsigned long)data->shdrtab.shdr_entries){
  
    perror("could not read section header table");
    return -1;
  }

  init_ptr(&data->shdrtab);
  return 0;
}

FILE *open_file(const char *filename){

  FILE *local = fopen(filename, "rb");
  if(!local){

    perror("coud not open binary file");
    return NULL;
  }

  return local;
}

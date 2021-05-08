#include "parse.h"

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

int get_no_of_strings(char *buffer, int size){

  

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

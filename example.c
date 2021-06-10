#include <stdlib.h>
#include "parse.h"

int main(void){

  ElfData data;
  data.filename = "./dwarf";
  puts("----------------------------------------------");
  printf("filename : %s\n", data.filename);
  puts("----------------------------------------------\n");

  data.fh = open_file(data.filename);
  if(data.fh == NULL) return -1;
  if(assign_headers(&data) == -1) return -1;

  puts("-----------------------------------------------");
  printf("total entries %d\n", data.shdrtab.shdr_entries);
  puts("-----------------------------------------------\n");

  parse_shstrtab(&data);

  puts("-----------------------------------------------");
  printf("size of shstrtab :%d\t no of shstrtab entries :%d\n", data.shdrtab.shstrtab.strtab_size, data.shdrtab.shstrtab.strtab_entries);
  puts("-----------------------------------------------\n");

  parse_symtab(&data.shdrtab, data.fh);
  if(parse_strtab(&data.shdrtab, data.fh) != -1){

    for(int i = 0; i < data.shdrtab.strtab.strtab_entries; i++){

      printf("%s\n", data.shdrtab.strtab.strtab_content[i]);
    }
  } 

  parse_dynsym(&data.shdrtab, data.fh);
  if(parse_dynstr(&data.shdrtab, data.fh) != -1){

    printf("%s\n", data.shdrtab.dynstr.strtab_buffer);
  }
  
  puts("-------------------------------------------------");
  for (int i = 0; i < data.shdrtab.shstrtab.strtab_entries; i++){

    printf("%d\t0x%lx\t%lu\t%s\n", i, data.shdrtab.shdr_table[i].sh_addr, data.shdrtab.shdr_table[i].sh_size, data.shdrtab.shstrtab.strtab_content[i]);
  }
  puts("------------------------------------------------\n");

  parse_note_abi_tag(&data.shdrtab, data.fh);

  parse_dynamic(&data.shdrtab, &data.shdrtab.dynamic, data.fh);
  get_lib_list(&data.shdrtab, &data.shdrtab.dynamic);

  free_all(&data);
  return 0;
}

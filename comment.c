#include <stdlib.h>
#include "parse.h"

int parse_comment(struct shdr_table *shdr, struct COMMENT *comment, FILE *fh){

  comment->comment_index = get_section_index(shdr, ".comment");
  if(comment->comment_index < 0)
    return -1;
  
  comment->comment_offset = shdr->shdr_table[comment->comment_index].sh_offset;
  comment->comment_size = shdr->shdr_table[comment->comment_index].sh_size;
  comment->comment_entries = comment->comment_size / sizeof(Elf64_Dyn);

  if(fseek(fh, comment->comment_offset, SEEK_SET) < 0) return -1;

  comment->buffer = malloc(comment->comment_size);
  if(!comment->buffer){

    perror("memory allocation error");
    return -1;
  }

  memset(comment->buffer, 0, comment->comment_size);

  if(fread(comment->buffer, 1, comment->comment_size, fh) < 0){

    perror("failed to read file");
    return -1;
  }

  return 0;
}

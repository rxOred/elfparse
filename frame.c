#include "parse.h"
#include <stdint.h>
#include <stdio.h>

int parse_frame(struct shdr_table *shdr, struct FRAME *frame, FILE *fh, const char *section_name){

  frame->frame_index = get_section_index(shdr, section_name);
  if(frame->frame_index == (uint8_t)NOT_FOUND)  return NOT_FOUND;

  frame->frame_size = shdr->shdr_table[frame->frame_index].sh_size;
  frame->frame_offset = shdr->shdr_table[frame->frame_index].sh_offset;

  frame->buffer = malloc(frame->frame_size);
  if(!frame->buffer){

    perror("Memory allocation error\n");
    return -1;
  }

  memset(frame->buffer, 0, frame->frame_size);

  fseek(fh, frame->frame_offset, SEEK_SET);
  if(fread(frame->buffer, 1, frame->frame_size, fh) < frame->frame_size){

    perror("error reading file");
    return -1;
  }

  for(int i = 0; i < frame->frame_size; i++){

    putchar(frame->buffer[i]);
  }

  return 0;
}

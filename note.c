#include "parse.h"
#include <stdio.h>

void free_note(struct NOTETAB *note){

  if(note->notetab != NULL)
    free(note->notetab);
}

Elf64_Word get_note_type(struct NOTETAB *note, int index){

  return note->notetab[index].n_type;
}

Elf64_Word get_note_name_len(struct NOTETAB *note, int index){

  return note->notetab[index].n_namesz;
}

Elf64_Word get_note_descriptor_len(struct NOTETAB *note, int index){

  return note->notetab[index].n_descsz;
}

int parse_notetab(struct shdr_table *shdr, struct NOTETAB *note, FILE *fh, const char *sectionname){

  note->notetab_index = get_section_index(shdr, sectionname);
  if(note->notetab_index == (uint8_t)NOT_FOUND)
    return NOT_FOUND;

  note->notetab_size = shdr->shdr_table[note->notetab_index].sh_size;
  note->notetab_entries = note->notetab_size / shdr->shdr_table[note->notetab_index].sh_entsize;
  note->notetab_offset = shdr->shdr_table[note->notetab_index].sh_offset;

  note->notetab = malloc(note->notetab_entries * sizeof(Elf64_Nhdr));
  if(!note->notetab){

    perror("memory allocation failed");
    return -1;
  }
  memset(note->notetab, 0, note->notetab_entries * sizeof(Elf64_Nhdr));

  fseek(fh, note->notetab_offset, SEEK_SET);
  if(fread(note->notetab, 1, note->notetab_size, fh) < note->notetab_size){

    perror("failed to read file");
    return -1;
  }

  return 0;
}

int parse_note_abi_tag(struct shdr_table *shdr, FILE *fh){

  shdr->note_abi_tag.notetab = NULL;
  int ret = parse_notetab(shdr, &shdr->note_abi_tag, fh, "note.ABI-tag");
  if(ret == NOT_FOUND){

    fprintf(stderr, "Section .note.ABI-tag not found\n");
    return NOT_FOUND;
  }
  else {

    return -1;
  }

  return 0;
}

int parse_note_gnu_property(struct shdr_table *shdr, FILE *fh){

  shdr->note_gnu_property.notetab = NULL;
  int ret = parse_notetab(shdr, &shdr->note_gnu_property, fh, "note.gnu.property");
  if(ret == NOT_FOUND){

    fprintf(stderr, "Section .note.gnu.property not found\n");
    return -1;
  }
  else{

    return -1;
  }

  return 0;
}

int parse_note_gnu_build_id(struct shdr_table *shdr, FILE *fh){

  shdr->note_gnu_property.notetab = NULL;
  int ret = parse_notetab(shdr, &shdr->note_gnu_property, fh, "note.gnu.build.id");
  if(ret == NOT_FOUND){

    fprintf(stderr, "Section .note.gnu.build-id not found\n");
    return -1;
  }
  else{

    return -1;
  }

  return 0;
}

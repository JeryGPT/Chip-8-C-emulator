#include<stdlib.h>
#include<stdio.h>
#include<string.h>



unsigned char fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
typedef struct {
  unsigned short opcode; // 2b
  unsigned char memory[4096]; // 4096b
  unsigned char V[16];  // 16b
  unsigned short I; // 2b
  unsigned short pc; // 2b
  unsigned char gfx[64*32]; // ekran 64x32 kazdy pixel 1b
  unsigned short stack[16]; // stack 32b lacznie
  unsigned short sp; // pointer do stacka
  unsigned char key[16]; // lista inputow
  unsigned char delay_timer;
  unsigned char sound_timer;
} Chip8;

FILE *file_ptr;
#define ROMS_PATH "./roms/"

int load_rom(Chip8* chip, const char* rom_name) {
  const size_t path_len = strlen(rom_name) + sizeof(ROMS_PATH) + 1;
  char *file_path = calloc(path_len, sizeof(char));
  strcat(file_path, ROMS_PATH);
  strcat(file_path, rom_name);
  file_ptr = fopen(file_path, "rb");
  if (file_ptr == NULL) {
    printf("[Error] ROM's file not found\n");
    return -1;
  };
  fseek(file_ptr, 0, SEEK_END);
  const long file_size = ftell(file_ptr);
  rewind(file_ptr);
  if (fread(&chip->memory[0x200], sizeof(chip->memory[0]), file_size, file_ptr) != file_size){
    printf("[Error] Couldn't load the ROM to the memory\n");
    return -1;
  };

  fclose(file_ptr);

  free(file_path);
  return 0;
}

void chip8_init(Chip8 *chip) {
  memset(chip, 0, sizeof(Chip8));
  chip->pc = 0x200;
  chip->opcode = 0;
  chip->I = 0;
  chip->sp = 0;
  memcpy(&chip->memory[0x050], fontset, sizeof(fontset));
  printf("[+] Fontset loaded to mSSemory\n");  
}

int main() {
  char rom_name[] = "Chip8Picture.ch8";
  Chip8 chip;
  chip8_init(&chip);
  if (load_rom(&chip, rom_name) != 0) {
    printf("[-] Failed to load the ROM\n");
    return -1;
  }else{
    printf("[+] ROM loaded to the memory\n");
  }
  return 0;
}
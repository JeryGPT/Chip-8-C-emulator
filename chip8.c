#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<time.h>

unsigned char fontset[80] = 
{
	0xF0, 0x90, 0x90, 0x9b, 0xF0, // 0
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
    free(file_path);
    return -1;
  };
  fseek(file_ptr, 0, SEEK_END);
  const long file_size = ftell(file_ptr);
  rewind(file_ptr);
  if (file_size > 4096 - 0x200) {
    printf("[Error] Couldn't load the ROM to the memory (OOM)");
    free(file_path);
    return -1;
  }
  if (fread(&chip->memory[0x200], sizeof(chip->memory[0]), file_size, file_ptr) != file_size){
    printf("[Error] Couldn't load the ROM to the memory\n");
    free(file_path);
    return -1;
  };

  fclose(file_ptr);

  free(file_path);
  return file_size;
}

int get_random_number() {
  return rand() % 256;
};

void chip8_init(Chip8 *chip) {
  memset(chip, 0, sizeof(Chip8));
  chip->pc = 0x200;
  chip->opcode = 0;
  chip->I = 0;
  chip->sp = 0;
  memcpy(&chip->memory[0x050], fontset, sizeof(fontset));
  printf("[+] Fontset loaded to memory\n");  
}

unsigned short get_opcode(Chip8 *chip) {
  unsigned short opcode = (chip->memory[chip->pc] << 8) | chip->memory[chip->pc+1]; // opcode sa z 2 bajtow, a tablica jest 1 bajtowa. Dlatego pobieram
  // opcode to 16bitow (czyli unsigned short). Bity 15-12 -> ROzkaz 11-8 Parametr 1 7-4 Parametr 2 

  chip->pc += 2;


  return opcode;
}

void run_opcode(Chip8* chip) {
  unsigned short opcode = get_opcode(chip);
  unsigned short T = opcode >> 12;
  unsigned short X = (opcode & 0x0F00) >> 8; // usuwa wszyskto poza 2 segment z ciagu 4 znakow. Jest TXYN i bierze tylko 2 - 0X00, potem jest przesuniete o 8 bitow wiec zostaje 0X czyli to co chce
  unsigned short Y = (opcode & 0x00F0) >> 4;
  unsigned short N = (opcode & 0x0000F);
  unsigned short NN = (opcode & 0x00FF);
  unsigned short NNN = (opcode & 0x0FFF);
  switch (T){
    case 0x0:
      if (NN == 0xE0) { // CLS
        system("cls");
      }else if (NN == 0xEE) { // RET
        chip->sp -= 1;
        chip->pc = chip->stack[chip->sp];
      }
      break;
    case 0x1:
      chip->pc = NNN;
      break;
    case 0x2:
      chip->stack[chip->sp] = chip->pc;
      chip->sp++;

      chip->pc = NNN;
      break;
    case 0x3:
      if (chip->V[X] == NN) {
        chip->pc += 2;
      }
      break;
    case 0x4:
      if (chip->V[X] != NN) {
        chip->pc += 2;
      }
      break;
    case 0x5: 
      if (chip->V[X] == chip->V[Y]){
        chip->pc += 2;
      }
      break;
    case 0x6:
      chip->V[X] = NN;
      break;
    case 0x7:
      chip->V[X] = chip->V[X] + NN;
      break;
    case 0x8:
      if (N == 0x0) {
        chip->V[X] = chip->V[Y];
      }
      else if (N == 0x1) {
        chip->V[X] = chip->V[X] | chip->V[Y];
      }
      else if (N == 0x2) {
        chip->V[X] = chip->V[X] & chip->V[Y];
      }
      else if (N == 0x3) {
        chip->V[X] = chip->V[X] ^ chip->V[Y];
      }
      else if (N == 0x4) {
        int sum = chip->V[X] + chip->V[Y];
        if (sum > 255) {
          chip->V[0xF] = 1;
        }else{
          chip->V[0xF] = 0;
        }
        chip->V[X] = (sum & 0xFF); // ucinam wszystkie bity po 8, nie chce overflowa 
      }
      else if (N == 0x5) {
        chip->V[0xF] = (chip->V[X] >= chip->V[Y]) ? 1 : 0;
        chip->V[X] = chip->V[X] - chip->V[Y];
      }
      else if (N == 0x6) {
        chip->V[0xF] = (chip->V[X] & 1) ? 1 : 0;
        chip->V[X] = chip->V[X] >> 1; // to samo co dzielenie przez 2 as far as i know
      }
      else if (N == 0x7) {
        chip->V[0xF] = (chip->V[Y] > chip->V[X]) ? 1 : 0;
        chip->V[X] = chip->V[Y] - chip->V[X];
      }
      else if (N == 0xE) {
        chip->V[0xF] = ((chip->V[X] & 0x80) == 0x80) ? 1 : 0;
        chip->V[X] <<= 1;
      }
     
      break;
    case 0x9:
      if (chip->V[X] != chip->V[Y]){
        chip->pc += 2;
      }
      break;

    case 0xA:
      chip->I = NNN;
      break;
    case 0xB:
      chip->pc = NNN + chip->V[0];
      break;
    case 0xC:
      chip->V[X] = get_random_number() & chip->V[X];
      break;
    case 0xD:
      uint8_t sprite_data[15]; 
      uint8_t x_position = chip->V[X];
      uint8_t y_position = chip->V[Y];
      int start_index = x_position * y_position;
       for (int i = 0; i < N; i++) { // kopiowanie N bajtow do sprite_data
          sprite_data[i] = chip->memory[chip->I + i];
      }
      break;
    case 0xE:
      switch (NN) {
        case 0x9E:
          int key_down = 1; // Jak przycisk z chip->V[X] jest nacisniety 
          if (key_down) {
            chip->pc+=2;
          }
          break;
        case 0xA1:
          int key_down = 1; // Jak przycisk z chip->V[X] jest NIE nacisniety 
          if (key_down) {
            chip->pc+=2;
          }
          break;
      };
    default:
      printf("I DONT HAVE IT 0x%X 0x%X\n", opcode, X);

  }
}

int start_chip8(Chip8 *chip, int rom_size) {
  printf("Start\n");

    while (1){
      run_opcode(chip);
    }
  return 0;
};

int main() {
  srand(time(NULL)); 
  char rom_name[] = "Chip8Picture.ch8";
  Chip8 chip;
  chip8_init(&chip);
  int rom_size = load_rom(&chip, rom_name);
  if (rom_size <= 0) {
    printf("[-] Failed to load the ROM\n");
    return -1;
  }else{
    printf("[+] ROM loaded to the memory\n");
    start_chip8(&chip, rom_size);

  };

  return 0;
}
unsigned short opcode;
unsigned char memory[4096];
unsigned char V[16];
unsigned short I; // Index register I
unsigned short pc; // program counter which can have a value from 0x000 to 0xFFF

/** Systems memory map
 0x000-0x1FF - Chip 8 interpreter (contains font set in em)
 0x050-0x0A0 - Used for the built in 4x5 pixet font set (0-F)
 0x200=0xFFF - Program ROM and work RAM
*/

bool drawFlag;
unsigned char gfx[64 * 32]; // array that holds pixel in a binary state
unsigned char delay_timer;
unsigned char sound_timer; // the two clocks, when set above zero they count down to zero

unsigned short stack[16]; // stack to store the program counter before a jump
unsigned short sp; // remember which level of the stack is used.

unsigned char key[16]; // hex keypad 0x0 - 0xF, array holds current state of the key

void initialize()
{
  // Initialize registers and memory once
  pc     = 0x200;  // Program counter starts at 0x200
  opcode = 0;      // Reset current opcode
  I      = 0;      // Reset index register
  sp     = 0;      // Reset stack pointer 

  // Clear display
  for(int i = 0; i < 2046; ++i)
  {
    gfx[i] = 0;
  }

  // Clear stack, keys, and registers
  for( int i = 0; i < 16; ++i)
  {
    stack[i] = key[i] = V[i] = 0;
  }

  // Clear memory
  for(int i = 0; i < 4096; ++i)
  {
    memory[i] = 0;
  }

  // Load fontset
  for(int i = 0; i < 80; ++i){
    memory[i] = chip8_fontset[i];
  }

  // reset timers
  delay_timer = sound_timer = 0;

  drawFlag = true
}

void emulateCycle()
{
  // Fetch Opcode
  // To start we must store both opcodes
  opcode = memory[pc] << 8 | memory[pc + 1]

  // Decode and Execute opcode
  switch(opcode & 0xF000)
  {
    // Some Opcodes // 
    case 0x0000: 
      switch(opcode & 0x000F)
      { 
        case 0x0000: // 0x00E0: Clears the screen
          for(in i = 0; i < 2046; ++i)
          {
            gfx[i] = 0x0;
          }
          drawFlag = true;
          pc += 2;
          break;

        case 0x000E: // 0x00EE: Returns from subroutine
          --sp;
          pc = stack[sp]; // put the return address from the stack into the counter
          pc += 2;
          break;

       default:
          printf("Unknown opcode 0x%X\n", opcode);
          break;
      }

    case 0x1000: // 0x1NNN: Jump to address NNN
      pc = opcode & 0x0FFF;
      break;

    case 0x2000: // Calls the subroutine to address NNN
      stack[sp] = pc;
      ++sp;
      pc = opcode & 0x0FFF;
      break;

    case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
      if (V[(opcode & 0x0800) >> 8] == (opcode & 0x00FF) )
      {
        pc += 4;
      }
      else
      {

      }
      break;

    case 0x4000: // 0x4XNN: Skips the next instruction if VX is not equal to NN
      if (V[(opcode & 0x0800) >> 8] != (opcode & 0x00FF) )
      {
        pc += 4;
      }
      else
      {

      }
      break;

    case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY
     if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
     {
      pc += 4;
     }
     else
     {
      pc += 2;
     }
     break;

    case 0x6000: // 0x6XNN : Sets VX to NN
      V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
      pc += 2;
      break;

    case 0x7000: // 0x7XNN : Adds NN to VX
      V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF)
      pc += 2;
      break;

    case 0x8000:
      switch (opcode & 0x000F)
      {
        case 0x0000: // 0x8XY0 : Sets VX to the values of VY
          V[(opcode & 0x0F00) >> 8)] = V[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;

        case 0x0001: // 0x8XY1 : Sets VX to VX or VY
          V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;

        case 0x0002: // 0x8XY2 : Sets VX to VX and VY
          V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;

        case 0x0003: // 0x8XY3 : Sets VX to VX xor VY
          V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;

        case 0x0004: //0x8XY4 : Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
          if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
          {
            V[0xF] = 1; //carry
          }
          else
          {
            V[0xF] = 0;
          }

          V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
          pc += 2;          
          break;

        case 0x0005: //0x8XY5 : VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
          if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
          {
            V[0xF] = 0; // there is a borrow
          }
          else 
          {
            V[0x0] = 1;
          }
          V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
          pc += 2;

        case 0x0006: // 0x8XY6  Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
           V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
           V[(opcode & 0x0F00) >> 8] >>= 1;
           pc += 2;
      }

    case 0xF000:
      switch(opcode & 0x00FF) // 0xFX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2 
      {
        case 0x0033: 
          memory[I]     = V[(opcode & 0x0F00) >> 8 ] / 100;
          memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
          memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
          pc += 2;
         break;
    } 


    case 0xA000: // ANN: Sets I to the address NNN
      // Exceture Opcode
      I = opcode & 0x0FFF;
      pc += 2;
      break;

    default:
     printf("Unknown opcode 0x%X\n", opcode);
    // More opcodes //
  }

  // Update Timers
  if(delay_timer > 0){ --delay_timer;}

  if(sound_timer > 0)
  {
    if(sound_timer == 1){
      printf("BEEP!\n");
      --sound_timer;
    }
  }

}

void loadGame(char gName[])
{
   FILE *game = fopen(gName, 'rb');
 
   if (game == NULL) {
     fprintf(stderr, "Can't open %s\n", gName);
     exit(1);
   }

   unsigned char buffer[3583];

   for( int i = 0; i < 3584; i++) // 0xFFF - 0x200 =... 3583
   {
    fread( &memory[i + 511], sizeof(unsigned char) , 1, game)); 
   }

  
   
}

typedef struct chip8{
	void (*initialize)() = initialize;
	void (*emulateCycle)() = emulateCycle;
	
} chip8;


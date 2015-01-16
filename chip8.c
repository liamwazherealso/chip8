#include <stdlib.h>
#include <stdio.h>
#include "chip8.h"

unsigned char chip8_fontset[80] =
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

  drawFlag = true;
  //srand (time(NULL))
}

void emulateCycle()
{
  // Fetch Opcode
  // To start we must store both opcodes
  opcode = memory[pc] << 8 | memory[pc + 1];

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

        case 0x0007: // 0x8XY7 : Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
          if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
          {
            V[0xF] = 0; // there is a borrow
          }
          else 
          {
            V[0x0] = 1;
          }

          V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
          pc += 2;
          break;

        case 0x000E: // 0x8XYE : Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
          V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
          V[(opcode & 0x0F00) >> 8] <<= 1;
          pc += 1;

        default:
          printf("Unknown opcode 0x%X\n", opcode);
      }

    case 0x9000: // 0x9XY0 : Skips the next instruction if VX doesn't equal VY.
      if (V[(opcode & 0x00F0) >> 4]  != V[(opcode & 0x0F00) >> 8]){
        pc += 4;
      }
      break;

    case 0xA000: // 0xANNN : Sets I to the address NNN.
      I = opcode &  0x0FFF;
      pc += 2;
      break;

    case 0xB000: // 0xBNNN : Jumps to the address NNN plus V0.
      pc = (opcode & 0x0FFF) + V[0];
      break;

    case 0xC000: // 0xCXNN : Sets VX to a random number and NN.
      V[(opcode & 0x0F00) >> 8 ] = (rand() % 0xFF) & (opcode & 0x00FF);
      pc += 2;
      break;
      
    case 0xD000: // 0xDXYN 	Sprites stored in memory at location in index register (I), maximum 8bits wide. Wraps around the screen. 
                 // If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen pixels)
      unsigned short x = V[(opcode & 0x0F00) >> 8];
      unsigned short y = V[(opcode & 0x00F0) >> 4];
      unsigned short height = opcode & 0x000F;
      unsigned short pixel;

      V[0xF] = 0;
      for(int xline = 0; xline < 8; xline++)
      {
        if((pixel & (0x80 >> xline)) != 0)
          {
            if(gfx[(x + xline + ((y + yline) * 64))] == 1)
            {
              V[0xF] = 1;   	
            }            
              gfx[x + xline + ((y + yline) * 64)] ^= 1;
           }

         drawFlag - true;
         pc += 2; 
    }
      break;

    case 0xE000:
      switch(opcode & 0x00FF)
      {
      	case 0x009E: // 0xEX9E : Skips the next instruction if the key stored in VX is pressed.
      	  if ( key[V[(opcode & 0x0F00) >> 8 ]] != 0) // redundant
      	  {
      	  	pc += 4;
      	  }
      	  else
      	  {
      	  	pc += 2;
      	  }
      	break;

      	case 0x00A1: // 0xEXA1 :	Skips the next instruction if the key stored in VX isn't pressed.
      	  if ( key[V[(opcode & 0x0F00) >> 8 ]] == 0)
      	  {
      	  	pc += 4;
      	  }
      	  else
      	  {
      	  	pc += 2;
      	  }
      	break;

      	default:
          printf("Unknown opcode 0x%X\n", opcode);
      }
    case 0xF000:
        switch(opcode & 0x00FF)
        {
           case 0x0007: // 0xFX07 :Sets VX to the value of the delay timer
             V[(opcode & 0x0F00) >> 8 ] = delay_timer;
             pc += 2;
             break;

           case 0x000A: // 0xFX0A 	A key press is awaited, and then stored in VX
             bool keyPress = false;

             for(int i = 0; i < 16; ++i)
             {
             	if(key[i] != 0)
             	{
             		V[(opcode & 0x0F00) >> 8] = i;
             		keyPress = true;
             	}
             }

             if(!keypress) {return;} // Skip cycle and keep trying until there is a keypress

             pc += 2;
             break;

           case 0x0015: // 0xFX15 :Sets the delay timer to VX.
             delay_timer = V[(opcode & 0x0F00) >> 8];
             pc += 2;
             break;

           case 0x0018: // 0xFX18 : Sets the sound timer to VX
             sound_timer = V[(opcode & 0x0F00)>> 8];
             pc += 2;
             break;

           case 0x001E: // 0xFX1E : Adds VX to I 
             if (I + V[(opcode & 0x0F00)>> 8] > 0xFFF)
             {
             	V[0xF] = 1; // there is overflow
             }
             else 
             {
             	V[0xF] = 0;
             }
             I += V[(opcode & 0x0F00)>> 8];
             pc += 2;
             break;

           case 0x0029: // 0xFX29 : Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.

             break;

           case 0x0033: // 0xFX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2 
             memory[I]     = V[(opcode & 0x0F00) >> 8 ] / 100;
             memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
             memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
             pc += 2;
             break;

           case 0x0055: // 0xFXFF : Stores V0 to VX in memory starting at address I
             for (int i = 0; i < (opcode & 0x0F00) >> 8; i++)
             {
             	memory[I + i] = V[i];
             }

             // In the original interpreter, when the operation is done, I = I + x + 1. Got that straight from the tut, have no clue why
             I += ((opcode & 0x0F00) >> 8) + 1;
             pc += 2;
             break;

           case 0x0055: // 0xFXFF : Fills V0 to VX with memory starting at address I
             for (int i = 0; i < (opcode & 0x0F00) >> 8; i++)
             {
             	V[i] = memory[I + i];

             }

             // In the original interpreter, when the operation is done, I = I + x + 1. Got that straight from the tut, have no clue why
             I += ((opcode & 0x0F00) >> 8) + 1;

             pc += 2;
             break;
           
           default:
             printf("Unknown opcode 0x%X\n", opcode);
        }

    
    default:
      printf("Unknown opcode 0x%X\n", opcode);
  }

  // Update Timers
  if(delay_timer > 0)
  	{ 
  		--delay_timer;
  	}

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
int main(int argc, char **argv)
{
	// Set up render system and register input callbacks
	setupGraphics();
	setupInput();

	// Initialize the Chip8 system and load the game into the memory 
	myChip8.initialize();
	myChip8.loadGame("Pong");
	
	// Emulation loop
	for(;;;)
	{
		// Emulate one cycle
		myChip8.emulateCycle();

		// If the draw flag is set, update the screen
		if(myChip8.drawFlag) { drawGraphics(); } // opCode 0x00E0 / 0xDXYN

		// Store key press state (Press and Release)
		myChip.setKets();
	}

	return 0;
}



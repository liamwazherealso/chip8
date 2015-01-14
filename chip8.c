#include "chip8.h"
chip8 myChip8;

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



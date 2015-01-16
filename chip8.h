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

unsigned char V[16];

unsigned short I;
unsigned short pc;


typedef struct chip8{
	void (*initialize)();
	void (*emulateCycle)();
	
} chip8;


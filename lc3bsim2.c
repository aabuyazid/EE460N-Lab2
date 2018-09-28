/*

    Name 1:
    Name 2:
    UTEID 1:
    UTEID 2:
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
*/

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

    int PC,		/* program counter */
            N,		/* n condition bit */
            Z,		/* z condition bit */
            P;		/* p condition bit */
    int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3b ISIM Help-----------------------\n");
    printf("go               -  run program to completion         \n");
    printf("run n            -  execute program for n instructions\n");
    printf("mdump low high   -  dump memory from low to high      \n");
    printf("rdump            -  dump the register & bus values    \n");
    printf("?                -  display this help menu            \n");
    printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

    process_instruction();
    CURRENT_LATCHES = NEXT_LATCHES;
    INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;

    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
        cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k;

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
    printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
    fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
        case 'G':
        case 'g':
            go();
            break;

        case 'M':
        case 'm':
            scanf("%i %i", &start, &stop);
            mdump(dumpsim_file, start, stop);
            break;

        case '?':
            help();
            break;
        case 'Q':
        case 'q':
            printf("Bye.\n");
            exit(0);

        case 'R':
        case 'r':
            if (buffer[1] == 'd' || buffer[1] == 'D')
                rdump(dumpsim_file);
            else {
                scanf("%d", &cycles);
                run(cycles);
            }
            break;

        default:
            printf("Invalid Command\n");
            break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
        MEMORY[i][0] = 0;
        MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
        program_base = word >> 1;
    else {
        printf("Error: Program file is empty\n");
        exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
                   program_filename, ii);
            exit(-1);
        }

        /* Write the word to memory array. */
        MEMORY[program_base + ii][0] = word & 0x00FF;
        MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
        ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
    int i;

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 2) {
        printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argc - 1);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
        get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

System_Latches process_ADD(int instruction);
System_Latches process_AND(int instruction);
System_Latches process_XOR(int instruction);
System_Latches process_SHF(int instruction);
System_Latches process_STB(int instruction);
System_Latches process_LDB(int instruction);
System_Latches setCC(System_Latches update, int Source1Reg);
void process_instruction(){
    /*  function: process_instruction
     *
     *    Process one instruction at a time
     *       -Fetch one instruction
     *       -Decode
     *       -Execute
     *       -Update NEXT_LATCHES
     */

}

//Do we treat them as unsigned or signed int?
//Do we read in PC, go to that memory address, and then update the conditions?
System_Latches process_ADD(int instruction){
    System_Latches temp = CURRENT_LATCHES;
    int maskOne = 0x00020;
    int DestReg = (instruction >> 9) & (0x00007);
    int Source1Reg = (instruction>>6) & (0x00007);
    int Source2Reg = (instruction & 0x00007);
    int immediate = (instruction & (0x0001F)); //Is this fine? I feel like it has
    //Issues with doing it like this
    int signMask = 0x00010;
    if((maskOne & instruction) == 0){
        temp.REGS[DestReg] = temp.REGS[Source1Reg] + temp.REGS[Source2Reg];
    }
    else{
            if((instruction & signMask) == 0){
                temp.REGS[DestReg] = temp.REGS[Source1Reg] + immediate;
            }
            else{
                temp.REGS[DestReg] = temp.REGS[Source1Reg] + (immediate - 32) ;
            }
    }
    temp = setCC(temp, Source1Reg);
}

System_Latches process_AND(int instruction){
    System_Latches temp = CURRENT_LATCHES;
    int maskOne = 0x00020;
    int DestReg = (instruction >> 9) & (0x00007);
    int Source1Reg = (instruction>>6) & (0x00007);
    int Source2Reg = (instruction & 0x00007);
    int immediate = (instruction & (0x0001F)); //Is this fine? I feel like it has
    //Issues with doing it like this
    int signMask = 0x00010;
    if((maskOne & instruction) == 0){
        temp.REGS[DestReg] = temp.REGS[Source1Reg] | temp.REGS[Source2Reg];
    }
    else{
        if((signMask & instruction) == 0){
            immediate &= 0x0001F;
        }
        else{
            immediate |= 0xFFFE0;
        }
        immediate &= 0x0FFFF;
        temp.REGS[DestReg] = temp.REGS[Source1Reg] | immediate;
    }
    temp = setCC(temp, Source1Reg);
    return temp;
}

System_Latches process_XOR(int instruction){
    System_Latches temp = CURRENT_LATCHES;
    int maskOne = 0x00020;
    int DestReg = (instruction >> 9) & (0x00007);
    int Source1Reg = (instruction>>6) & (0x00007);
    int Source2Reg = (instruction & 0x00007);
    int immediate = (instruction & (0x0001F)); //Is this fine? I feel like it has
    //Issues with doing it like this
    int signMask = 0x00010;
    if((maskOne & instruction) == 0){
        temp.REGS[DestReg] = temp.REGS[Source1Reg] ^ temp.REGS[Source2Reg];
    }
    else{
        if((signMask & instruction) == 0){
            immediate &= 0x0001F;
        }
        else{
            immediate |= 0xFFFE0;
        }
        immediate &= 0x0FFFF;
        temp.REGS[DestReg] = temp.REGS[Source1Reg] ^ immediate;
    }
    temp = setCC(temp, Source1Reg);
    return temp;
}

System_Latches process_SHF(int instruction){
    System_Latches temp = CURRENT_LATCHES;
    int bitFourMask = 0x00010;
    int bitFiveMask = 0x00020;
    int DestReg = (instruction >> 9) & (0x00007);
    int Source1Reg = (instruction>>6) & (0x00007);
    int immediate = (instruction & (0x0000F));
    //Make sure it is only 16 bit value
    if((bitFourMask & instruction) == 0){
        temp.REGS[DestReg] = (temp.REGS[Source1Reg] << immediate) & 0x0FFFF;
    }
    else if((bitFiveMask & instruction) == 0){
        temp.REGS[DestReg] = temp.REGS[Source1Reg] >> immediate;
    }
    else{
        int highBit = (0x08000 & temp.REGS[Source1Reg]);
        if(highBit == 0){
            int tempReg = 0;
            for(int i = 0; i < immediate; i++){
                tempReg = temp.REGS[Source1Reg] & 0x07FFF;
                tempReg = tempReg >> 1;
            }
            temp.REGS[DestReg] = tempReg;
        }
        else{
            int tempReg = 0;
            for(int i = 0; i < immediate; i++){
                tempReg = temp.REGS[Source1Reg] | 0x08000;
                tempReg = tempReg >> 1;
            }
            temp.REGS[DestReg] = tempReg;
        }
    }
    temp = setCC(temp, Source1Reg);
    return temp;
}

System_Latches process_STB(int instruction){
    System_Latches temp = CURRENT_LATCHES;
    int bitFiveMask = 0x00020;
    int Source1Reg = (instruction >> 9) & (0x00007);
    int BaseReg = (instruction>>6) & (0x00007);
    int immediate = (instruction & (0x0003F));
    int memLocation = temp.REGS[BaseReg];
    if((bitFiveMask & instruction) == 0){
        int local = memLocation + immediate;
        int highLowByte = local % 2;
        MEMORY[local / 2][highLowByte] = 0x000FF & temp.REGS[Source1Reg] ;
    }
    else{
        immediate -= 64 ;
        int local = memLocation + immediate;
        int highLowByte = local % 2;
        MEMORY[local / 2][highLowByte] = 0x000FF & temp.REGS[Source1Reg] ;
    }
    return temp;
}

//SetsCC
System_Latches process_LDB(int instruction){
    System_Latches temp = CURRENT_LATCHES;
    int bitFiveMask = 0x00020;
    int signMask = 0x00080;
    int Source1Reg = (instruction >> 9) & (0x00007);
    int BaseReg = (instruction>>6) & (0x00007);
    int immediate = (instruction & (0x0003F));
    int memLocation = temp.REGS[BaseReg];
    if((bitFiveMask & instruction) == 0){
        int local = memLocation + immediate;
        int highLowByte = local % 2;
        int tempMem = MEMORY[local / 2][highLowByte] & 0x000FF;
        if(tempMem & signMask == 0){
            temp.REGS[Source1Reg] = tempMem & 0x000FF;
        }
        else{
            temp.REGS[Source1Reg] = tempMem | 0x0FF00;
        }
    }
    else{
        immediate -= 64 ;
        int local = memLocation + immediate;
        int highLowByte = local % 2;
        int tempMem = MEMORY[local / 2][highLowByte] & 0x000FF;
        if(tempMem & signMask == 0){
            temp.REGS[Source1Reg] = tempMem & 0x000FF;
        }
        else{
            temp.REGS[Source1Reg] = tempMem | 0x0FF00;
        }
    }
    temp = setCC(temp, Source1Reg);
    return temp;
}

System_Latches setCC(System_Latches update, int Source1Reg){
    int checkBITS = (update.REGS[Source1Reg] & 0x0FFFF);
    int negPos = (checkBITS & 0x08000) >> 15;
    if(negPos == 1){
        update.N = 1;
        update.P = 0;
        update.Z = 0;
    }
    else{
        if(checkBITS == 0){
            update.N = 0;
            update.P = 0;
            update.Z = 1;
        }
        else{
            update.N = 0;
            update.P = 1;
            update.Z = 0;
        }
    }
    return update;
}
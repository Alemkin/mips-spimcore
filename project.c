#include "spimcore.h"

/* ALU */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    //switching for passed in operation
    switch ((int)ALUControl){
        case 0: //add
            *ALUresult = A + B;
            break;
        case 1: //sub
            *ALUresult = A - B;
            break;
        case 2: //slt
            if (A < B) {
                *ALUresult = 1;
            }
            else {
                *ALUresult = 0;
            }
            break;
        case 3: //sltu
            if (A < B) {
                *ALUresult = 1;
            }
            else {
                *ALUresult = 0;
            }
            break;
        case 4: //and
            *ALUresult = A & B;
            break;
        case 5: //or
            *ALUresult = A | B;
            break;
        case 6: //shl not setting alu
            B << 16;
            break;
        case 7: //bitwise not, set all to complement in A
            *ALUresult = ~A;
            break;
    }
    //set zero to 1 if alu is 0
    if(*ALUresult == 0) {
        *Zero = 1;
    }
	else {
        *Zero = 0;
	}
}

/* instruction fetch */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    // setting the pc to relative
   //and checking for word alignment
	PC -= 16384;
	if(PC % 4 != 0) {
        return 1; //false
	}
	PC /= 4; //reset for alignment
    *instruction = Mem[PC + 4096]; //find instruction in mem by PC offset by memory size
	return 0;
}


/* instruction partition */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
//http://glenngeenen.be/bitwise-operators-2/
//http://www.cprogramming.com/tutorial/bitwise_operators.html

    *op = (instruction >> 26) & 0x0000003f;
    //op; 6 bits; 26-31

    *r1 = (instruction >> 21) & 0x1f;
    //r1; 5 bits;  21-25

    *r2 = (instruction >> 16) & 0x1f;
    //r2 = 16-20

    *r3 = (instruction >> 11) & 0x1f;
    //r3 = 11 - 15

    *funct = instruction & 0x0000003f;
    // funct; 6 bits back from 0; 0-5

    *offset = instruction & 0x0000ffff;
    //offset; 16 bits back from 0; 0-15

    *jsec = instruction & 0x03ffffff;
    //jsec; 26 bits back from 0; 0-25
}



/* instruction decode */
int instruction_decode(unsigned op,struct_controls *controls)
{
    // switch by op to set the struct
	if(op == 0) {
        //r-type
		controls->RegWrite = 1;
		controls->MemWrite = 0;
		controls->MemtoReg = 0;
		controls->Branch = 2;
		controls->MemRead = 0;
		controls->RegDst = 1;
		controls->ALUSrc = 0;
		controls->ALUOp = 7;
		controls->Jump = 0;
		return 0;
	} else if (op == 2) {
        // j-type
		controls->RegWrite = 0;
		controls->Jump = 1;
		controls->ALUOp = 0;
		controls->Branch = 0;
		controls->MemRead = 0;
		controls->MemtoReg = 2;
		controls->ALUSrc = 0;
		controls->MemWrite = 0;
		controls->RegDst = 2;
		return 0;
	}
		// change controls as necessary per op
		switch(op) {
		case 4: //beq
		    controls->RegDst = 2;
            controls->Jump = 0;
            controls->Branch = 1;
            controls->RegWrite = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 2;
            controls->MemWrite = 0;
            controls->ALUSrc = 0;
            controls->ALUOp = 1;
			break;
		case 8: //adi
		    controls->RegDst = 0;
            controls->Jump = 0;
            controls->ALUSrc = 1;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->RegWrite = 1;
            controls->MemtoReg = 0;
            controls->ALUOp = 0;
            controls->MemWrite = 0;
		    break;
		case 9: //adiu

            controls->MemWrite = 0;
            controls->ALUSrc = 1;
            controls->Branch = 0;
            controls->RegDst = 0;
            controls->Jump = 0;
            controls->MemtoReg = 0;
            controls->ALUOp = 2;
            controls->MemRead = 0;
            controls->RegWrite = 1;
			break;
		case 10: //slti
			controls->RegDst = 0;
            controls->Jump = 0;
            controls->MemtoReg = 0;
            controls->ALUOp = 2;
            controls->MemWrite = 0;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            controls->Branch = 0;
            controls->MemRead = 0;
            break;
		case 11: //sltiu
			controls->RegDst = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 0;
            controls->ALUOp = 3;
            controls->MemWrite = 0;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
			break;
		case 15: //lui
		    controls->RegDst = 0;
            controls->Jump = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 0;
            controls->ALUOp = 6;
            controls->MemWrite = 0;
            controls->ALUSrc = 1;
            controls->Branch = 0;
            controls->RegWrite = 1;
			break;
		case 35: //lw
		    controls->RegDst = 0;
            controls->Jump = 0;
            controls->MemRead = 1;
            controls->MemtoReg = 1;
            controls->ALUOp = 0;
            controls->MemWrite = 0;
            controls->ALUSrc = 1;
            controls->Branch = 0;
            controls->RegWrite = 1;
			break;
		case 43: //sw
		    controls->RegDst = 2;
            controls->Jump = 0;
            controls->ALUSrc = 1;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 2;
            controls->ALUOp = 0;
            controls->MemWrite = 1;
            controls->RegWrite = 0;
			break;
		default:
			return 1; //returning false, no op found
		}
	return 0; //returning true
}

/* Read Register */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    //Storing the data into the data variables from their respective registers
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}


/* Sign Extend */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    //setting sign to the bit representing sign in offset
    unsigned sign = offset >> 15;

    //if negative, make 32 bit from 16
    if(sign == 1) {
        *extended_value = offset | 0xFFFF0000;
        /*1111 1111 1111 1111 0000 0000 0000 0000 */
    } else //positive, keep 16bit
    {
        *extended_value = offset & 0x0000ffff;
        /* 1111 1111 1111 1111 */
    }
}

/* ALU operations */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    // Call the function ALU if the op is between 0 and 6
	if(ALUOp >= 0 && ALUOp < 7) {
        if (ALUSrc == 1) {
            data2 = extended_value;
        }
		ALU(data1, data2, ALUOp, ALUresult, Zero);
        return 0;
	}
	if(ALUOp == 7) {
		unsigned ALUcontinue;
		//find the correct function of type 7, or R type
		switch(funct) {
            case 6:
                ALUcontinue = 6;
                break;
            case 32:
                ALUcontinue = 0;
                break;
            case 34:
                ALUcontinue = 1;
                break;
            case 36:
                ALUcontinue = 4;
                break;
            case 37:
                ALUcontinue = 5;
                break;
            case 39:
                ALUcontinue = 7;
                break;
            case 42:
                ALUcontinue = 2;
                break;
            case 43:
                ALUcontinue = 3;
                break;
            default:
                return 1; // return false if no function found
		}
		//extend if alu source says so
		if (ALUSrc == 1) {
            data2 = extended_value;
        }
		// call ALU when the control is properly set
		ALU(data1, data2, ALUcontinue, ALUresult, Zero);
		return 0; //return true
	}
	else {
        return 1;
	}
	return 0; //true
}

/* Read / Write Memory */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    //grab the address and shift ALU 2 to get the proper word alignment
    unsigned addressToUse = ALUresult >> 2;
    //if read is marked, read the address in memory to the memdata
	if(MemRead == 1) {
        //checking for correct word shift
		if(ALUresult % 4 != 0) {
            return 1; //false if
		}
		*memdata = Mem[addressToUse];
	}
    //if write is marked, write the address in memory to data2
	if(MemWrite == 1) {
        //checking for correct word shift
		if(ALUresult % 4 != 0) {
            return 1; //false if
		}
		Mem[addressToUse] = data2;
	}

	return 0;
}


/* Write Register */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    //if write and mem are 1, memdata is used
    if (RegWrite == 1 && MemtoReg == 1) {
        if (RegDst == 0) {
            Reg[r2] = memdata;
        }
        else if (RegDst == 1){
            Reg[r3] = memdata;
        }
    }
    //if write is 1 nd mem is 0, ALUresult is used
    if (RegWrite == 1 && MemtoReg == 0){
        if (RegDst == 0) {
            Reg[r2] = ALUresult;
        }
        else if(RegDst == 1) {
            Reg[r3] = ALUresult;
        }
    }
}

/* PC update */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    *PC += 4;
	// if we need to branch, and Zero found
	if (Branch == 1) {
		if(Zero == 1) {
			*PC = (extended_value << 2);
		}
	}

	//if there is a jump, set PC to the jsec shifted 2 bits
	if(Jump == 1) {
		*PC = (jsec << 2);
	}

}


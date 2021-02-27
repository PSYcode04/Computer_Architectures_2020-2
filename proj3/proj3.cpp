#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <bitset>
#include <cmath>
#include <sstream>
#define MAXSIZE 65536
using namespace std;

int unknown = 0;
int Checksum = 0;

class Inst_Ram
{
public:
    unsigned char inst[MAXSIZE];
    int pc_add;

    Inst_Ram() {
        memset(inst, 255, sizeof(inst));
    }
};

///////////////////////////////////////////////////////////////////////
class Regi
{
public:
    int reg[33] = {0, }; // register 32 + pc 1
    // pc = reg[32]

    void write_value(int dist, int value)
	{
		reg[dist] = value;
	}
};
///////////////////////////////////////////////////////////////////////
class Data_mem
{
public:
    unsigned char mem[MAXSIZE];

    Data_mem() {
        memset(mem, 255, sizeof(mem));
    }

};
///////////////////////////////////////////////////////////////////////
Regi reg;
Data_mem d_mem;
///////////////////////////////////////////////////////////////////////
class InputFile
{
public:
    long size = 0;
    Inst_Ram inst;

    void input(const char* _fileName)
    {            
        std::ifstream input ( _fileName, std::ifstream::binary);

        input.seekg(0,std::ifstream::end);
        size=input.tellg();
        input.seekg(0);

        input.read((char*)&inst.inst[0], size);
        input.close();
    }
};
///////////////////////////////////////////////////////////////////////
class IF_ID // pc+4, inst 저장
{
public:
    int pc_0 = 0; // current PC
    int pc_4 = 0; // pc+4
    string inst = "00000000000000000000000000000000"; // instruction 초기값은 nop
    
    bool IF_ID_Write = true;
};

class ID_EX
{
public:
    /* stored data*/
    int pc_0 = 0; // current PC
    int pc_4 = 0; // pc+4

    int Reg_rs_value = 0;
    int Reg_rt_value = 0;
    int imm1 = 0; // sign-extend
    int imm2 = 0; // zero-extend
    int rs_num = 0; // inst[25:21]
    int rt_num = 0; // inst[20:16]
    int rd_num = 0; // inst[15:11]

    bool stall = false;

    /*control signal*/
    bool MemToReg = false; // lw (memory의 data를 reg에 저장)
    bool RegWrite = false; // lw, R-type
    bool MemWrite = false; // sw (register의 data를 memory에 저장)
    bool MemRead = false; // lw (memmory data read)

    string ALUControl = ""; // functcode
    bool Branch = false; // beq & bne (rs, rt 데이터를 비교해서 true or false 저장)
    bool RegDst = false; // rd register의 사용 여부 (true면 rd사용 - R type)
    bool ALUSrc = false; // lw, sw (imm1 값을 ALU연산에 쓰는가?)
    // bool jump = false; // jump는 IF stage에서 판단

};

class EX_MEM
{
public:
    /* stored data*/
    int pc_0 = 0; // current PC
    int pc_4 = 0; // pc+4
    int regDst_num = 0; // inst[15:11] or inst[20:16] => rt, rd 중에 사용하는 것
    int ALU_result = 0; // ALU에서 연산한 결과
    int Reg_rt_value = 0;


    bool stall = false;
    /*control signal*/
    bool MemToReg = false; // lw (memory의 data를 reg에 저장)
    bool RegWrite = false; // lw, R-type
    bool MemWrite = false; // sw (register의 data를 memory에 저장)
    bool MemRead = false; // lw (memmory data read)
};

class MEM_WB
{
public:
    /* stored data*/
    int pc_0 = 0; // current PC
    int pc_4 = 0; // pc+4
    int regDst_num = 0; // inst[15:11] or inst[20:16] => rt, rd 중에 사용하는 것 (sw명령어일 경우 rt 레지스터에 MEM_read_data를 저장해야함)
    int ALU_result = 0; // ALU에서 연산한 결과(R-type) 즉 memory에 안쓰는 경우!
    int MEM_read_data = 0; // lw로 memory에서 읽은 데이터

    bool stall = false;
    /*control signal*/
    bool MemToReg = false; // lw (memory의 data를 reg에 저장)
    bool RegWrite = false; // lw, R-type
};

/*declate pipeline register*/
IF_ID IF_ID_register;
ID_EX ID_EX_register;
EX_MEM EX_MEM_register;
MEM_WB MEM_WB_register;

//////////////////////////////OPCODE//////////////////////////////
map<string, int> opcode_map()
{
    map<string, int> opcode;
    opcode.insert(make_pair("001000", 1));  // addi
    opcode.insert(make_pair("001100", 2));  // andi
    opcode.insert(make_pair("000100", 3));  // beq
    opcode.insert(make_pair("000101", 4));  // bne
    opcode.insert(make_pair("100011", 5)); // lw
    opcode.insert(make_pair("001101", 6)); // ori
    opcode.insert(make_pair("001010", 7)); // slti
    opcode.insert(make_pair("001111", 8)); // lui
    opcode.insert(make_pair("101011", 9)); // sw
//    opcode.insert(make_pair("000010", 10)); // j

    return opcode;
}

//////////////////////////////FUNCTCODE//////////////////////////////
map<string, int> funct_map()
{
    map<string, int> funct;
    funct.insert(make_pair("100000", 1));   // add
    funct.insert(make_pair("100100", 2));   // and
    funct.insert(make_pair("100101", 3));  // or
    funct.insert(make_pair("101010", 4));  // slt
    funct.insert(make_pair("100010", 5));  // sub
    funct.insert(make_pair("000000", 6));  // nop

    return funct;
}
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
string HexToBin(Inst_Ram& _inst, int n)
{
    string dis_inst;
    int i = 4*n;

    bitset<8> bit1 = _inst.inst[i];
    bitset<8> bit2 = _inst.inst[i+1];
    bitset<8> bit3 = _inst.inst[i+2];
    bitset<8> bit4 = _inst.inst[i+3];


    dis_inst.append(bit1.to_string());
    dis_inst.append(bit2.to_string());
    dis_inst.append(bit3.to_string());
    dis_inst.append(bit4.to_string());

    return dis_inst;
}

int BinToDec(string regi_num)
{
    int e = 1;
    int result = 0;
    for(int i = regi_num.length() - 1; i >= 0; i--)
    {
        for(int j = 0; j < regi_num.length()-i-1; j++)
            e*=2;
        if(regi_num[i] == '1')
            result += e;
        e = 1;
    }

    return result;
}

//BinToDec signed value (sign-extension)
int BinToSignedDec (string imm)
{
    
    int i = 0;
    int temp;
    int decimal = 0;

    for(i=1; i<imm.length(); i++)
    {
        temp = (int)pow(2, imm.length()-1-i);
        if(imm[i] == '1')
            decimal = decimal + (temp * 1);
    }
    
    if(imm[0] == '1')
    {
        decimal = pow(2, imm.length()-1) - decimal;
        decimal = decimal * (-1);
    }
    return decimal;
}
//BinToDec unsigned value (unsiged-extension)
int BinToUnsignedDec (string imm)
{
    
    int i = 0;
    int temp;
    int decimal = 0;

    for(i=0; i<imm.length(); i++)
    {
        temp = (int)pow(2, imm.length()-1-i);
        if(imm[i] == '1')
            decimal = decimal + (temp * 1);
    }
        
    return decimal;
}

void SaveToMem(int reg_value, int add_index)
{
    
    d_mem.mem[add_index] = reg_value >> 24;
    d_mem.mem[add_index+1] = reg_value >> 16;
    d_mem.mem[add_index+2] = reg_value >> 8;
    d_mem.mem[add_index+3] = reg_value;
}

int LoadDataMem(int add_index)
{
    int readvalue = 0;
    readvalue = readvalue | d_mem.mem[add_index] << 24;
    readvalue = readvalue | d_mem.mem[add_index+1] << 16;
    readvalue = readvalue | d_mem.mem[add_index+2] << 8;
    readvalue = readvalue | d_mem.mem[add_index+3];

    return readvalue;
}
// void LoadToReg(int add_index, int reg_num)
// {
//     // initialize register
//     reg.reg[reg_num] = 0;
//     reg.reg[reg_num] = reg.reg[reg_num] | d_mem.mem[add_index] << 24;
//     reg.reg[reg_num] = reg.reg[reg_num] | d_mem.mem[add_index+1] << 16;
//     reg.reg[reg_num] = reg.reg[reg_num] | d_mem.mem[add_index+2] << 8;
//     reg.reg[reg_num] = reg.reg[reg_num] | d_mem.mem[add_index+3];
// }

void NewPCtoJ(InputFile& _i, Regi& _reg, int i)
{
    int int_inst = 0;
    i = i*4;
    int new_f = 15 << 28;
    int add = 0, new_pc = 0;
    new_pc = _reg.reg[32] & new_f;

    int_inst = int_inst | _i.inst.inst[i] << 24;
    int_inst = int_inst | _i.inst.inst[i+1] << 16;
    int_inst = int_inst | _i.inst.inst[i+2] << 8;
    int_inst = int_inst | _i.inst.inst[i+3];

    add = int_inst << 2;
    add = add & ~(new_f);
    _reg.reg[32] = new_pc | add;
}
////////////////////////////////////////////////////////////////////////
void IFstage(InputFile& _i, map<string, int> opcode, map<string, int> funct, int n);
void IDstage(map<string, int> opcode, map<string, int> funct);
void EXstage( map<string, int> opcode, map<string, int> funct);
void MEMstage();
void WBstage();
void flush_IF_ID();
void flush_ID_EX();
void flush_EX_MEM();
void flush_MEM_WB();
////////////////////////////////////////////////////////////////////////
void flush_IF_ID()
{
    IF_ID_register.inst = "00000000000000000000000000000000"; // instruction 초기값은 nop
    
    IF_ID_register.IF_ID_Write = true;
}
void flush_ID_EX()
{

    ID_EX_register.MemToReg = false; // lw (memory의 data를 reg에 저장)
    ID_EX_register.RegWrite = false; // lw, R-type
    ID_EX_register.MemWrite = false; // sw (register의 data를 memory에 저장)
    ID_EX_register.MemRead = false; // lw (memmory data read)

    ID_EX_register.ALUControl = ""; // functcode
    ID_EX_register.Branch = false; // beq & bne (rs, rt 데이터를 비교해서 true or false 저장)
    ID_EX_register.RegDst = false; // rd register의 사용 여부 (true면 rd사용 - R type)
    ID_EX_register.ALUSrc = false; // lw, sw (imm1 값을 ALU연산에 쓰는가?)
}

void flush_EX_MEM()
{
    EX_MEM_register.MemToReg = false;
    EX_MEM_register.RegWrite = false;
    EX_MEM_register.MemWrite = false;
    EX_MEM_register.MemRead = false;
}

void flush_MEM_WB()
{
    MEM_WB_register.MEM_read_data = 0;

    MEM_WB_register.MemToReg = false;
    MEM_WB_register.RegWrite =false;
}

////////////////////////////////////////////////////////////////////////
// opcode로 inst 판단
void IFstage(InputFile& _i, map<string, int> opcode, map<string, int> funct, int n) // instruction 종류 판단 (opcode)
{
    int i = n/4;
    printf("inst num: %d\n", i);
    string inst = HexToBin(_i.inst, i); // Hex instruction을 Binary 형태로 inst string에 저장
    string op = inst.substr(0,6); // op코드부분

    if(!IF_ID_register.IF_ID_Write)
    {
        IF_ID_register.pc_0 = IF_ID_register.pc_4;
        IF_ID_register.inst = "00000000000000000000000000000000";
        IF_ID_register.IF_ID_Write = true;
        return;
    }

    IF_ID_register.pc_4 += 4; // pc+4 업데이트!

    // pc값만 저장하고 jump일 경우 pc+4
    if(!op.compare("000010")) // jump일 경우
    {
        // IF_ID flush (명령어 초기화)
        // PC+4값은 다음 명령어 주소로!
        int int_inst = 0;
        int new_f = 15 << 28;
        int add = 0, new_pc = 0;
        
        IF_ID_register.inst = "00000000000000000000000000000000"; // flush(nop)

        new_pc = IF_ID_register.pc_4 & new_f;
        int_inst = int_inst | _i.inst.inst[i] << 24;
        int_inst = int_inst | _i.inst.inst[i+1] << 16;
        int_inst = int_inst | _i.inst.inst[i+2] << 8;
        int_inst = int_inst | _i.inst.inst[i+3];

        add = int_inst << 2;
        add = add & ~(new_f);
        IF_ID_register.pc_4 = new_pc | add; // pc_4 update
    }
    else // jump가 아닐 경우
    {
        IF_ID_register.inst = inst; // inst에 현재 instruction 저장
        // IF_ID_register.pc_4 += 4; // pc값 업데이트
        IF_ID_register.pc_0 = IF_ID_register.pc_4 - 4; // 현재 명령어주소 (PC)
    }
}

////////////////////////////////////////////////////////////////////////
// rs, rt 판단, branch prediction and detection
void IDstage(map<string, int> opcode, map<string, int> funct)
{
    if(ID_EX_register.MemRead)
    {
        if(ID_EX_register.rt_num == BinToDec(IF_ID_register.inst.substr(6,5)) || ID_EX_register.rt_num == BinToDec(IF_ID_register.inst.substr(11,5)))
        {
            IF_ID_register.IF_ID_Write = false;
            // ID_EX_register.stall = true;
        }
    }

    flush_ID_EX();
    
    string op = IF_ID_register.inst.substr(0,6);
    string fun = IF_ID_register.inst.substr(26,6);

    ID_EX_register.pc_0 = IF_ID_register.pc_0;
    ID_EX_register.pc_4 = IF_ID_register.pc_4;

    ID_EX_register.rs_num = BinToDec(IF_ID_register.inst.substr(6,5));
    ID_EX_register.rt_num = BinToDec(IF_ID_register.inst.substr(11,5));
    ID_EX_register.rd_num = BinToDec(IF_ID_register.inst.substr(16,5));

    ID_EX_register.imm1 = BinToSignedDec(IF_ID_register.inst.substr(16,16)); // sign-extend
    ID_EX_register.imm2 = BinToUnsignedDec(IF_ID_register.inst.substr(16,16)); // zero-extend


    ID_EX_register.Reg_rs_value = reg.reg[ID_EX_register.rs_num];

    // printf("Checksum before: 0x%08x\n", Checksum);
    // printf("rs value: 0x%08x\n", ID_EX_register.Reg_rs_value);
    Checksum = (Checksum << 1 | (Checksum >> 31) & 1) ^ ID_EX_register.Reg_rs_value;

    
    /*HAZARD*/
    // EX/MEM hazard (rs)
    if((EX_MEM_register.RegWrite && EX_MEM_register.regDst_num != 0) && (ID_EX_register.rs_num == EX_MEM_register.regDst_num)) // 현재 ID stage의 rs register == 현재 EX stage의 rd/rt register
    {
        ID_EX_register.Reg_rs_value = EX_MEM_register.ALU_result;
    } 
    else if ((MEM_WB_register.RegWrite && MEM_WB_register.regDst_num != 0) && (ID_EX_register.rs_num == MEM_WB_register.regDst_num))
    {
        ID_EX_register.Reg_rs_value = MEM_WB_register.ALU_result;
    }
    else
    {
        ID_EX_register.Reg_rs_value = reg.reg[ID_EX_register.rs_num];
    }
    
    // EX/MEM hazard (rt)
    if((EX_MEM_register.RegWrite && EX_MEM_register.regDst_num != 0) && (ID_EX_register.rt_num == EX_MEM_register.regDst_num)) // 현재 ID stage의 rt register == 현재 EX stage의 rd/rt register
    {
        ID_EX_register.Reg_rt_value = EX_MEM_register.ALU_result;
    }
    else if ((MEM_WB_register.RegWrite && MEM_WB_register.regDst_num != 0) && (ID_EX_register.rt_num == MEM_WB_register.regDst_num))
    {
        ID_EX_register.Reg_rt_value = MEM_WB_register.ALU_result;
    }
    else
    {
        ID_EX_register.Reg_rt_value = reg.reg[ID_EX_register.rt_num];
    }

    // Load-Use hazard
    // if(EX_MEM_register.MemRead)
    // {
    //     if(ID_EX_register.rs_num == EX_MEM_register.regDst_num || ID_EX_register.rt_num == EX_MEM_register.regDst_num)

    // }

    // printf("Checksum before: 0x%08x\n", Checksum);
    // printf("rs value: 0x%08x\n", ID_EX_register.Reg_rs_value);
    // Checksum = (Checksum << 1 | (Checksum >> 31) & 1) ^ ID_EX_register.Reg_rs_value;

    if(!op.compare("000000")) // R-type instruction
    {
        // unknown instruction (모르는 funct코드이면)
        if(funct.find(fun) == funct.end()) {
            return; // unknown instruction, jump
        }
        /*control signal*/
        ID_EX_register.RegWrite = true; // lw, R-type
        ID_EX_register.ALUControl = fun; // funct code
        ID_EX_register.RegDst = true; // rd register의 사용 여부 (true면 rd사용 - R type)
    }
    else // I-type instruction (lw, sw, beq, bne extra)
    {
        ID_EX_register.ALUControl = op;
        if(opcode.find(op) == opcode.end()) {
            return; // unknown instruction, jump
        }
        else {
            switch(opcode.at(op)) {
            case 1: // addi
                ID_EX_register.RegWrite = true;
                ID_EX_register.ALUSrc = true;
                break;
            case 2: // andi
                ID_EX_register.RegWrite = true;
                ID_EX_register.ALUSrc = true;
                break;
            case 3: // beq
                break;
            case 4: // bne
                break;
            case 5: // lw
                ID_EX_register.MemToReg = true;
                ID_EX_register.RegWrite = true;
                ID_EX_register.MemRead = true;
                ID_EX_register.ALUSrc = true;
                break;
            case 6: // ori
                ID_EX_register.RegWrite = true;
                ID_EX_register.ALUSrc = true;
                break;
            case 7: // slti
                ID_EX_register.RegWrite = true;
                ID_EX_register.ALUSrc = true;
                break;
            case 8: // lui
                ID_EX_register.RegWrite = true;
                ID_EX_register.ALUSrc = true;
                break;
            case 9: // sw
                ID_EX_register.MemWrite = true;
                ID_EX_register.ALUSrc = true;
                break;
            }
        }
    }

}

////////////////////////////////////////////////////////////////////////
// ALU계산 (ALUcontrol이 funct코드에 따라서 계산, ALUcontrol = "" 이면 주소연산임!)
void EXstage(map<string, int> opcode, map<string, int> funct)
{
    // cout << "EX" << endl;
    if(ID_EX_register.stall)
    {
        EX_MEM_register.stall =true;
        ID_EX_register.stall = false;
        return;
    }
    flush_EX_MEM();
    int address = 268435456;
    EX_MEM_register.pc_0 = ID_EX_register.pc_0;
    EX_MEM_register.pc_4 = ID_EX_register.pc_4;


    if(ID_EX_register.RegDst) // rd 사용하는 경우
    {
        EX_MEM_register.regDst_num = ID_EX_register.rd_num;
    }
    else
    {
        EX_MEM_register.regDst_num = ID_EX_register.rt_num;
        EX_MEM_register.Reg_rt_value = ID_EX_register.Reg_rt_value;
    }
    
    EX_MEM_register.MemToReg = ID_EX_register.MemToReg;
    EX_MEM_register.RegWrite = ID_EX_register.RegWrite;
    EX_MEM_register.MemWrite = ID_EX_register.MemWrite;
    EX_MEM_register.MemRead = ID_EX_register.MemRead;
    
    if(ID_EX_register.RegDst) // R-type인 경우
    {
        if(funct.find(ID_EX_register.ALUControl) == funct.end()) {
            return;
        } else {
        switch(funct.at(ID_EX_register.ALUControl)) {
        case 1: // add
            EX_MEM_register.ALU_result = ID_EX_register.Reg_rs_value + ID_EX_register.Reg_rt_value;
            break;
        case 2: // and
            EX_MEM_register.ALU_result = ID_EX_register.Reg_rs_value & ID_EX_register.Reg_rt_value;
            break;
        case 3: // or
            EX_MEM_register.ALU_result = ID_EX_register.Reg_rs_value | ID_EX_register.Reg_rt_value;
            break;
        case 4: // slt
            if(ID_EX_register.Reg_rs_value < ID_EX_register.Reg_rt_value)
                EX_MEM_register.ALU_result = 1;
            else
                EX_MEM_register.ALU_result = 0;
            break;
        case 5: // sub
            EX_MEM_register.ALU_result = ID_EX_register.Reg_rs_value - ID_EX_register.Reg_rt_value;
            break;
        case 6: // nop
            EX_MEM_register.ALU_result = 0;
            break;
        }
        }
    }
    else // R-type이 아닌 경우
    {
        if(opcode.find(ID_EX_register.ALUControl) == opcode.end()) {
            return;
        } else {

        switch(opcode.at(ID_EX_register.ALUControl)) {
        case 1: // addi
            EX_MEM_register.ALU_result = ID_EX_register.Reg_rs_value + ID_EX_register.imm1;
            break;
        case 2: // andi
            EX_MEM_register.ALU_result = ID_EX_register.Reg_rs_value & ID_EX_register.imm2;
            break;
        case 3: // beq
            break;
        case 4: // bne
            break;
        case 5: // lw
            EX_MEM_register.ALU_result = (address ^ ID_EX_register.Reg_rs_value) + ID_EX_register.imm1;
            cout << "lw result: " << EX_MEM_register.ALU_result << endl;
            break;
        case 6: // ori
            EX_MEM_register.ALU_result = ID_EX_register.Reg_rs_value | ID_EX_register.imm2;
            break;
        case 7: // slti
            if(ID_EX_register.Reg_rs_value < ID_EX_register.imm1)
                EX_MEM_register.ALU_result = 1;
            else
                EX_MEM_register.ALU_result = 0;
            break;
        case 8: // lui
            EX_MEM_register.ALU_result = ID_EX_register.Reg_rs_value | (ID_EX_register.imm1 << 16);
            break;
        case 9: // sw
            EX_MEM_register.ALU_result = (address ^ ID_EX_register.Reg_rs_value) + ID_EX_register.imm1;
            break;
        }
        }
    }
    
}

////////////////////////////////////////////////////////////////////////
// use DataMemory
void MEMstage()
{
    // cout << "MEM" << endl;
    if(EX_MEM_register.stall)
    {
        MEM_WB_register.stall =true;
        EX_MEM_register.stall = false;
        return;
    }
    flush_MEM_WB();

    MEM_WB_register.pc_0 = EX_MEM_register.pc_0;
    MEM_WB_register.pc_4 = EX_MEM_register.pc_4;
    MEM_WB_register.regDst_num = EX_MEM_register.regDst_num; // rd 혹은 rt 복사
    MEM_WB_register.ALU_result = EX_MEM_register.ALU_result;

    MEM_WB_register.MemToReg = EX_MEM_register.MemToReg;
    MEM_WB_register.RegWrite = EX_MEM_register.RegWrite;

    if(EX_MEM_register.MemRead) // lw인 경우
    {
        MEM_WB_register.MEM_read_data = LoadDataMem(EX_MEM_register.ALU_result);
    }
    if(EX_MEM_register.MemWrite) // sw인 경우
    {
        SaveToMem(EX_MEM_register.Reg_rt_value, EX_MEM_register.ALU_result);
    }
}

////////////////////////////////////////////////////////////////////////
// write regFile
void WBstage()
{
    if(MEM_WB_register.stall)
    {
        MEM_WB_register.stall = false;
        return;
    }
    if(MEM_WB_register.MemToReg && EX_MEM_register.RegWrite) // lw (MEM_read_data)
    {
        cout << "regWrite: " << MEM_WB_register.MEM_read_data << endl;
        reg.reg[MEM_WB_register.regDst_num] = 0;
        reg.reg[MEM_WB_register.regDst_num] = MEM_WB_register.MEM_read_data;
    }
    else if(MEM_WB_register.RegWrite)
    {
        reg.reg[MEM_WB_register.regDst_num] = 0;
        reg.reg[MEM_WB_register.regDst_num] = MEM_WB_register.ALU_result;
    }
}


int main(int argc, char **argv)
{
    InputFile input;
    input.input(argv[1]);
    map<string, int> opcode = opcode_map();
    map<string, int> funct = funct_map();
    int numOfInst = 0; // 실행할 수 있는 instruction 수

    if(argc == 3) // 아무것도 출력하지 말것
    {
        for(int i = 0; i < stoi(argv[2]); i++) // 입력한 cycle 수 많큼 반복문 수행
        {
            WBstage();
            MEMstage();
            EXstage(opcode, funct);
            IDstage(opcode, funct);
            IFstage(input, opcode, funct, IF_ID_register.pc_4);
        }
        return 0;
    }
    else if(argc == 4) // reg
    {

        for(int i = 0; i < stoi(argv[2]); i++) // 입력한 cycle 수 많큼 반복문 수행
        {
            WBstage();
            MEMstage();
            EXstage(opcode, funct);
            IDstage(opcode, funct);
            IFstage(input, opcode, funct, IF_ID_register.pc_4);
        }

        //print register
        printf("Checksum: 0x%08x\n", Checksum);
        for(int i = 0; i < 32; i++)
        {
            printf("$%d: 0x%08x\n", i, reg.reg[i]);
        }
        printf("PC: 0x%08x\n", IF_ID_register.pc_0);
    }
    else if(argc == 6) // mem
    {
        for(int i = 0; i < stoi(argv[2]); i++) // 입력한 cycle 수 많큼 반복문 수행
        {
            WBstage();
            MEMstage();
            EXstage(opcode, funct);
            IDstage(opcode, funct);
            IFstage(input, opcode, funct, IF_ID_register.pc_4);
        }

        string add = argv[4];
        int add_mem = 0;
        add = add.substr(5, 5);
        
        stringstream convert(add);
        convert >> hex >> add_mem;

        // mem 
        for(int i = add_mem; i < add_mem+(4*stoi(argv[5])); i+=4)
        {
            printf("0x%02x%02x%02x%02x\n", d_mem.mem[i], d_mem.mem[i+1], d_mem.mem[i+2], d_mem.mem[i+3]);
        }

    }
    return 0;
}
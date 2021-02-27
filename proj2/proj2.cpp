#include <cstdio>
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
    opcode.insert(make_pair("000010", 10)); // j

    return opcode;
}

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

void SaveToMem(int reg_value, Data_mem& _data_mem, int add_index)
{
    _data_mem.mem[add_index] = reg_value >> 24;
    _data_mem.mem[add_index+1] = reg_value >> 16;
    _data_mem.mem[add_index+2] = reg_value >> 8;
    _data_mem.mem[add_index+3] = reg_value;
}

void LoadToReg(Data_mem& _data_mem, int add_index, int reg_num, Regi& _reg)
{
    // initialize register
    _reg.reg[reg_num] = 0;
    _reg.reg[reg_num] = _reg.reg[reg_num] | _data_mem.mem[add_index] << 24;
    _reg.reg[reg_num] = _reg.reg[reg_num] | _data_mem.mem[add_index+1] << 16;
    _reg.reg[reg_num] = _reg.reg[reg_num] | _data_mem.mem[add_index+2] << 8;
    _reg.reg[reg_num] = _reg.reg[reg_num] | _data_mem.mem[add_index+3];

}

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

void simulator(InputFile& _i, Regi& _reg, Data_mem& _data_mem, map<string, int> opcode, map<string, int> funct, int n)
{
    int i = n/4;
    string inst = HexToBin(_i.inst, i);
    string op = inst.substr(0,6);
    int rs, rt, rd, sa, imm1, imm2 = 0;
    int address = 268435456;
    int save_pc = 0;

    if(!op.compare("000000"))
    {
        string funct_code = inst.substr(26,6);
        rs = BinToDec(inst.substr(6,5));
        rt = BinToDec(inst.substr(11,5));
        rd = BinToDec(inst.substr(16,5));
        sa = BinToDec(inst.substr(21,5));

        if(funct.find(funct_code) == funct.end()) {
            printf("unknown instruction\n");
            unknown = 1;
        }
        else {
            switch(funct.at(funct_code)) {
            case 1: // add
                _reg.write_value(rd, _reg.reg[rs]+_reg.reg[rt]);
                break;
            case 2: // and
                _reg.write_value(rd, _reg.reg[rs] & _reg.reg[rt]);
                break;
            case 3: //or
                _reg.write_value(rd, _reg.reg[rs] | _reg.reg[rt]);
                break;
            case 4: // slt
                if(_reg.reg[rs] < _reg.reg[rt])
                    _reg.write_value(rd, 1);
                else
                    _reg.write_value(rd, 0);
                break;
            case 5: // sub
                _reg.write_value(rd, _reg.reg[rs]-_reg.reg[rt]);
                break;
            case 6: // nop
                break;
            }
        }    
    }
    else
    {
        imm1 = BinToSignedDec(inst.substr(16,16));
        imm2 = BinToUnsignedDec(inst.substr(16,16));
        rs = BinToDec(inst.substr(6,5));
        rt = BinToDec(inst.substr(11, 5));
        if(opcode.find(op) == opcode.end()) {
            printf("unknown instruction\n");
            unknown = 1;
        }
        else {
            switch(opcode.at(op)) {
            case 1: // addi
                _reg.reg[rt] = _reg.reg[rs] + imm1;
                break;
            case 2: // andi
                _reg.reg[rt] = _reg.reg[rs] & imm2;
                break;
            case 3: // beq
                if(_reg.reg[rs] == _reg.reg[rt])
                {
                    _reg.reg[32] = _reg.reg[32] + (imm1 << 2);
                }
                break;
            case 4: // bne
                if(_reg.reg[rs] != _reg.reg[rt])
                {
                    _reg.reg[32] = _reg.reg[32] + (imm1 << 2);
                }
                break;
            case 5: // lw
                address = (address ^ _reg.reg[rs]) + imm1;
                LoadToReg(_data_mem, address, rt, _reg);
                break;
            case 6: // ori
                _reg.reg[rt] = _reg.reg[rs] | imm1;
                break;
            case 7: // slti
                if(_reg.reg[rs] < imm1)
                    _reg.write_value(rt, 1);
                else
                    _reg.write_value(rt, 0);
                break;
            case 8: // lui
                _reg.reg[rt] = _reg.reg[rs] | (imm1 << 16);
                break;
            case 9: // sw
                address = (address ^ _reg.reg[rs]) + imm1;
                SaveToMem(_reg.reg[rt], _data_mem, address);
                cout << "add: " << address << endl;
                break;
            case 10: // j
                NewPCtoJ(_i, _reg, i);
                break;
            }
        }
    }
        
}


int main(int argc, char **argv)
{   

    InputFile input;
    Regi reg;
    Data_mem d_mem;
    input.input(argv[1]);
    map<string, int> opcode = opcode_map();
    map<string, int> funct = funct_map();

    if(argc == 3)
    {
        
        for(int i = 0; i < stoi(argv[2]); i++)
        {
            reg.reg[32]+=4; // PC+4
            if(input.inst.inst[reg.reg[32]-4] == 255)
            {
                printf("unknown instruction\n");
                break;
            }
            simulator(input, reg, d_mem, opcode, funct, reg.reg[32]-4);
        }
    }
    else if(argc == 4) // reg
    {
        for(int i = 0; i < stoi(argv[2]); i++)
        {
            reg.reg[32]+=4; // PC+4            
            if(input.inst.inst[reg.reg[32]-4] == 255)
            {
                printf("unknown instruction\n");
                break;
            }
            simulator(input, reg, d_mem, opcode, funct, reg.reg[32]-4);
            if(unknown)
                break;
        }
        // register
        for(int i = 0; i < 32; i++)
        {
            printf("$%d: 0x%08x\n", i, reg.reg[i]);
        }
        printf("PC: 0x%08x\n", reg.reg[32]);
    }
    else if(argc == 6) // mem
    {
        
        for(int i = 0; i < stoi(argv[2]); i++)
        {
            reg.reg[32]+=4; // PC+4
            if(input.inst.inst[reg.reg[32]-4] == 255)
            {
                printf("unknown instruction\n");
                break;
            }
            simulator(input, reg, d_mem, opcode, funct, reg.reg[32]-4);
            if(unknown)
                break;
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
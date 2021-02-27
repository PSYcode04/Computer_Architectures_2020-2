#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <bitset>
#include <cmath>
using namespace std;

class InputFile
{
public:
    long size = 0;
    std::vector<unsigned char> buf;

    void input(const char* _fileName)
    {            
        std::ifstream input ( _fileName, std::ifstream::binary);

        input.seekg(0,std::ifstream::end);
        size=input.tellg();
        input.seekg(0);

        buf.resize(size);

        input.read( (char*)&buf[0], size);
        input.close();
    }
};

map<string, int> opcode_map()
{
    map<string, int> opcode;
    opcode.insert(make_pair("001000", 1));  // addi
    opcode.insert(make_pair("001001", 2));  // addiu
    opcode.insert(make_pair("001100", 3));  // andi
    opcode.insert(make_pair("000100", 4));  // beq
    opcode.insert(make_pair("000101", 5));  // bne
    opcode.insert(make_pair("100000", 6));  // lb
    opcode.insert(make_pair("100100", 7));  // lbu
    opcode.insert(make_pair("100001", 8));  // lh
    opcode.insert(make_pair("100101", 9));  // lhu
    opcode.insert(make_pair("001111", 10)); // lui
    opcode.insert(make_pair("100011", 11)); // lw
    opcode.insert(make_pair("001101", 12)); // ori
    opcode.insert(make_pair("101000", 13)); // sb
    opcode.insert(make_pair("001010", 14)); // slti
    opcode.insert(make_pair("001011", 15)); // sltiu
    opcode.insert(make_pair("101001", 16)); // sh
    opcode.insert(make_pair("101011", 17)); // sw
    opcode.insert(make_pair("001110", 18)); // xori
    opcode.insert(make_pair("000010", 19)); // j
    opcode.insert(make_pair("000011", 20)); // jal

    return opcode;
}

map<string, int> funct_map()
{
    map<string, int> funct;
    funct.insert(make_pair("100000", 1));   // add
    funct.insert(make_pair("100001", 2));   // addu
    funct.insert(make_pair("100100", 3));   // and
    funct.insert(make_pair("011010", 4));   // div
    funct.insert(make_pair("011011", 5));   // divu
    funct.insert(make_pair("001001", 6));   // jalr
    funct.insert(make_pair("001000", 7));   // jr
    funct.insert(make_pair("010000", 8));   // mfhi
    funct.insert(make_pair("010010", 9));   // mflo
    funct.insert(make_pair("010001", 10));  // mthi
    funct.insert(make_pair("010011", 11));  // mtlo
    funct.insert(make_pair("011000", 12));  // mult
    funct.insert(make_pair("011001", 13));  // multu
    funct.insert(make_pair("100111", 14));  // nor
    funct.insert(make_pair("100101", 15));  // or
    funct.insert(make_pair("000000", 16));  // sll
    funct.insert(make_pair("000100", 17));  // sllv
    funct.insert(make_pair("101010", 18));  // slt
    funct.insert(make_pair("101011", 19));  // sltu
    funct.insert(make_pair("000011", 20));  // sra
    funct.insert(make_pair("000111", 21));  // srav
    funct.insert(make_pair("000010", 22));  // srl
    funct.insert(make_pair("000110", 23));  // srlv
    funct.insert(make_pair("100010", 24));  // sub
    funct.insert(make_pair("100011", 25));  // subu
    funct.insert(make_pair("001100", 26));  // syscall
    funct.insert(make_pair("100110", 27));  // xor

    return funct;
}

//HexToBin
string HexToBin(InputFile& _i, int n)
{
    string dis_inst;
    int i = 4*n;

    bitset<8> bit1 = _i.buf[i];
    bitset<8> bit2 = _i.buf[i+1];
    bitset<8> bit3 = _i.buf[i+2];
    bitset<8> bit4 = _i.buf[i+3];


    dis_inst.append(bit1.to_string());
    dis_inst.append(bit2.to_string());
    dis_inst.append(bit3.to_string());
    dis_inst.append(bit4.to_string());

    return dis_inst;
}

//BinToDec
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

//BinToDec signed value
int BinToSignedDec (string imm)
{
    
    int i = 0;
    // 16-bit, 2byte 정수
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

//Disassembled
void Disassembled(InputFile& _i, int n, map<string, int> opcode, map<string, int> funct)
{
    //4byte가 1 instruction이므로 i=4*n
    int i = 4*n;
    //string name;
    string inst = HexToBin(_i, n);
    string op = inst.substr(0,6);
    string name = "";
    int rs, rt, rd, sa = 0;
    int offset = 0;

    if(!op.compare("000000"))
    {
        string funct_code = inst.substr(26,6);
        rs = BinToDec(inst.substr(6,5));
        rt = BinToDec(inst.substr(11,5));
        rd = BinToDec(inst.substr(16,5));
        sa = BinToDec(inst.substr(21,5));
        
        if(funct.find(funct_code) == funct.end())
        {
            printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
            cout << "unknown instruction" << endl;
        }
        else
        {
            switch(funct.at(funct_code)) {
            case 1:
                name = "add";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << ", $" << rt << endl;
                break;
            case 2:
                name = "addu";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << ", $" << rt << endl;
                break;
            case 3:
                name = "and";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << ", $" << rt << endl;
                break;
            case 4:
                name = "div";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rs << ", $" << rt << endl;
                break;
            case 5:
                name = "divu";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rs << ", $" << rt << endl;
                break;
            case 6:
                name = "jalr";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << endl;
                break;
            case 7:
                name = "jr";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rs << endl;
                break;
            case 8:
                name = "mfhi";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << endl;
                break;
            case 9:
                name = "mflo";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << endl;
                break;
            case 10:
                name = "mthi";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rs << endl;
                break;
            case 11:
                name = "mtlo";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rs << endl;
                break;
            case 12:
                name = "mult";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rs << ", $" << rt << endl;
                break;
            case 13:
                name = "multu";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rs << ", $" << rt << endl;
                break;
            case 14:
                name = "nor";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << ", $" << rt << endl;
                break;
            case 15:
                name = "or";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << ", $" << rt << endl;
                break;
            case 16:
                name = "sll";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rt << ", " << sa << endl;
                break;
            case 17:
                name = "sllv";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rt << ", $" << rs << endl;
                break;
            case 18:
                name = "slt";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << ", $" << rt << endl;
                break;
            case 19:
                name = "sltu";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << ", $" << rt << endl;
                break;
            case 20:
                name = "sra";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rt << ", " << sa << endl;
                break;
            case 21:
                name = "srav";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rt << ", $" << rs << endl;
                break;
            case 22:
                name = "srl";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rt << ", " << sa << endl;
                break;
            case 23:
                name = "srlv";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rt << ", $" << rs << endl;
                break;
            case 24:
                name = "sub";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << ", $" << rt << endl;
                break;
            case 25:
                name = "subu";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << ", $" << rt << endl;
                break;
            case 26:
                name = "syscall";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << endl;
                break;
            case 27:
                name = "xor";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " $" << rd << ", $" << rs << ", $" << rt << endl;
                break;
            }
        }
    }
    else
    {
        rs = BinToDec(inst.substr(6,5));
        rt = BinToDec(inst.substr(11, 5));
        offset = BinToSignedDec(inst.substr(16,16));
        if(opcode.find(op) == opcode.end())
        {
            printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
            cout << "unknown instruction" << endl;
        }
        else
        {
            switch(opcode.at(op)) {
            case 1:
                name = "addi";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", $" << rs << ", " << offset << endl;
                break;
            case 2:
                name = "addiu";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", $" << rs << ", " << offset << endl;
                break;
            case 3:
                name = "andi";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", $" << rs << ", " << offset << endl;
                break;
            case 4:
                name = "beq";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rs << ", $" << rt << ", " << offset << endl;
                break;
            case 5:
                name = "bne";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rs << ", $" << rt << ", " << offset << endl;
                break;
            case 6:
                name = "lb";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", " << offset << "($" << rs << ")" << endl;
                break;
            case 7:
                name = "lbu";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", " << offset << "($" << rs << ")" << endl;
                break;
            case 8:
                name = "lh";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", " << offset << "($" << rs << ")" << endl;
                break;
            case 9:
                name = "lhu";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", " << offset << "($" << rs << ")" << endl;
                break;
            case 10:
                name = "lui";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", " << offset << endl;
                break;
            case 11:
                name = "lw";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", " << offset << "($" << rs << ")" << endl;
                break;
            case 12:
                name = "ori";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", $" << rs << ", " << offset << endl;
                break;
            case 13:
                name = "sb";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", " << offset << "($" << rs << ")" << endl;
                break;
            case 14:
                name = "slti";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", $" << rs << ", " << offset << endl;
                break;
            case 15:
                name = "sltiu";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", $" << rs << ", " << offset << endl;
                break;
            case 16:
                name = "sh";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", " << offset << "($" << rs << ")" << endl;
                break;
            case 17:
                name = "sw";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", " << offset << "($" << rs << ")" << endl;
                break;
            case 18:
                name = "xori";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << "$" << rt << ", $" << rs << ", " << offset << endl;
                break;
            case 19:
                offset = BinToSignedDec(inst.substr(6,26));
                name = "j";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << offset << endl;
                break;
            case 20:
                offset = BinToSignedDec(inst.substr(6,26));
                name = "jal";
                printf("inst %d: %.2x%.2x%.2x%.2x ", n, _i.buf[i], _i.buf[i+1], _i.buf[i+2], _i.buf[i+3]);
                cout << name << " " << offset << endl;
                break;
            }
        }
    }
}

int main(int argc, char **argv)
{   

    InputFile input;
    input.input(argv[1]);

    map<string, int> opcode = opcode_map();
    map<string, int> funct = funct_map();


    for(int i = 0; i<(input.size/4); i++)
    {
        Disassembled(input, i, opcode, funct);
    }

    return 0;
}
    
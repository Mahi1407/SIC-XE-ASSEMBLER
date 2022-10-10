#include<bits/stdc++.h>
using namespace std;

class extdef{
  public:
    string name ;
    string address ;
    bool exists ;
    extdef(){
        name="undefined" ;
        address="0" ;
        exists= 0;
    }
};

class extref{
  public:
    string name ;
    string address ;
    bool exists ;
    extref(){
        name="undefined" ;
        address="0" ;
        exists=0 ;
    }
};

class csect{
  public:
    string name ;
    string LOCCTR ;
    int section_number ;
    int length ;
    map<string,extdef> EXTDEF_TAB ;
    map<string,extref> EXTREF_TAB ;
    csect(){
        name="DEFAULT" ;
        LOCCTR="0" ;
        section_number=0 ;
        length=0 ;
    }
};

class info_op{
  public:
    string opcode;
    int format;
    bool exists;
    info_op(){
      opcode="undefined";
      format=0;
      exists=0;
    }
    info_op(string a,int b, bool c){
      opcode=a;
      format=b;
      exists=c;
    }

};

class literal{
  public:
    string value;
    string address;
    bool exists;
    int block_num = 0;
    literal(){
      value="";
      address="?";
      block_num = 0;
      exists=0;
    }
};

class info_label{
  public:
     string address;
     string name;
     int relative;
     int block_num;
     bool exists;
     info_label(){
       name="undefined";
       address="0";
       block_num = 0;
       exists=0;
       relative = 0;
     }
};

class blocks{
  public:
     string start_addr;
     string name;
     string LOCCTR;
     int number;
     bool exists;
     blocks(){
       name="undefined";
       start_addr="?";
       exists=0;
       number = -1;
       LOCCTR = "0";
     }
};

class info_reg{
  public:
     char num;
     bool exists;
     info_reg(){
       num = 'F';
       exists=0;
     }
     info_reg(char a, bool exist){
       num=a;
       exists=exist;
     }
};

typedef map<string,info_label> SYMBOL_TABLE;
typedef map<string,info_op> OPCODE_TABLE;
typedef map<string,info_reg> REG_TABLE;
typedef map<string,literal> LIT_TABLE;
typedef map<string,blocks> BLOCK_TABLE;
typedef map<string,csect> CSECT_TABLE;

SYMBOL_TABLE SYMTAB;
OPCODE_TABLE OP_TAB;
REG_TABLE REG_TAB;
LIT_TABLE LITTAB;
BLOCK_TABLE BLOCKS;
CSECT_TABLE CSECT_TAB;

void load_REG_TAB(){
  info_reg r('0',1); REG_TAB["A"]=r;
  info_reg r1('1',1); REG_TAB["X"]=r1;
  info_reg r2('2',1); REG_TAB["L"]=r2;
  info_reg r3('3',1); REG_TAB["B"]=r3;
  info_reg r4('4',1); REG_TAB["S"]=r4;
  info_reg r5('5',1); REG_TAB["T"]=r5;
  info_reg r6('6',1); REG_TAB["F"]=r6;
  info_reg r7('8',1); REG_TAB["PC"]=r7;
  info_reg r8('9',1); REG_TAB["SW"]=r8;

}
void load_OP_TAB(){
  info_op op1("18",3,1); OP_TAB["ADD"]=op1;
  info_op op2("58",3,1); OP_TAB["ADDF"]=op2;
  info_op op3("90",2,1); OP_TAB["ADDR"]=op3;
  info_op op4("40",3,1); OP_TAB["AND"]=op4;
  info_op op5("B4",2,1); OP_TAB["CLEAR"]=op5;
  info_op op6("28",3,1); OP_TAB["COMP"]=op6;
  info_op op7("88",3,1); OP_TAB["COMPF"]=op7;
  info_op op8("A0",2,1); OP_TAB["COMPR"]=op8;
  info_op op9("24",3,1); OP_TAB["DIV"]=op9;
  info_op op10("64",3,1); OP_TAB["DIVF"]=op10;
  info_op op11("9C",2,1); OP_TAB["DIVR"]=op11;
  info_op op12("C4",1,1); OP_TAB["FIX"]=op12;
  info_op op13("C0",1,1); OP_TAB["FLOAT"]=op13;
  info_op op14("F4",1,1); OP_TAB["HIO"]=op14;
  info_op op15("3C",3,1); OP_TAB["J"]=op15;
  info_op op16("30",3,1); OP_TAB["JEQ"]=op16;
  info_op op17("34",3,1); OP_TAB["JGT"]=op17;
  info_op op18("38",3,1); OP_TAB["JLT"]=op18;
  info_op op19("48",3,1); OP_TAB["JSUB"]=op19;
  info_op op20("00",3,1); OP_TAB["LDA"]=op20;
  info_op op21("68",3,1); OP_TAB["LDB"]=op21;
  info_op op22("50",3,1); OP_TAB["LDCH"]=op22;
  info_op op23("70",3,1); OP_TAB["LDF"]=op23;
  info_op op24("08",3,1); OP_TAB["LDL"]=op24;
  info_op op25("6C",3,1); OP_TAB["LDS"]=op25;
  info_op op26("74",3,1); OP_TAB["LDT"]=op26;
  info_op op27("04",3,1); OP_TAB["LDX"]=op27;
  info_op op28("D0",3,1); OP_TAB["LPS"]=op28;
  info_op op29("20",3,1); OP_TAB["MUL"]=op29;
  info_op op30("60",3,1); OP_TAB["MULF"]=op30;
  info_op op31("98",2,1); OP_TAB["MULR"]=op31;
  info_op op32("C8",1,1); OP_TAB["NORM"]=op32;
  info_op op33("44",3,1); OP_TAB["OR"]=op33;
  info_op op34("D8",3,1); OP_TAB["RD"]=op34;
  info_op op35("AC",2,1); OP_TAB["RMO"]=op35;
  info_op op36("4F",3,1); OP_TAB["RSUB"]=op36;
  info_op op37("A4",2,1); OP_TAB["SHIFTL"]=op37;
  info_op op38("A8",2,1); OP_TAB["SHIFTR"]=op38;
  info_op op39("F0",1,1); OP_TAB["SIO"]=op39;
  info_op op40("EC",3,1); OP_TAB["SSK"]=op40;
  info_op op41("0C",3,1); OP_TAB["STA"]=op41;
  info_op op42("78",3,1); OP_TAB["STB"]=op42;
  info_op op43("54",3,1); OP_TAB["STCH"]=op43;
  info_op op44("80",3,1); OP_TAB["STF"]=op44;
  info_op op45("D4",3,1); OP_TAB["STI"]=op45;
  info_op op46("14",3,1); OP_TAB["STL"]=op46;
  info_op op47("7C",3,1); OP_TAB["STS"]=op47;
  info_op op48("E8",3,1); OP_TAB["STSW"]=op48;
  info_op op49("84",3,1); OP_TAB["STT"]=op49;
  info_op op50("10",3,1); OP_TAB["STX"]=op50;
  info_op op51("1C",3,1); OP_TAB["SUB"]=op51;
  info_op op52("5C",3,1); OP_TAB["SUBF"]=op52;
  info_op op53("94",2,1); OP_TAB["SUBR"]=op53;
  info_op op54("B0",2,1); OP_TAB["SVC"]=op54;
  info_op op55("E0",3,1); OP_TAB["TD"]=op55;
  info_op op56("F8",1,1); OP_TAB["TIO"]=op56;
  info_op op57("2C",3,1); OP_TAB["TIX"]=op57;
  info_op op58("B8",2,1); OP_TAB["TIXR"]=op58;
  info_op op59("DC",3,1); OP_TAB["WD"]=op59;

}

void load_BLOCKS(){
  BLOCKS["DEFAULT"].exists = 1;
  BLOCKS["DEFAULT"].name = "DEFAULT";
  BLOCKS["DEFAULT"].start_addr = "00000";
  BLOCKS["DEFAULT"].number=0;
  BLOCKS["DEFAULT"].LOCCTR = "0";
}
void load_tables(){
  load_BLOCKS();
  load_OP_TAB();
  load_REG_TAB();
}


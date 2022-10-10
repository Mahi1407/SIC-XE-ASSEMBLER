#include<bits/stdc++.h>
using namespace std;
string get_str(char c){
  string s(1,c) ;
  return s ;
}

string int_to_strHex(int x,int fill = 5){
  stringstream s;
  s << setfill('0') << setw(fill) << hex << x;
  string temp = s.str();
  temp = temp.substr(temp.length()-fill,fill);
  transform(temp.begin(), temp.end(),temp.begin(),::toupper);
  return temp;
}

string expand_str(string d,int len,char fillChar,bool b=false){
  if(b){
    if(len<=d.length()){
      return d.substr(0,len);
    }
    else{
      for(int i = len-d.length();i>0;i--){
        d += fillChar;
      }
    }
  }
  else{
    if(len<=d.length()){
      return d.substr(d.length()-len,len);
    }
    else{
      for(int i = len-d.length();i>0;i--){
        d = fillChar + d;
      }
    }
  }
  return d;
}

int str_hex_to_int(string x){
  return stoul(x,nullptr,16);
}

string str_to_hexa_str(const string& input){
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

bool Is_space(char x){
  if(x==' ' || x=='\t'){
    return true;
  }
  return false;
}

bool Is_comment(string line){
  if(line[0]=='.'){
    return true;
  }
  return false;
}

bool if_all_num(string x){
  bool all_num = true;
  int i = 0;
  while(all_num && (i<x.length())){
    all_num &= isdigit(x[i++]);
  }
  return all_num;
}

void read_first_non_space(string line,int& index,bool& status,string& d,bool readTillEnd=false){
  d = "";
  status = true;
  if(readTillEnd){
    d = line.substr(index,line.length() - index);
    if(d==""){
      status = false;
    }
    return;
  }
  while(index<line.length()&&!Is_space(line[index])){
    d += line[index];
    index++;
  }
  if(d==""){
    status = false;
  }
  while(index<line.length()&&Is_space(line[index])){
    index++;
  }
}

void read_byte_operand(string line,int& index,bool& status,string& d){
  d = "";
  status = true;
  if(line[index]=='C'){
    d += line[index++];
    char identifier = line[index++];
    d += identifier;
    while(index<line.length() && line[index]!=identifier){
      d += line[index];
      index++;
    }
    d += identifier;
    index++;
  }
  else{
    while(index<line.length()&&!Is_space(line[index])){
      d += line[index];
      index++;
    }
  }
  if(d==""){
    status = false;
  }
  while(index<line.length()&&Is_space(line[index])){
    index++;
  }
}

void write_to_file(ofstream& file,string d,bool newline=true){
  if(newline){
    file<<d<<endl;
  }else{
    file<<d;
  }
}

string get_real_opcode(string opcode){
  if(opcode[0] == '+' || opcode[0] == '@'){
    return opcode.substr(1,opcode.length() - 1);
  }
  return opcode;
}

char get_flag_format(string d){
  if(d[0] == '#' || d[0] == '+' || d[0] == '@' || d[0] == '='){
    return d[0];
  }
  return ' ';
}

class Eval_str{
public:
  int get_result();
  Eval_str(string d);
private:
  string storedData;
  int index;
  char peek();
  char get();
  int term();
  int factor();
  int number();
};

Eval_str::Eval_str(string d){
  storedData = d;
  index=0;
}

int Eval_str::get_result(){
  int result = term();
  while(peek()=='+' || peek() == '-'){
    if(get() == '+'){
      result += term();
    }else{
      result -= term();
    }
  }
  return result;
}

int Eval_str::term(){
  int result = factor();
  while(peek() == '*' || peek() == '/'){
    if(get()=='*'){
      result *= factor();
    }
    else{
      result /= factor();
    }
  }
  return result;
}

int Eval_str::factor(){
  if(peek() >= '0' && peek() <= '9'){
    return number();
  }
  else if(peek() == '('){
    get();
    int result = get_result();
    get();
    return result;
  }
  else if(peek()=='-'){
    get();
    return -factor();
  }
  return 0;
}

int Eval_str::number(){
  int result = get() - '0';
  while(peek() >= '0' && peek() <= '9'){
    result = 10*result + get()-'0';
  }
  return result;
}

char Eval_str::get(){
  return storedData[index++];
}

char Eval_str::peek(){
  return storedData[index];
}



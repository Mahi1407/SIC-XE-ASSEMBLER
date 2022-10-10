#include "pass1.cpp"
using namespace std;
ifstream intermediate_File;
ofstream error_File,object_File,Listing_File;
ofstream print_tab ;
string write_str ;
bool isComment;
string label,opcode,operand,comment;
string operand_1,operand_2;
int line_number,block_num,address,start_addr;
string objectCode, write_data, curr_rec, modify_rec="M^", end_rec, write_R_Data, write_D_Data,curr_sec_name="DEFAULT";
int sec_counter=0;
int program_section_length=0;
int p_c, base_register_value, curr_text_rec_len;
bool nobase;
string read_till_tab(string data,int& index){
  string temp_buffer = "";
  while(index<data.length() && data[index] != '\t'){
    temp_buffer += data[index];
    index++;
  }
  index++;
  if(temp_buffer==" "){
    temp_buffer="-1" ;
  }
  return temp_buffer;
}
bool read_intermediate_file(ifstream& read_File,bool& isComment, int& line_number, int& address, int& block_num, string& label, string& opcode, string& operand, string& comment){
  string file_Line="";
  string temp_buffer="";
  bool temp_status;
  int index=0;
  if(!getline(read_File, file_Line)){
    return false;
  }
  line_number = stoi(read_till_tab(file_Line,index));
  isComment = (file_Line[index]=='.')?true:false;
  if(isComment){
    read_first_non_space(file_Line,index,temp_status,comment,true);
    return true;
  }
  address = str_hex_to_int(read_till_tab(file_Line,index));
  temp_buffer = read_till_tab(file_Line,index);
  if(temp_buffer == " "){
    block_num = -1;
  }
  else{
    block_num = stoi(temp_buffer);
  }
  label = read_till_tab(file_Line,index);
  if(label=="-1"){
    label=" " ;
  }
  opcode = read_till_tab(file_Line,index);
  if(opcode=="BYTE"){
    read_byte_operand(file_Line,index,temp_status,operand);
  }
  else{
    operand = read_till_tab(file_Line,index);
    if(operand=="-1"){
      operand=" " ;
    }
    if(opcode=="CSECT"){
      return true ;
    }
  }
  read_first_non_space(file_Line,index,temp_status,comment,true);  
  return true;
}
string create_obj_code_format_34(){
  string obj_code;
  int half_bytes;
  half_bytes = (get_flag_format(opcode)=='+')?5:3;
  if(get_flag_format(operand)=='#'){
    if(operand.substr(operand.length()-2,2)==",X"){
      write_data = "Line: "+to_string(line_number)+" Index based addressing not supported with Indirect addressing";
      write_to_file(error_File,write_data);
      obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+1,2);
      obj_code += (half_bytes==5)?"100000":"0000";
      return obj_code;
    }
    string temp_operand = operand.substr(1,operand.length()-1);
    if(if_all_num(temp_operand)||((SYMTAB[temp_operand].exists==1)&&(SYMTAB[temp_operand].relative==0)
    &&(CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists==0))){
      int imm_val;
      if(if_all_num(temp_operand)){
        imm_val = stoi(temp_operand);
      }
      else{
        imm_val = str_hex_to_int(SYMTAB[temp_operand].address);
      }
      if(imm_val>=(1<<4*half_bytes)){
        write_data = "Line: "+to_string(line_number)+" Immediate value exceeds format limit";
        write_to_file(error_File,write_data);
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+1,2);
        obj_code += (half_bytes==5)?"100000":"0000";
      }
      else{
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+1,2);
        obj_code += (half_bytes==5)?'1':'0';
        obj_code += int_to_strHex(imm_val,half_bytes);
      }
      return obj_code;
    }
    else if(SYMTAB[temp_operand].exists==0||CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists==1) {
      if(CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists!=1 || half_bytes==3) { 
      write_data = "Line "+to_string(line_number);
     if(half_bytes==3 && CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists==1){
         write_data+= " : Invalid format for external reference " + temp_operand; 
      } else{ 
         write_data += " : Symbol doesn't exists. Found " + temp_operand;
       } 
      write_to_file(error_File,write_data);
      obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+1,2);
      obj_code += (half_bytes==5)?"100000":"0000";
      return obj_code;
    }  
            if(SYMTAB[temp_operand].exists==1&& CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists==1) {
             obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+1,2);
             obj_code += "100000";
             modify_rec += "M^" + int_to_strHex(address+1,6) + '^';
             modify_rec += "05+";
             modify_rec += CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].name ;
             modify_rec += '\n';       

            return obj_code ;
         }
    }
    else{
      int operand_addr = str_hex_to_int(SYMTAB[temp_operand].address) + str_hex_to_int(BLOCKS[Blocks_num_to_name[SYMTAB[temp_operand].block_num]].start_addr);
      if(half_bytes==5){
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+1,2);
        obj_code += '1';
        obj_code += int_to_strHex(operand_addr,half_bytes);
        modify_rec += "M^" + int_to_strHex(address+1,6) + '^';
        modify_rec += (half_bytes==5)?"05":"03";
        modify_rec += '\n';
        return obj_code;
      }
      p_c = address + str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr);
      p_c += (half_bytes==5)?4:3;
      int rel_addr = operand_addr - p_c;
      if(rel_addr>=(-2048) && rel_addr<=2047){
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+1,2);
        obj_code += '2';
        obj_code += int_to_strHex(rel_addr,half_bytes);
        return obj_code;
      }
      if(!nobase){
        rel_addr = operand_addr - base_register_value;
        if(rel_addr>=0 && rel_addr<=4095){
          obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+1,2);
          obj_code += '4';
          obj_code += int_to_strHex(rel_addr,half_bytes);
          return obj_code;
        }
      }
      if(operand_addr<=4095){
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+1,2);
        obj_code += '0';
        obj_code += int_to_strHex(operand_addr,half_bytes);
        modify_rec += "M^" + int_to_strHex(address+1+str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr),6) + '^';
        modify_rec += (half_bytes==5)?"05":"03";
        modify_rec += '\n';
        return obj_code;
      }
    }
  }
  else if(get_flag_format(operand)=='@'){
    string temp_operand = operand.substr(1,operand.length()-1);
    if(temp_operand.substr(temp_operand.length()-2,2)==",X" || SYMTAB[temp_operand].exists==0 ||CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists=='y'){
      if(CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists!=1 || half_bytes==3) {
      write_data = "Line "+to_string(line_number);
      if(half_bytes==3 && CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists==1){
         write_data+= " : Invalid format for external reference " + temp_operand; 
      } else{ 
         write_data += " : Symbol doesn't exists.Index based addressing not supported with Indirect addressing ";
       } 
      write_to_file(error_File,write_data);
      obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+2,2);
      obj_code += (half_bytes==5)?"100000":"0000";
      return obj_code;
    }
    if(SYMTAB[temp_operand].exists==1&& CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists==1) {
          obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+2,2);
             obj_code += "100000";
             modify_rec += "M^" + int_to_strHex(address+1,6) + '^';
             modify_rec += "05+";
             modify_rec += CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].name ;
             modify_rec += '\n';      

             return obj_code ; 
         }
}
    int operand_addr = str_hex_to_int(SYMTAB[temp_operand].address) + str_hex_to_int(BLOCKS[Blocks_num_to_name[SYMTAB[temp_operand].block_num]].start_addr);
    p_c = address + str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr);
    p_c += (half_bytes==5)?4:3;
    if(half_bytes==3){
      int rel_addr = operand_addr - p_c;
      if(rel_addr>=(-2048) && rel_addr<=2047){
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+2,2);
        obj_code += '2';
        obj_code += int_to_strHex(rel_addr,half_bytes);
        return obj_code;
      }
      if(!nobase){
        rel_addr = operand_addr - base_register_value;
        if(rel_addr>=0 && rel_addr<=4095){
          obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+2,2);
          obj_code += '4';
          obj_code += int_to_strHex(rel_addr,half_bytes);
          return obj_code;
        }
      }
      if(operand_addr<=4095){
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+2,2);
        obj_code += '0';
        obj_code += int_to_strHex(operand_addr,half_bytes);
        modify_rec += "M^" + int_to_strHex(address+1+str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr),6) + '^';
        modify_rec += (half_bytes==5)?"05":"03";
        modify_rec += '\n';

        return obj_code;
      }
    }
    else{
      obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+2,2);
      obj_code += '1';
      obj_code += int_to_strHex(operand_addr,half_bytes);
      modify_rec += "M^" + int_to_strHex(address+1+str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr),6) + '^';
      modify_rec += (half_bytes==5)?"05":"03";
      modify_rec += '\n';

      return obj_code;
    }
    write_data = "Line: "+to_string(line_number);
    write_data += "Can't fit into program counter based or base register based addressing.";
    write_to_file(error_File,write_data);
    obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+2,2);
    obj_code += (half_bytes==5)?"100000":"0000";

    return obj_code;
  }
  else if(get_flag_format(operand)=='='){
    string temp_operand = operand.substr(1,operand.length()-1);
    if(temp_operand=="*"){
      temp_operand = "X'" + int_to_strHex(address,6) + "'";
      modify_rec += "M^" + int_to_strHex(str_hex_to_int(LITTAB[temp_operand].address)+str_hex_to_int(BLOCKS[Blocks_num_to_name[LITTAB[temp_operand].block_num]].start_addr),6) + '^';
      modify_rec += int_to_strHex(6,2);
      modify_rec += '\n';
    }
    if(LITTAB[temp_operand].exists==0){
      write_data = "Line "+to_string(line_number)+" : Symbol doesn't exists. Found " + temp_operand;
      write_to_file(error_File,write_data);

      obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
      obj_code += (half_bytes==5)?"000":"0";
      obj_code += "000";
      return obj_code;
    }
    int operand_addr = str_hex_to_int(LITTAB[temp_operand].address) + str_hex_to_int(BLOCKS[Blocks_num_to_name[LITTAB[temp_operand].block_num]].start_addr);
    p_c = address + str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr);
    p_c += (half_bytes==5)?4:3;
    if(half_bytes==3){
      int rel_addr = operand_addr - p_c;
      if(rel_addr>=(-2048) && rel_addr<=2047){
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
        obj_code += '2';
        obj_code += int_to_strHex(rel_addr,half_bytes);
        return obj_code;
      }
      if(!nobase){
        rel_addr = operand_addr - base_register_value;
        if(rel_addr>=0 && rel_addr<=4095){
          obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
          obj_code += '4';
          obj_code += int_to_strHex(rel_addr,half_bytes);
          return obj_code;
        }
      }
      if(operand_addr<=4095){
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
        obj_code += '0';
        obj_code += int_to_strHex(operand_addr,half_bytes);
        modify_rec += "M^" + int_to_strHex(address+1+str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr),6) + '^';
        modify_rec += (half_bytes==5)?"05":"03";
        modify_rec += '\n';

        return obj_code;
      }
    }
    else{
      obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
      obj_code += '1';
      obj_code += int_to_strHex(operand_addr,half_bytes);
      modify_rec += "M^" + int_to_strHex(address+1+str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr),6) + '^';
      modify_rec += (half_bytes==5)?"05":"03";
      modify_rec += '\n';

      return obj_code;
    }
    write_data = "Line: "+to_string(line_number);
    write_data += "Can't fit into program counter based or base register based addressing.";
    write_to_file(error_File,write_data);
    obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
    obj_code += (half_bytes==5)?"100":"0";
    obj_code += "000";
    return obj_code;
  }
  else{
    int x_b_p_e=0;
    string temp_operand = operand;
    if(operand.substr(operand.length()-2,2)==",X"){
      temp_operand = operand.substr(0,operand.length()-2);
      x_b_p_e = 8;
    }
    if(SYMTAB[temp_operand].exists==0||CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists==1) {
      if(CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists!=1 || half_bytes==3) { 
      write_data = "Line "+to_string(line_number);
      if(half_bytes==3 && CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists==1){
         write_data+= " : Invalid format for external reference " + temp_operand; 
      } else{ 
         write_data += " : Symbol doesn't exists. Found " + temp_operand;
       } 
      write_to_file(error_File,write_data);
      obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
      obj_code += (half_bytes==5)?(int_to_strHex(x_b_p_e+1,1)+"00"):int_to_strHex(x_b_p_e,1);
      obj_code += "000";
      return obj_code;
    }
   if(SYMTAB[temp_operand].exists==1&& CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].exists==1) {
             obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
             obj_code += "100000";
             modify_rec += "M^" + int_to_strHex(address+1,6) + '^';
             modify_rec += "05+";
             modify_rec += CSECT_TAB[curr_sec_name].EXTREF_TAB[temp_operand].name ;
             modify_rec += '\n';       
            return obj_code ;
         }
    }
else{
    int operand_addr = str_hex_to_int(SYMTAB[temp_operand].address) + str_hex_to_int(BLOCKS[Blocks_num_to_name[SYMTAB[temp_operand].block_num]].start_addr);
    p_c = address + str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr);
    p_c += (half_bytes==5)?4:3;
    if(half_bytes==3){
      int rel_addr = operand_addr - p_c;
      if(rel_addr>=(-2048) && rel_addr<=2047){
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
        obj_code += int_to_strHex(x_b_p_e+2,1);
        obj_code += int_to_strHex(rel_addr,half_bytes);
        return obj_code;
      }
      if(!nobase){
        rel_addr = operand_addr - base_register_value;
        if(rel_addr>=0 && rel_addr<=4095){
          obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
          obj_code += int_to_strHex(x_b_p_e+4,1);
          obj_code += int_to_strHex(rel_addr,half_bytes);
          return obj_code;
        }
      }
      if(operand_addr<=4095){
        obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
        obj_code += int_to_strHex(x_b_p_e,1);
        obj_code += int_to_strHex(operand_addr,half_bytes);
        modify_rec += "M^" + int_to_strHex(address+1+str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr),6) + '^';
        modify_rec += (half_bytes==5)?"05":"03";
        modify_rec += '\n';
        return obj_code;
      }
    }
    else{
      obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
      obj_code += int_to_strHex(x_b_p_e+1,1);
      obj_code += int_to_strHex(operand_addr,half_bytes);
      modify_rec += "M^" + int_to_strHex(address+1+str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr),6) + '^';
      modify_rec += (half_bytes==5)?"05":"03";
      modify_rec += '\n';
      return obj_code;
    }
    write_data = "Line: "+to_string(line_number);
    write_data += "Can't fit into program counter based or base register based addressing.";
    write_to_file(error_File,write_data);
    obj_code = int_to_strHex(str_hex_to_int(OP_TAB[get_real_opcode(opcode)].opcode)+3,2);
    obj_code += (half_bytes==5)?(int_to_strHex(x_b_p_e+1,1)+"00"):int_to_strHex(x_b_p_e,1);
    obj_code += "000";
    return obj_code;
  }}
  return "";
}
void write_text_rec(bool last_rec=false){
  if(last_rec){
    if(curr_rec.length()>0){
      write_data = int_to_strHex(curr_rec.length()/2,2) + '^' + curr_rec;
      write_to_file(object_File,write_data);
      curr_rec = "";
    }
    return;
  }
  if(objectCode != ""){
    if(curr_rec.length()==0){
      write_data = "T^" + int_to_strHex(address+str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr),6) + '^';
      write_to_file(object_File,write_data,false);
    }
    if((curr_rec + objectCode).length()>60){
      write_data = int_to_strHex(curr_rec.length()/2,2) + '^' + curr_rec;
      write_to_file(object_File,write_data);
      curr_rec = "";
      write_data = "T^" + int_to_strHex(address+str_hex_to_int(BLOCKS[Blocks_num_to_name[block_num]].start_addr),6) + '^';
      write_to_file(object_File,write_data,false);
    }
    curr_rec += objectCode;
  }
  else{
    if(opcode=="START"||opcode=="END"||opcode=="BASE"||opcode=="NOBASE"||opcode=="LTORG"||opcode=="ORG"||opcode=="EQU"/****/||opcode=="EXTREF"||opcode=="EXTDEF"/******/){
    }
    else{
      if(curr_rec.length()>0){
        write_data = int_to_strHex(curr_rec.length()/2,2) + '^' + curr_rec;
        write_to_file(object_File,write_data);
      }
      curr_rec = "";
    }
  }
}
void write_D_rec(){
    write_D_Data="D^" ;
    string temp_address="" ;
    int length=operand.length() ;
    string inp="" ;
    for(int i=0;i<length;i++){
      while(operand[i]!=',' && i<length){
        inp+=operand[i] ;
        i++ ;
      }
      temp_address=CSECT_TAB[curr_sec_name].EXTDEF_TAB[inp].address ;
      write_D_Data+=expand_str(inp,6,' ',true)+temp_address;
      inp="" ;
    }
    write_to_file(object_File,write_D_Data);
}
void write_R_rec(){
    write_R_Data="R^" ;
    string temp_address="" ;
    int length=operand.length() ;
    string inp="" ;
    for(int i=0;i<length;i++){
      while(operand[i]!=',' && i<length){
        inp+=operand[i] ;
        i++ ;
      }
      write_R_Data+=expand_str(inp,6,' ',true);
      inp="" ;
    }
    write_to_file(object_File,write_R_Data);
}
void write_end_rec(bool write=true){
  if(write){
    if(end_rec.length()>0){
      write_to_file(object_File,end_rec);
    }
    else{
      write_end_rec(false);
    }
  }
  if((operand==""||operand==" ")&&curr_sec_name=="DEFAULT"){
    end_rec = "E^" + int_to_strHex(start_addr,6);
  }else if(curr_sec_name!="DEFAULT"){
    end_rec = "E";
  }
  else{
    int firstExecutableAddress;
   
      firstExecutableAddress = str_hex_to_int(SYMTAB[first_execu_sec].address);
    
    end_rec = "E^" + int_to_strHex(firstExecutableAddress,6)+"\n";
  }
}
void pass2(){
  string temp_buffer;
  intermediate_File.open("intermediate_file.txt");
  if(!intermediate_File){
    cout<<"Unable to open file: intermediate_"<<fileName<<endl;
    exit(1);
  }
  getline(intermediate_File, temp_buffer); 
  object_File.open("object_file.txt");
  if(!object_File){
    cout<<"Unable to open file: object_"<<fileName<<endl;
    exit(1);
  }
  Listing_File.open("listing_file.txt");
  if(!Listing_File){
    cout<<"Unable to open file: listing_"<<fileName<<endl;
    exit(1);
  }
  write_to_file(Listing_File,"Line\tAddress\tLabel\tOPCODE\tOPERAND\tObjectCode\tComment");
  error_File.open("error_file.txt",fstream::app);
  if(!error_File){
    cout<<"Unable to open file: error_"<<fileName<<endl;
    exit(1);
  }
  write_to_file(error_File,"\n\n************PASS2************");
  objectCode = "";
  curr_text_rec_len=0;
  curr_rec = "";
  modify_rec = "";
  block_num = 0;
  nobase = true;
  read_intermediate_file(intermediate_File,isComment,line_number,address,block_num,label,opcode,operand,comment);
  while(isComment){
    write_data = to_string(line_number) + "\t" + comment;
    write_to_file(Listing_File,write_data);
    read_intermediate_file(intermediate_File,isComment,line_number,address,block_num,label,opcode,operand,comment);
  }
  if(opcode=="START"){
    start_addr = address;
    write_data = to_string(line_number) + "\t" + int_to_strHex(address) + "\t" + to_string(block_num) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
    write_to_file(Listing_File,write_data);
  }
  else{
    label = "";
    start_addr = 0;
    address = 0;
  }
  if(BLOCKS.size()>1){
    program_section_length = p_l ;
  }
  else{
    program_section_length=CSECT_TAB[curr_sec_name].length ;
  }
  write_data = "H^"+expand_str(label,6,' ',true)+'^'+int_to_strHex(address,6)+'^'+int_to_strHex(program_section_length,6);
  write_to_file(object_File,write_data);
  read_intermediate_file(intermediate_File,isComment,line_number,address,block_num,label,opcode,operand,comment);
  curr_text_rec_len  = 0;
  while(opcode!="END"){
      while(opcode!="END" && opcode!="CSECT"){
    if(!isComment){
      if(OP_TAB[get_real_opcode(opcode)].exists==1){
        if(OP_TAB[get_real_opcode(opcode)].format==1){
          objectCode = OP_TAB[get_real_opcode(opcode)].opcode;
        }
        else if(OP_TAB[get_real_opcode(opcode)].format==2){
          operand_1 = operand.substr(0,operand.find(','));
          operand_2 = operand.substr(operand.find(',')+1,operand.length()-operand.find(',') -1 );

          if(operand_2==operand){
            if(get_real_opcode(opcode)=="SVC"){
              objectCode = OP_TAB[get_real_opcode(opcode)].opcode + int_to_strHex(stoi(operand_1),1) + '0';
            }
            else if(REG_TAB[operand_1].exists==1){
              objectCode = OP_TAB[get_real_opcode(opcode)].opcode + REG_TAB[operand_1].num + '0';
            }
            else{
              objectCode = get_real_opcode(opcode) + '0' + '0';
              write_data = "line : "+to_string(line_number)+" invalid register name ";
              write_to_file(error_File,write_data);
            }
          }
          else{
            if(REG_TAB[operand_1].exists==0){
              objectCode = OP_TAB[get_real_opcode(opcode)].opcode + "00";
              write_data = "line : "+to_string(line_number)+" inalid register name ";
              write_to_file(error_File,write_data);
            }
            else if(get_real_opcode(opcode)=="SHIFTR" || get_real_opcode(opcode)=="SHIFTL"){
              objectCode = OP_TAB[get_real_opcode(opcode)].opcode + REG_TAB[operand_1].num + int_to_strHex(stoi(operand_2),1);
            }
            else if(REG_TAB[operand_2].exists==0){
              objectCode = OP_TAB[get_real_opcode(opcode)].opcode + "00";
              write_data = "line : "+to_string(line_number)+" invalid register name";
              write_to_file(error_File,write_data);
            }
            else{
              objectCode = OP_TAB[get_real_opcode(opcode)].opcode + REG_TAB[operand_1].num + REG_TAB[operand_2].num;
            }
          }
        }
        else if(OP_TAB[get_real_opcode(opcode)].format==3){
          if(get_real_opcode(opcode)=="RSUB"){
            objectCode = OP_TAB[get_real_opcode(opcode)].opcode;
            objectCode += (get_flag_format(opcode)=='+')?"000000":"0000";
          }
          else{
            objectCode = create_obj_code_format_34();
          }
        }
      }
      else if(opcode=="BYTE"){
        if(operand[0]=='X'){
          objectCode = operand.substr(2,operand.length()-3);
        }
        else if(operand[0]=='C'){
          objectCode = str_to_hexa_str(operand.substr(2,operand.length()-3));
        }
      }
      else if(label=="*"){
        if(opcode[1]=='C'){
          objectCode = str_to_hexa_str(opcode.substr(3,opcode.length()-4));
        }
        else if(opcode[1]=='X'){
          objectCode = opcode.substr(3,opcode.length()-4);
        }
      }
      else if(opcode=="WORD"){
        objectCode = int_to_strHex(stoi(operand),6);
      }
      else if(opcode=="BASE"){
        if(SYMTAB[operand].exists==1){
          base_register_value = str_hex_to_int(SYMTAB[operand].address) + str_hex_to_int(BLOCKS[Blocks_num_to_name[SYMTAB[operand].block_num]].start_addr);
          nobase = false;
        }
        else{
          write_data = "Line "+to_string(line_number)+" : Symbol doesn't exists. Found " + operand;
          write_to_file(error_File,write_data);
        }
        objectCode = "";
      }
      else if(opcode=="NOBASE"){
        if(nobase){
          write_data = "Line "+to_string(line_number)+": Assembler wasn't using base addressing";
          write_to_file(error_File,write_data);
        }
        else{
          nobase = true;
        }
        objectCode = "";
      }
      else{
        objectCode = "";
      }
      write_text_rec();
      if(block_num==-1 && address!=-1){
        write_data = to_string(line_number) + "\t" + int_to_strHex(address) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
      }
      else if(address==-1){
        if(opcode=="EXTDEF"){
          write_D_rec() ;
        } else if(opcode=="EXTREF"){
          write_R_rec() ;
        }
        write_data = to_string(line_number) + "\t" + " " + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
      } 
     
      else{ write_data = to_string(line_number) + "\t" + int_to_strHex(address) + "\t" + to_string(block_num) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
      }
    }
    else{
      write_data = to_string(line_number) + "\t" + comment;
    }
    write_to_file(Listing_File,write_data);
    read_intermediate_file(intermediate_File,isComment,line_number,address,block_num,label,opcode,operand,comment);
    objectCode = "";
  }
  write_text_rec();
  write_end_rec(false);
   if(opcode=="CSECT"&&!isComment){
        write_data = to_string(line_number) + "\t" + int_to_strHex(address) + "\t" + to_string(block_num) + "\t" + label + "\t" + opcode + "\t" + " " + "\t" + objectCode +"\t"+" ";
      }else if(!isComment){
  write_data = to_string(line_number) + "\t" + int_to_strHex(address) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + "" +"\t" + comment;
}else{
  write_data = to_string(line_number) + "\t" + comment;
 }
  write_to_file(Listing_File,write_data);

if(opcode!="CSECT"){
  while(read_intermediate_file(intermediate_File,isComment,line_number,address,block_num,label,opcode,operand,comment)){
    if(label=="*"){
      if(opcode[1]=='C'){
        objectCode = str_to_hexa_str(opcode.substr(3,opcode.length()-4));
      }
      else if(opcode[1]=='X'){
        objectCode = opcode.substr(3,opcode.length()-4);
      }
      write_text_rec();
    }
    write_data = to_string(line_number) + "\t" + int_to_strHex(address) + "\t" + to_string(block_num) + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
    write_to_file(Listing_File,write_data);
  }
}    
write_text_rec(true);
if(!isComment){
  write_to_file(object_File,modify_rec,false);
  write_end_rec(true);
  curr_sec_name=label;
  modify_rec="";
}
if(!isComment&&opcode!="END"){
write_data = "\n********************object program for "+ label+" **************";
  write_to_file(object_File,write_data);
write_data = "\nH^"+expand_str(label,6,' ',true)+'^'+int_to_strHex(address,6)+'^'+int_to_strHex(CSECT_TAB[label].length,6);
  write_to_file(object_File,write_data);
  }
  read_intermediate_file(intermediate_File,isComment,line_number,address,block_num,label,opcode,operand,comment);//Read next line
    objectCode = "";

}

}

int main(){
  load_tables();
  cout<<"\nExecuting PASS1"<<endl;
  pass1();
  print_tab.open("tables_file.txt") ;
  write_to_file(print_tab,"**********************************SYMBOL TABLE*****************************\n") ;
    for (auto const& it: SYMTAB) { 
        write_str+=it.first+":-\t"+ "name:"+it.second.name+"\t|"+ "address:"+it.second.address+"\t|"+ "relative:"+int_to_strHex(it.second.relative)+" \n" ;
    } 
    write_to_file(print_tab,write_str) ;
write_str="" ;
  write_to_file(print_tab,"**********************************LITERAL TABLE*****************************\n") ;
    for (auto const& it: LITTAB) { 
        write_str+=it.first+":-\t"+ "value:"+it.second.value+"\t|"+ "address:"+it.second.address+" \n" ;
    } 
    write_to_file(print_tab,write_str) ;
write_str="" ;
  write_to_file(print_tab,"**********************************EXTREF TABLE*****************************\n") ;
    for (auto const& it0: CSECT_TAB) { 
      for (auto const& it: it0.second.EXTREF_TAB) 
        write_str+=it.first+":-\t"+ "name:"+it.second.name+"\t|"+it0.second.name+" \n" ;
    } 
    write_to_file(print_tab,write_str) ;
write_str="" ;
  write_to_file(print_tab,"**********************************EXTDEF TABLE*****************************\n") ;
    for (auto const& it0: CSECT_TAB) {
      for (auto const& it: it0.second.EXTDEF_TAB) {
        if(it.second.name!="undefined")
          write_str+=it.first+":-\t"+ "name:"+it.second.name+"\t|"+ "address:"+it.second.address+"\t|"+" \n" ;
      }
    }  
    write_to_file(print_tab,write_str) ;
  cout<<"\nExecuting PASS2"<<endl;
  pass2();
  cout<<"\nASSEMBLED SUCESSFULLY"<<endl;
  cout<<endl;
}

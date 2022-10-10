#include "useful_methods.cpp"
#include "ALL_TABLES.cpp"
using namespace std;
string fileName="input.txt"; bool error_flag=false; int p_l; string *Blocks_num_to_name; string first_execu_sec;
void handle_LTORG(string& lit_prefix, int& line_number_delta,int line_number,int& LOCCTR, int& last_delta_locctr, int curr_block_number){
  string lit_addr,lit_val; lit_prefix = "";
  for(auto const& it: LITTAB){
    lit_addr = it.second.address; lit_val = it.second.value;
    if(lit_addr!="?"){
    }
    else{
      line_number += 5; line_number_delta += 5;
      LITTAB[it.first].address = int_to_strHex(LOCCTR); LITTAB[it.first].block_num = curr_block_number;
      lit_prefix += "\n" + to_string(line_number) + "\t" + int_to_strHex(LOCCTR) + "\t" + to_string(curr_block_number) + "\t" + "*" + "\t" + "="+lit_val + "\t" + " " + "\t" + " ";
      if(lit_val[0]=='X'){
        LOCCTR += (lit_val.length() -3)/2; last_delta_locctr += (lit_val.length() -3)/2;
      }
      else if(lit_val[0]=='C'){
        LOCCTR += lit_val.length() -3; last_delta_locctr += lit_val.length() -3;
      }
    }
  }
}
void evaluate_expression(string expression, bool& relative,string& temp_operand,int line_number, ofstream& error_File,bool& error_flag){
  string single_operand="?",single_operator="?",val_str="",val_temp="",write_data="";
  int last_operand=0,last_operator=0,pair_count=0; char last_byte = ' '; bool Illegal = false;
  for(int i=0;i<expression.length();){
    single_operand = ""; last_byte = expression[i];
    while((last_byte!='+' && last_byte!='-' && last_byte!='/' && last_byte!='*') && i<expression.length()){
      single_operand += last_byte; last_byte = expression[++i];
    }
    if(SYMTAB[single_operand].exists==1){
      last_operand = SYMTAB[single_operand].relative; val_temp = to_string(str_hex_to_int(SYMTAB[single_operand].address));
    }
    else if((single_operand != "" || single_operand !="?" ) && if_all_num(single_operand)){
      last_operand = 0; val_temp = single_operand;
    }
    else{
      write_data = "Line: "+to_string(line_number)+" : Can't find symbol. Found "+single_operand;
      write_to_file(error_File,write_data); Illegal = true;
      break;
    }
    if(last_operand*last_operator == 1){//Check expressions legallity
      write_data = "Line: "+to_string(line_number)+" : Illegal expression";
      write_to_file(error_File,write_data);  error_flag = true;  Illegal = true;
      break;
    }
    else if((single_operator=="-" || single_operator=="+" || single_operator=="?")&&last_operand==1){
      if(single_operator=="-"){
        pair_count--;
      }
      else{
        pair_count++;
      }
    }
    val_str += val_temp; single_operator= "";
    while(i<expression.length()&&(last_byte=='+'||last_byte=='-'||last_byte=='/'||last_byte=='*')){
      single_operator += last_byte; last_byte = expression[++i];
    }
    if(single_operator.length()>1){
      write_data = "Line: "+to_string(line_number)+" : Illegal operator in expression. Found "+single_operator;
      write_to_file(error_File,write_data); error_flag = true;  Illegal = true;
      break;
    }
    if(single_operator=="*" || single_operator == "/"){
      last_operator = 1;
    }
    else{
      last_operator = 0;
    }
    val_str += single_operator;
  }
  if(!Illegal){
    if(pair_count==1){
      relative = 1; Eval_str tempOBJ(val_str); temp_operand = int_to_strHex(tempOBJ.get_result());
    }
    else if(pair_count==0){
      relative = 0; cout<<val_str<<endl; Eval_str tempOBJ(val_str); temp_operand = int_to_strHex(tempOBJ.get_result());
    }
    else{
      write_data = "Line: "+to_string(line_number)+" : Illegal expression";
      write_to_file(error_File,write_data); error_flag = true; temp_operand = "00000"; relative = 0;
    }
  }
  else{
    temp_operand = "00000"; error_flag = true; relative = 0;
  }
}
void pass1(){
  ifstream source_File; ofstream intermediate_File, error_File;
  source_File.open(fileName);
  if(!source_File){
    cout<<"error in  opening file: "<<fileName<<endl; exit(1);
  }
  intermediate_File.open("intermediate_file.txt");
  if(!intermediate_File){
    cout<<"error in  opening file: intermediate_file.txt"<<endl; exit(1);
  }
  write_to_file(intermediate_File,"Line\tAddress\tinfo_label\tOPCODE\tOPERAND\tComment");
  error_File.open("error_file.txt");
  if(!error_File){
    cout<<"Unable to open file: error_file.txt"<<endl; exit(1);
  }
  write_to_file(error_File,"************PASS1************");  
  string file_Line; string write_data,write_data_suffix="",write_data_prefix=""; int index=0; string current_Block_Name = "DEFAULT";
  int curr_block_number = 0; int total_blocks = 1; bool status_code; string info_label,opcode,operand,comment; string temp_operand;
  int start_addr,LOCCTR,save_LOCCTR,line_number,last_delta_locctr,line_number_delta=0; line_number = 0;last_delta_locctr = 0;
  getline(source_File,file_Line);
  line_number += 5;
  while(Is_comment(file_Line)){
    write_data = to_string(line_number) + "\t" + file_Line; write_to_file(intermediate_File,write_data);
    getline(source_File,file_Line); line_number += 5; index = 0;
  }
  read_first_non_space(file_Line,index,status_code,info_label); read_first_non_space(file_Line,index,status_code,opcode);
  if(opcode=="START"){
    read_first_non_space(file_Line,index,status_code,operand); read_first_non_space(file_Line,index,status_code,comment,true);
    start_addr = str_hex_to_int(operand); LOCCTR = start_addr;
    write_data = to_string(line_number) + "\t" + int_to_strHex(LOCCTR-last_delta_locctr) + "\t" + to_string(curr_block_number) + "\t" + info_label + "\t" + opcode + "\t" + operand + "\t" + comment;
    write_to_file(intermediate_File,write_data);
    getline(source_File,file_Line); line_number += 5; index = 0;
    read_first_non_space(file_Line,index,status_code,info_label);  read_first_non_space(file_Line,index,status_code,opcode);
  }
  else{
    start_addr = 0; LOCCTR = 0;
  }
   string curr_sec_name="DEFAULT" ; int sec_counter=0;
  while(opcode!="END"){  		
  	while(opcode!="END" && opcode!="CSECT"){
    if(!Is_comment(file_Line)){
      if(info_label!=""){
        if(SYMTAB[info_label].exists==0){
          SYMTAB[info_label].name = info_label; SYMTAB[info_label].address = int_to_strHex(LOCCTR); SYMTAB[info_label].relative = 1;
          SYMTAB[info_label].exists = 1; SYMTAB[info_label].block_num = curr_block_number;
 		  if(CSECT_TAB[curr_sec_name].EXTDEF_TAB[info_label].exists == 1){
 		  		CSECT_TAB[curr_sec_name].EXTDEF_TAB[info_label].address=SYMTAB[info_label].address ;
 		  }
        }
        else{
          write_data = "Line: "+to_string(line_number)+" : Duplicate symbol for '"+info_label+"'. Previously defined at "+SYMTAB[info_label].address;
          write_to_file(error_File,write_data); error_flag = true;
        }
      }
      if(OP_TAB[get_real_opcode(opcode)].exists==1){
        if(OP_TAB[get_real_opcode(opcode)].format==3){
          LOCCTR += 3; last_delta_locctr += 3;
          if(get_flag_format(opcode)=='+'){
            LOCCTR += 1; last_delta_locctr += 1;
          }
          if(get_real_opcode(opcode)=="RSUB"){
            operand = " ";
          }
          else{
            read_first_non_space(file_Line,index,status_code,operand);
            if(operand[operand.length()-1] == ','){
              read_first_non_space(file_Line,index,status_code,temp_operand); operand += temp_operand;
            }
          }

          if(get_flag_format(operand)=='='){
            temp_operand = operand.substr(1,operand.length()-1);
            if(temp_operand=="*"){
              temp_operand = "X'" + int_to_strHex(LOCCTR-last_delta_locctr,6) + "'";
            }
            if(LITTAB[temp_operand].exists==0){
              LITTAB[temp_operand].value = temp_operand; LITTAB[temp_operand].exists = 1; LITTAB[temp_operand].address = "?";
              LITTAB[temp_operand].block_num = -1;
            }
          }
        }
        else if(OP_TAB[get_real_opcode(opcode)].format==1){
          operand = " "; LOCCTR += OP_TAB[get_real_opcode(opcode)].format; last_delta_locctr += OP_TAB[get_real_opcode(opcode)].format;
        }
        else{
          LOCCTR += OP_TAB[get_real_opcode(opcode)].format; last_delta_locctr += OP_TAB[get_real_opcode(opcode)].format;
          read_first_non_space(file_Line,index,status_code,operand);
          if(operand[operand.length()-1] == ','){
            read_first_non_space(file_Line,index,status_code,temp_operand); operand += temp_operand;
          }
        }
      }
      else if(opcode == "EXTDEF"){
      	read_first_non_space(file_Line,index,status_code,operand);	int length=operand.length() ;	string inp="" ;
		for(int i=0;i<length;i++){
			while(operand[i]!=',' && i<length){
				inp+=operand[i] ;	i++ ;
			}
			CSECT_TAB[curr_sec_name].EXTDEF_TAB[inp].name=inp ;	CSECT_TAB[curr_sec_name].EXTDEF_TAB[inp].exists=1 ;	inp="" ;
		}
      }
      else if(opcode == "EXTREF"){
      	read_first_non_space(file_Line,index,status_code,operand);	int length=operand.length() ;	string inp="" ;
		for(int i=0;i<length;i++){
			while(operand[i]!=',' && i<length){
				inp+=operand[i] ;	i++ ;
			}
			CSECT_TAB[curr_sec_name].EXTREF_TAB[inp].name=inp ;	CSECT_TAB[curr_sec_name].EXTREF_TAB[inp].exists=1 ;	inp="" ;
		}
      }
      else if(opcode == "WORD"){
        read_first_non_space(file_Line,index,status_code,operand);  LOCCTR += 3;  last_delta_locctr += 3;
      }
      else if(opcode == "RESW"){
        read_first_non_space(file_Line,index,status_code,operand);  LOCCTR += 3*stoi(operand);  last_delta_locctr += 3*stoi(operand);
      }
      else if(opcode == "RESB"){
        read_first_non_space(file_Line,index,status_code,operand); LOCCTR += stoi(operand); last_delta_locctr += stoi(operand);
      }
      else if(opcode == "BYTE"){
        read_byte_operand(file_Line,index,status_code,operand);
        if(operand[0]=='X'){
          LOCCTR += (operand.length() -3)/2;  last_delta_locctr += (operand.length() -3)/2;
        }
        else if(operand[0]=='C'){
          LOCCTR += operand.length() -3;  last_delta_locctr += operand.length() -3;
        }
      }
      else if(opcode=="BASE"){
        read_first_non_space(file_Line,index,status_code,operand);
      }
      else if(opcode=="LTORG"){
        operand = " "; handle_LTORG(write_data_suffix,line_number_delta,line_number,LOCCTR,last_delta_locctr,curr_block_number);
      }
      else if(opcode=="ORG"){
        read_first_non_space(file_Line,index,status_code,operand); char last_byte = operand[operand.length()-1];
        while(last_byte=='+'||last_byte=='-'||last_byte=='/'||last_byte=='*'){
          read_first_non_space(file_Line,index,status_code,temp_operand);  operand += temp_operand; last_byte = operand[operand.length()-1];
        }
        int temp_variable;  temp_variable = save_LOCCTR;  save_LOCCTR = LOCCTR;  LOCCTR = temp_variable;
        if(SYMTAB[operand].exists==1){
          LOCCTR = str_hex_to_int(SYMTAB[operand].address);
        }
        else{
          bool relative; error_flag = false;
          evaluate_expression(operand,relative,temp_operand,line_number,error_File,error_flag);
          if(!error_flag){
            LOCCTR = str_hex_to_int(temp_operand);
          }
          error_flag = false;
        }
      }
      else if(opcode=="USE"){
        read_first_non_space(file_Line,index,status_code,operand); BLOCKS[current_Block_Name].LOCCTR = int_to_strHex(LOCCTR);
        if(BLOCKS[operand].exists==0){
          BLOCKS[operand].exists = 1;  BLOCKS[operand].name = operand; BLOCKS[operand].number = total_blocks++; BLOCKS[operand].LOCCTR = "0";
        }
        curr_block_number = BLOCKS[operand].number; current_Block_Name = BLOCKS[operand].name; LOCCTR = str_hex_to_int(BLOCKS[operand].LOCCTR);
      }
      else if(opcode=="EQU"){
        read_first_non_space(file_Line,index,status_code,operand); temp_operand = ""; bool relative;
        if(operand=="*"){
          temp_operand = int_to_strHex(LOCCTR-last_delta_locctr,6); relative = 1;
        }
        else if(if_all_num(operand)){
          temp_operand = int_to_strHex(stoi(operand),6); relative = 0;
        }
        else{
          char last_byte = operand[operand.length()-1];
          while(last_byte=='+'||last_byte=='-'||last_byte=='/'||last_byte=='*'){
            read_first_non_space(file_Line,index,status_code,temp_operand); operand += temp_operand; last_byte = operand[operand.length()-1];
          }       
          evaluate_expression(operand,relative,temp_operand,line_number,error_File,error_flag);
        }
        SYMTAB[info_label].name = info_label; SYMTAB[info_label].address = temp_operand; SYMTAB[info_label].relative = relative; SYMTAB[info_label].block_num = curr_block_number;
        last_delta_locctr = LOCCTR - str_hex_to_int(temp_operand);
      }
      else{
        read_first_non_space(file_Line,index,status_code,operand); write_data = "Line: "+to_string(line_number)+" : Invalid OPCODE. Found " + opcode;
        write_to_file(error_File,write_data); error_flag = true;
      }
      read_first_non_space(file_Line,index,status_code,comment,true);
      if(opcode=="EQU" && SYMTAB[info_label].relative == 0){
        write_data = write_data_prefix + to_string(line_number) + "\t" + int_to_strHex(LOCCTR-last_delta_locctr) + "\t" + " " + "\t" + info_label + "\t" + opcode + "\t" + operand + "\t" + comment + write_data_suffix;
      } 
      else if(opcode=="EXTDEF" || opcode=="EXTREF"){
        write_data = write_data_prefix + to_string(line_number) + "\t"+" " +"\t" +" "+"\t"+" "+ "\t" + opcode + "\t" + operand + "\t" + comment + write_data_suffix;    
        }else if(opcode=="CSECT"){
          write_data = write_data_prefix + to_string(line_number) + "\t"+int_to_strHex(LOCCTR-last_delta_locctr)+"\t" +" "+"\t"+info_label+ "\t" + opcode + "\t" + " "+ "\t"+" " + write_data_suffix;
    }
      else{
        write_data = write_data_prefix + to_string(line_number) + "\t" + int_to_strHex(LOCCTR-last_delta_locctr) + "\t" + to_string(curr_block_number) + "\t" + info_label + "\t" + opcode + "\t" + operand + "\t" + comment + write_data_suffix;

      }
      write_data_prefix = "";  write_data_suffix = "";
    }
    else{
      write_data = to_string(line_number) + "\t" + file_Line;
    }
    write_to_file(intermediate_File,write_data);
    BLOCKS[current_Block_Name].LOCCTR = int_to_strHex(LOCCTR);
    getline(source_File,file_Line); 
    line_number += 5 + line_number_delta; line_number_delta = 0; index = 0; last_delta_locctr = 0;
    read_first_non_space(file_Line,index,status_code,info_label);  read_first_non_space(file_Line,index,status_code,opcode);
  }  	
  if(opcode!="END"){
     if(SYMTAB[info_label].exists==0){
          SYMTAB[info_label].name = info_label; SYMTAB[info_label].address = int_to_strHex(LOCCTR); SYMTAB[info_label].relative = 1;
          SYMTAB[info_label].exists = 1; SYMTAB[info_label].block_num = curr_block_number;
         }
  	CSECT_TAB[curr_sec_name].LOCCTR=int_to_strHex(LOCCTR-last_delta_locctr,6) ;	CSECT_TAB[curr_sec_name].length=(LOCCTR-last_delta_locctr) ;
  	LOCCTR=last_delta_locctr=0;	curr_sec_name=info_label;	CSECT_TAB[curr_sec_name].name=curr_sec_name ;	sec_counter++;
  	CSECT_TAB[curr_sec_name].section_number=sec_counter ;
  	write_to_file(intermediate_File, write_data_prefix + to_string(line_number) + "\t" + int_to_strHex(LOCCTR-last_delta_locctr) + "\t" + to_string(curr_block_number) + "\t" + info_label + "\t" + opcode);
	getline(source_File,file_Line); 	line_number += 5;
    read_first_non_space(file_Line,index,status_code,info_label);  read_first_non_space(file_Line,index,status_code,opcode); 	
  }
  else{
  	CSECT_TAB[curr_sec_name].LOCCTR=int_to_strHex(LOCCTR-last_delta_locctr,6) ;	CSECT_TAB[curr_sec_name].length=(LOCCTR-last_delta_locctr) ;
  	CSECT_TAB[curr_sec_name].name=curr_sec_name ;	CSECT_TAB[curr_sec_name].section_number=sec_counter ;
  }
}
if(opcode=="END"){
	first_execu_sec=SYMTAB[info_label].address;	SYMTAB[first_execu_sec].name=info_label;	SYMTAB[first_execu_sec].address=first_execu_sec;
}
  read_first_non_space(file_Line,index,status_code,operand);  read_first_non_space(file_Line,index,status_code,comment,true);
  current_Block_Name = "DEFAULT";  curr_block_number = 0;
  LOCCTR = str_hex_to_int(BLOCKS[current_Block_Name].LOCCTR);
  handle_LTORG(write_data_suffix,line_number_delta,line_number,LOCCTR,last_delta_locctr,curr_block_number);
  write_data = to_string(line_number) + "\t" + int_to_strHex(LOCCTR-last_delta_locctr) + "\t" + " " + "\t" + info_label + "\t" + opcode + "\t" + operand + "\t" + comment + write_data_suffix;
  write_to_file(intermediate_File,write_data); int LOCCTR_arr[total_blocks]; Blocks_num_to_name = new string[total_blocks];
  for(auto const& it: BLOCKS){
    LOCCTR_arr[it.second.number] = str_hex_to_int(it.second.LOCCTR); Blocks_num_to_name[it.second.number] = it.first;
  }
  for(int i = 1 ;i<total_blocks;i++){
    LOCCTR_arr[i] += LOCCTR_arr[i-1];
  }
  for(auto const& it: BLOCKS){
    if(it.second.start_addr=="?"){
      BLOCKS[it.first].start_addr= int_to_strHex(LOCCTR_arr[it.second.number - 1]);
    }
  }
  p_l = LOCCTR_arr[total_blocks - 1] - start_addr; source_File.close(); intermediate_File.close();  error_File.close();
}

// Name - Vikash Kumar Verma
// Roll No- 2101CS82

#include<bits/stdc++.h>

using namespace std;

map<string, pair<string,string>> instructions;	//Stores associations of Instructions Table i.e.mnemonic with its opcode and operandinfo
map<string,string>labels;
vector<pair<string,string>>linesofcode;
vector<pair<string,pair<string,string>>>listingfile;
vector<string>machinecode;
vector<string>errors;

bool isoctal(string s);
bool ishexadecimal(string s);
bool isdecimal(string s);
bool isvalidlabelname(string s);
void wrtlogfile();
void fill_instructions();
void firstpass();
void secondpass();
void writetofile();
string convertohex(int num);               // if num is +ve decimal  like a program counter
string decToBinary(int n,int bits);        // converts a decimal to 2's complement form of n bits number
string binTohex(string s);                 // Converts a 2's complement into a hex
string octToBinary(string oct,int bits);   //converts an octal no 078.. to n bit binary no
string hexToBinary(string hex,int bits);    //converts a hex 0xa... to n bits binary
int bintodecimal(string s);

string inputfile;

int main(){
 fill_instructions();
 firstpass();
 secondpass();
 cout<<"Machine code :"<<endl;
 for(auto it:machinecode){
    cout<<it<<endl;
 }
 cout<<"Instruction Lines :"<<endl;
 for(auto it:linesofcode){
   cout<<it<<endl;
 }


return 0;
}



void firstpass(){

cout<<"Enter .asm file to assemble :\n";
cin>>inputfile;
ifstream inptr;
inptr.open(inputfile);
if(inptr.fail()){
    cout << "Couldn't open assembly file." << endl;
		exit(0);
}
string s;
int i=0;
while(getline(inptr,s)){
  // To remove trailing whitespace
  while(s[s.length()-1]==' '){
    s.pop_back();
  }
  // To remove leading whitespace
  reverse(s.begin(),s.end());
  while(s[s.length()-1]==' '){
    s.pop_back();
  }
  reverse(s.begin(),s.end());

  if(s[0]!=';'){ // If line starts with a comment, ignore it completely
   // If line has comment in the ending ,then trim the comment part from it
  int pos=0;bool checkcomment =false;
  for(int i=0;i<s.length();i++){
    if(s[i]==';'){pos=i;checkcomment=true;break;}
  }
  string tempo=s;
  if(checkcomment &&  pos < tempo.length()){
   s=tempo.substr(0,pos);
  }
  // s is now without comment part but may contain trailing whitespace
  while(s[s.length()-1]==' '){
    s.pop_back();
  }
  //Convert memory location into hex form
  string loc=convertohex(i);
  //if It's a label . then  store the association of label with memory location
   bool checklabel=false;string t="";
   for(int i=0;i<s.length();i++){
    if(s[i]==':'){checklabel=true;break;}else{t+=s[i];}
   }
   if(checklabel){
   if(labels.count(t)){//push in errors , a message for duplicate labels;
    string el= "ERROR :Duplicate Label at : "+s;
   errors.push_back(el);
    }
    else{
    // Its a new label but check before its validity
    if(isvalidlabelname(t)){
    labels[t]=loc;
    }else{
    //Invalid label name so record -> error
    string invlabel="ERROR :Invalid label name at : "+s;
    }
    }
    }
  linesofcode.push_back({loc,s}); i++;
    }}


}

void secondpass(){
//Will generate 32 bit machine code instruction
for(auto it:linesofcode){
    int poslab=-1;string mnem="",code,t,machcode="",instr,opl;
   code=it.second; bool labpre=false;
 for(int i=0;i<code.length();i++){
    if(code[i]==':'){poslab=i;labpre=true;break;}
 }

 if(!labpre){
     // label is not present so -> mnemonic + operand format
    instr=code;
    //Now starting from 1st pos until a whitespace should be a valid mnemonic
    int cm=0,posmn=-1;
    for(int i=0;i<instr.length();i++){
        if(instr[i]==' '){posmn=i;break;}else{cm++;}
    }
    mnem=instr.substr(0,cm);
   // cout<<mnem<<endl;
    if(instructions.count(mnem)){
        // Then the mnemonic is valid
        //Now get the operand
      if(posmn==-1){
        // No operand present , check for no of expected operands if less then generate error
        if(instructions[mnem].second!="none"){
        string reqoper="ERROR :Less operand at line : "+it.second;
        errors.push_back(reqoper);
        }else{
        //Its a mnemonic which doesn't take any operand
        machcode+="000000";
        }
      }else{
    // Operand without spaces will start from posmn index of instr substring and end until instr last pos
    int itslen = instr.length()-mnem.length();
      string operwithsp = instr.substr(posmn,itslen); bool isunexop=false;
      //Now trim leading whitespaces from operand
      reverse(operwithsp.begin(),operwithsp.end());
      while(operwithsp[operwithsp.length()-1]==' '){operwithsp.pop_back();}
      reverse(operwithsp.begin(),operwithsp.end());
       opl = operwithsp;
       if(instructions[mnem].second=="none"){
       string unexoper="ERROR :Unexpected operand at line : "+it.second;
        errors.push_back(unexoper);
        isunexop=true;}

      if(!isunexop){
      //its an expected operand so proceed for its validity
      bool isopval= false;
      if(labels.count(opl)){
            //operand may be a label also so check for it
        isopval=true;
        //calculate the offset and then stick the difference  to the pc
        string s1= "0x" + labels[opl];
     string s2= "0x" + it.first;
     int offset = bintodecimal(hexToBinary(s1,24))-bintodecimal(hexToBinary(s2,24))-1;
     string labplslab= binTohex(decToBinary(offset,24));
     machcode+=labplslab;
      }else{
      //operand is not label so may be hex,decimal or oct
      if(ishexadecimal(opl)){
      // in order to get generate machine code  we can directly extend it to 6 bit hex and append it to machcode
      string opapp=opl.substr(2,opl.length()-2);// to exclude '0x' from it
      while(opapp.length()<6){
        opapp+='0';
      }
      machcode+=opapp;
      isopval=true;
      }
      else{
        // its not hexa , so check if its oct or dec
        if(isoctal(opl)){
           // its octal so convert it into 6 bit hex
           string laboct = binTohex(octToBinary(opl,24));
            machcode+=laboct;isopval=true;
        }else{
        // its not octal so check -> for decimal
        if(isdecimal(opl)){
                string labdec=opl;
           machcode+=binTohex(decToBinary(stoi(labdec),24));isopval=true;
        }else{
        // Its none of them so -> record Error if was also not an label
        if(!isopval){
            string operr="ERROR :Invalid Operand at line : "+it.second;
            errors.push_back(operr);
        }
        }
      }
      }
      }
    }}}else{
    // It's an invalid mnemonic and so->record error
    string invmn= "ERROR :Invalid mnemonic at : "+it.second;
    errors.push_back(invmn);
    }
        }
    else{
     //label is present so->check if its  a label without instruction
     if(code[code.length()-1]==':'){
    // its a label without instruction then machcode will be empty
        machcode="        ";
     }else{
     //Or its a label + instruction i.e (mnemonic + opcode)
     // Now we should get the instruction
     poslab++;
      instr = code.substr(poslab,code.length()-poslab);
     //Now trim instruction from leading whitespaces
     reverse(instr.begin(),instr.end());
     while(instr[instr.length()-1]==' '){
        instr.pop_back();
     }
       reverse(instr.begin(),instr.end());
    //Now starting from 1st pos until a whitespace should be a valid mnemonic
    int cm=0,posmn=-1;
    for(int i=0;i<instr.length();i++){
        if(instr[i]==' '){posmn=i;break;}else{cm++;}
    }
    mnem=instr.substr(0,cm);

    if(instructions.count(mnem)){
        // Then the mnemonic is valid
        //Now get the operand
      if(posmn==-1){
        // No operand present , check for no of expected operands if less then generate error
        if(instructions[mnem].second!="none"){
        string reqoper="ERROR :Less operand at line : "+it.second;
        errors.push_back(reqoper);
        }
      }else{
    // Operand without spaces will start from posmn index of instr substring and end until instr last pos
    int itslen = instr.length()-mnem.length();
      string operwithsp = instr.substr(posmn,itslen);
      //Now trim leading whitespaces from operand
      reverse(operwithsp.begin(),operwithsp.end());
      while(operwithsp[operwithsp.length()-1]==' '){operwithsp.pop_back();}
      reverse(operwithsp.begin(),operwithsp.end());
      string opl = operwithsp;
      //Check if the mnemonic requires an operand or not
      bool isunexop=false;
      if(instructions[mnem].second=="none"){
        //unexpected operand
        string unexoper="ERROR :Unexpected operand at line : "+it.second;
        errors.push_back(unexoper);
        isunexop=true;
      }

      if(!isunexop){
        // if its an expected operand then proceed for its valididty
      bool isopval= false;
      if(labels.count(opl)){
            //operand may be a label also so check for it
        isopval=true;
        //calculate the offset and then stick the difference  to the pc
        string s1= "0x" + labels[opl];
     string s2= "0x" + it.first;
     int offset = bintodecimal(hexToBinary(s1,24))-bintodecimal(hexToBinary(s2,24))-1;
     string labplslab= binTohex(decToBinary(offset,24));
     machcode+=labplslab;
      }else{
      //operand is not label so may be hex,decimal or oct
      if(ishexadecimal(opl)){
      // in order to get generate machine code  we can directly extend it to 6 bit hex and append it to machcode
      string opapp=opl.substr(2,opl.length()-2);// to exclude '0x' from it
      while(opapp.length()<6){
        opapp+='0';
      }
      machcode+=opapp;
      isopval=true;
      }
      else{
        // its not hexa , so check if its oct or dec
        if(isoctal(opl)){
           // its octal so convert it into 6 bit hex
           string laboct = binTohex(octToBinary(opl,24));
            machcode+=laboct;isopval=true;
        }else{
        // its not octal so check -> for decimal
        if(isdecimal(opl)){
                string labdec=opl;
           machcode+=binTohex(decToBinary(stoi(labdec),24));isopval=true;
        }else{
        // Its none of them so -> record Error if was also not an label
        if(!isopval){
            string operr="ERROR :Invalid Operand at line : "+it.second;
            errors.push_back(operr);
        }
        }
      }
      }
      }
    }}}else{
    // It's an invalid mnemonic and so->record error
    string invmn= "ERROR :Invalid mnemonic at : "+it.second;
    errors.push_back(invmn);
    }
     }
        }
if(mnem=="data"){
  // machcode will contain 8 bit hex value of operand
  //as this condition is out of two cases so opl is back to its global value empty
  int cp=0,posopl=-1;
  for(int i=0;i<instr.length();i++){
    if(instr[i]==' '){posopl=i;break;}else{cp++;}
  }
   opl = instr.substr(posopl,instr.length()-cp);
  reverse(opl.begin(),opl.end());
  while(opl[opl.length()-1]==' '){opl.pop_back();}
  reverse(opl.begin(),opl.end());
  if(!opl.empty()){
     if(ishexadecimal(opl)){
      // in order to get generate machine code  we can directly extend it to 6 bit hex and append it to machcode
      string opapp=opl.substr(2,opl.length()-2);// to exclude '0x' from it
      while(opapp.length()<8){
        opapp+='0';
      }
      machcode=opapp;

      }
      else{
        // its not hexa , so check if its oct or dec
        if(isoctal(opl)){
           // its octal so convert it into 8 bit hex
           string laboct = binTohex(octToBinary(opl,32));
            machcode=laboct;

        }else{
        // its not octal so check -> for decimal
        if(isdecimal(opl)){
                string labdec=opl;
           machcode=binTohex(decToBinary(stoi(labdec),32));
        }
  }

}}
}

machcode+=instructions[mnem].first;  //add it at last to machcode the two bits(opcode)
if(mnem=="ldc" && labels.count(opl)){machcode=labels[opl];}
listingfile.push_back({it.first,{machcode,it.second}});
machinecode.push_back(machcode);

}
wrtlogfile();

}

void writetofile(){
string s=inputfile.substr(0,inputfile.length()-4);s+=".l";
ofstream myfile(s);
string s1="";
int i1=0;
 for(auto it:listingfile){
 s1="";
 s1+=it.first+" "+machinecode[i1]+" "+it.second.second;
 i1++;
myfile<<s1<<endl;
}
myfile.close();
string m=inputfile.substr(0,inputfile.length()-4);m+=".o";
ofstream myfile2(m);
for(auto it:machinecode){
    string tm;string macode=it;
if(macode.length()!=0){
 for(int i=0;i<macode.length();i+=2){
    tm="";tm+="0x";
    tm+=macode.substr(i,2);
    tm=hexToBinary(tm,8);tm+=' ';
    myfile2<<tm;
 }
myfile2<<endl;
}}
myfile2.close();
}

void wrtlogfile(){
string lo=inputfile.substr(0,inputfile.length()-4);lo+=".log";
ofstream myfile3(lo);
myfile3<<"Log File generated.\n";
if(errors.empty()){
   myfile3<<"No Errors.\nObject file and Listing File generated.\n";
   writetofile();
}else{
    myfile3<<"Object File and Listing File not generated.\nFollowing errors found.\n";
}
for(auto it:errors){
    myfile3<<it<<endl;
}
}


bool ishexadecimal(string s){
    int len = s.length();
    if(len < 2){return false;}else{
    bool check=true;
    if(!((s[0]=='0') && (s[1]=='X' || s[1]=='x'))){check=false;}
    for(int i = 2; i < len; i++){
        if (!((s[i] >= '0' && s[i] <= '9') || ((s[i]>='a' && s[i]<='f') || (s[i]>='A' && s[i]<='F')))){check =false;break;};
    }
    return check;}
}
bool isdecimal(string s){
	int len =s.length();
	if(!(s[0]=='+' || s[0]=='-' || (s[0]>='0' && s[0]<='9'))){return false;}
	for(int i = 1; i < len; i++)
		if(!(s[i] >= '0' && s[i] <= '9')){return false;};
	return true;
}

bool isvalidlabelname(string s){
    string t=s;
  for(int i=0;i<t.length();i++){
    if(t[i]>='A' && t[i] <='Z'){t[i]+=32;}
  }
  if(!(t[0]>='a' && t[0]<='z')){return false;}
   for(int i=1;i<t.length();i++){
    if(!((t[i]>='a' && t[i]<='z')||(t[i]>='0' && t[i]<='9'))){return false;}
   }
	return true;
}
bool isoctal(string s){
if(s.length()<2){return false;}
if(s[0]!='0'){
    return false;
}
for(int i=1;i<s.length();i++){
   if(!(s[i]>='0' && s[i]<='7') ){return false;}
}
return true;
}
void fill_instructions(){
// For each association Mnemonic is key(as String) and Value is pair of {opcode , operand-info} as {string,int}
     instructions["data"]   = {"", "value"};
     instructions["ldc"]    = {"00", "value"};
     instructions["adc"]    = {"01", "value"};
     instructions["ldl"]    = {"02", "offset"};
     instructions["stl"]    = {"03", "offset"};
     instructions["ldnl"]   = {"04", "offset"};
     instructions["stnl"]   = {"05", "offset"};
     instructions["add"]    = {"06", "none"};
     instructions["sub"]    = {"07", "none"};
     instructions["shl"]    = {"08", "none"};
     instructions["shr"]    = {"09", "none"};
     instructions["adj"]    = {"0A" , "value"};
     instructions["a2sp"]   = {"0B" , "none"};
     instructions["sp2a"]   = {"0C" , "none"};
     instructions["call"]   = {"0D" , "offset"};
     instructions["return"] = {"0E" , "none"};
     instructions["brz"]    = {"0F" , "offset"};
     instructions["brlz"]   = {"10" , "offset"};
     instructions["br"]     = {"11", "offset"};
     instructions["HALT"]   = {"12", "none"};
     instructions["SET"]    = {"" , "value"};
}
string convertohex(int num){
string hex="";
int temp =num,r=0;
while(temp!=0){
  r=temp%16;
      if (r < 10){hex+=(char) r + 48;}
        else{hex += (char)r + 55;}
        temp = temp / 16;}
 while(hex.length()<8){hex+='0';}
 reverse(hex.begin(),hex.end());
 return hex;
}

int bintodecimal(string bin){
    int bits=bin.length();
  long long decresult=0,bia=pow(2,bits);
  if(bin[0]=='0'){
    decresult= stoll(bin,nullptr,2);
  }else{
  decresult=bia-stoll(bin,nullptr,2);
  decresult=-decresult;
  }
return decresult;
}
int hextodecimal(string hex){
   string  s=hex.substr(2,hex.length()-2);
 for(int i=0;i<s.length();i++){
    if(s[i]>='a'){s[i]-=32;}
 }
 int ba=1,pv=0,decresult=0;
reverse(s.begin(),s.end());
for(int i=0;i<s.length();i++){
    if(s[i]<='9'){pv=(int)s[i]-48;}else{pv=(int)s[i]-55;}
  decresult+=ba*pv; ba*=16;
}
return decresult;
}
string decToBinary(int n,int bits)
{
  string s;
    if(bits==32){s = bitset<32>(n).to_string();}
    if(bits==24){s = bitset<24>(n).to_string();}
	return s;
}
string binTohex(string s)
{ string res="";
map<string,char>mp;mp["0000"]='0';mp["0001"]='1';mp["0010"]='2';mp["0011"]='3';mp["0100"]='4';
mp["0101"]='5';mp["0110"]='6';mp["0111"]='7';mp["1000"]='8';mp["1001"]='9';mp["1010"]='A';
mp["1011"]='B';mp["1100"]='C';mp["1101"]='D';mp["1110"]='E';mp["1111"]='F';
  string t="";
   for(int i=0;i<32;i+=4){
        t="";
    for(int j=i;j<i+4;j++){
    t+=s[j];
    }
    res+=mp[t];
   }

	return res;
}
string octToBinary(string oct,int bits)
{    string s=oct.substr(1,oct.length()-1);
    string res="";
 map<char,string>oc;oc['0']="000";oc['1']="001";oc['2']="010";oc['3']="011";
 oc['4']="100";oc['5']="101";oc['6']="110";oc['7']="111";
 for(int i=0;i<s.length();i++){
    res+=oc[s[i]];
 }
 reverse(res.begin(),res.end());
 while(res.length()<bits){
    res+='0';
 }
 reverse(res.begin(),res.end());
 return res;
}
string hexToBinary(string hex,int bits){
 string s=hex.substr(2,hex.length()-1);
 for(int i=0;i<s.length();i++){
if(s[i]>='a'){s[i]-=32;}
 }
 map<char,string>hexb;
 hexb['0']="0000";hexb['1']="0001";hexb['2']="0010";hexb['3']="0011";
 hexb['4']="0100";hexb['5']="0101";hexb['6']="0110";hexb['7']="0111";
 hexb['8']="1000";hexb['9']="1001";hexb['A']="1010";hexb['B']="1011";
 hexb['C']="1100";hexb['D']="1101";hexb['E']="1110";hexb['F']="1111";
 string res="";
 for(int i=0;i<s.length();i++){
    res+=hexb[s[i]];
 }
 reverse(res.begin(),res.end());
 while(res.length()<bits){res+='0';}
  reverse(res.begin(),res.end());
  return res;
}

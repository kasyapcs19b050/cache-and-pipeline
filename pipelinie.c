#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<string.h>
#include<stdbool.h>

int convert(char c){// function to convert a character into an integer 
    int k;
    if(c>='0'&&c<='9'){
        k=c-'0';
    }        // if the given character is in between 0 and 9 we want integers as 0 and 9 only
    else if(c>='a'&&c<='f'){
        k=c-'W';
    }       //if the gven character is in b/w a and f then integer must lie b/w 10 and 15
    else{
       k=-1;
    } // if it is not in the above two ranges we return -1  
    return k;
}

char* convert_to_hexa(int p){ //func to convert an integer into a hexa decimal number into two parts like 10 is stored as 0a
    static char v[2];
    if(p<0){
        p+=16;
    }
    if(p/16>=0&&p/16<=9){
        v[0]=p/16+'0';
    } //we first check if integer is negative or not and if it is negative then we add 16 to it and then changes it as we are assured -15<p<16
    else{
        v[0]=p/16+'W';
    } //we now changes the integer p/16 as symbol and store it in first part of two sized array 
    if(p%16>=0&&p%16<=9){
        v[1]=p%16+'0';
    }
    else{
        v[1]=p%16+'W';
    }  //we store p%16 in the second part of two sized array and we return array itself
    return v;
}

int merge(char a,char b){  //func to merge two characters a and b to obtain the integer value of it
    int k;
    if(a>='a'&&a<='f'){
        k=convert(a)-16;
    }
    else{
        k=convert(a);
    } // we first convert the char a into integer by calling convert func after modifying a into the changeable range
    k=16*k+convert(b);   //now adding int(b) to 16k which gives the original value of hexadecimal notation ab
    return k;
}

int main(){
  //opening the input file and taking the file descriptoe value in fd.
    int fd=open("ICache.txt",O_RDWR);
    if(fd<0){
        printf("could not open\n");
        exit(1);
    }

    struct stat statbuf;
    int err = fstat(fd, &statbuf);

    if(err < 0){
        printf("could not open\n");
        exit(2);
    }
   //we are mmap() the input file and store the data as a string in icache
    char *icache = mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(icache == MAP_FAILED){
        printf("Mapping Failed\n");
        return 1;
    }
    close(fd);
   //since icache also contains "\n" as a character we remove the character and store them in string "instr".
    char instr[512];
    int i=0,j=0;

    while(i<512){
        if(j%3!=2){
            instr[i]=icache[j];
           i++;
           j++;
         }
         else{
             j++;
         }

    }


//now we open the "DCache.txt" file and store the file descriptor in fd1.
    int fd1=open("DCache.txt",O_RDWR);
    if(fd1<0){
        printf("could not open\n");
        exit(1);
    }

    struct stat statbuf1;
    int err1 = fstat(fd1, &statbuf1);

    if(err1 < 0){
        printf("could not open\n");
        exit(2);
    }
//mmap() the file and store it as a string in idata.
    char *idata = mmap(NULL,statbuf1.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd1,0);
    if(idata == MAP_FAILED){
        printf("Mapping Failed\n");
        return 1;
    }
    close(fd1);
//since we have "\n" as a character in the string as above,
//so we remove the character and store the data in the string "data".
    char data[512];
    i=0,j=0;

    while(i<512){
        if(j%3!=2){
            data[i]=idata[j];
           i++;
           j++;
         }
         else{
             j++;
         }

    }


//we store the register values in reg[16].
    int reg[16];
    for(i=0;i<16;i++){
        reg[i]=i;
    }
    i=0;
    int z,p;
    char* arr;
    int instr_count=0;
    int arith_count=0;
    int log_count=0;
    int data_count=0;
    int contr_count=0;
    int halt_count=0;
    int l=0;
    int pos=0;
    int pipe_line[300][1500];//it is used to understand the pipelining mechanism using pipe_line matrix.
    int stalls=0;
    for(i=0;i<300;i++){
        for(j=0;j<300;j++)
            pipe_line[i][j]=-1;
    }
    i=0;
    j=0;
    int k;
    int x;
    bool find_col;
    int result;
    //we execute the program until we reach the halt Instruction,which is represented using "f".
    while(instr[i]!='f'){
        /***********************************************************************************************************************/
        j=i;
        i=i+4;//i is effectively used as "PC" because we have four char for each Instruction,we need to increment i by 4.
        instr_count++;
        
        pipe_line[l][pos]=0;//if dependency on a register R_i is found we use i to mark the location i.
        /***********************************************************************************************************************/
        switch (instr[j]){
        /***********************************************************************************************************************/
            case '0':
                   //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback.
                   /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it.
                   while(!find_col){
                       for(x=0;x<l;x++){
                           if(pipe_line[x][pos+k]==convert(instr[j+2])||pipe_line[x][pos+k]==convert(instr[j+3])){
                            k++;
                            break;
                        }
                    }
                    //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.
                   stalls+=k-1;
                   pos=pos+k;
                  pipe_line[l][pos]=0;
                  //Here the dependent instruction is the first register so we keep the value in the matrix.
                  //To know that it is the dependent register.here it is instr[j+1].
                  pipe_line[l][pos+1]=convert(instr[j+1]);
                  pipe_line[l][pos+2]=convert(instr[j+1]);
                  pipe_line[l][pos+3]=0;
                  /***********************************************************************************************************************/
                  arith_count++;//0 is Arithmetic instruction so we increment it by 1.
                   //adding the source registers to store the value in dest register.
                   result=reg[convert(instr[j+2])] + reg[convert(instr[j+3])];
                  /***********************************************************************************************************************/
                   reg[convert(instr[j+1])]=result;
                   
                  break;

            case '1':
                   //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback
                   /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it.
                   while(!find_col){
                       for(x=0;x<l;x++){
                           if(pipe_line[x][pos+k]==convert(instr[j+2])||pipe_line[x][pos+k]==convert(instr[j+3])){
                            k++;
                            break;
                        }
                    }
                    //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.

                   stalls+=k-1;
                   pos=pos+k;
                   //Here the dependent instruction is the first register so we keep the value in the matrix.
                   //To know that it is the dependent register.here it is instr[j+1].

                  pipe_line[l][pos]=0;
                  pipe_line[l][pos+1]=convert(instr[j+1]);
                  pipe_line[l][pos+2]=convert(instr[j+1]);
                  pipe_line[l][pos+3]=0;
                  /***********************************************************************************************************************/
                   arith_count++;//1 is Arithmetic instruction so we increment it by 1.
                   result=reg[convert(instr[j+2])] - reg[convert(instr[j+3])];
                   /***********************************************************************************************************************/
                   reg[convert(instr[j+1])]=result;
                   break;

            case '2':
                   //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback
                   /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it.
                   while(!find_col){
                       for(x=0;x<l;x++){
                           if(pipe_line[x][pos+k]==convert(instr[j+2])||pipe_line[x][pos+k]==convert(instr[j+3])){
                            k++;
                            break;
                        }
                    }
                    //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.

                   stalls+=k-1;
                   pos=pos+k;
                   //Here the dependent instruction is the first register so we keep the value in the matrix.
                   //To know that it is the dependent register.here it is instr[j+1].

                  pipe_line[l][pos]=0;
                  pipe_line[l][pos+1]=convert(instr[j+1]);
                  pipe_line[l][pos+2]=convert(instr[j+1]);
                  pipe_line[l][pos+3]=0;
                  /***********************************************************************************************************************/
                   arith_count++;//2 is Arithmetic instruction so we increment it by 1.
                   result=reg[convert(instr[j+2])] * reg[convert(instr[j+3])];
                   /***********************************************************************************************************************/
                   reg[convert(instr[j+1])]=result;
                   break;

            case '3':
                   //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback
                   /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it.
                   while(!find_col){
                       for(x=0;x<l;x++){
                           if(pipe_line[x][pos+k]==convert(instr[j+1])){
                            k++;
                            break;
                        }
                    }
                    //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.

                   stalls+=k-1;
                   pos=pos+k;
                   //Here the dependent instruction is the first register so we keep the value in the matrix.
                   //To know that it is the dependent register.here it is instr[j+1].

                  pipe_line[l][pos]=0;
                  pipe_line[l][pos+1]=convert(instr[j+1]);
                  pipe_line[l][pos+2]=convert(instr[j+1]);
                  pipe_line[l][pos+3]=0;
                  /***********************************************************************************************************************/
                  arith_count++;//3 is Arithmetic instruction so we increment it by 1.
                   result=reg[convert(instr[j+1])] + 1;
                   /***********************************************************************************************************************/
                   reg[convert(instr[j+1])]=result;
                   break;

            case '4':
                   //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback
                   /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it.
                   while(!find_col){
                       for(x=0;x<l;x++){
                           if(pipe_line[x][pos+k]==convert(instr[j+2])||pipe_line[x][pos+k]==convert(instr[j+3])){
                            k++;
                            break;
                        }
                    }
                    //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.
                   stalls+=k-1;
                   pos=pos+k;
                   //Here the dependent instruction is the first register so we keep the value in the matrix.
                   //To know that it is the dependent register.here it is instr[j+1].

                  pipe_line[l][pos]=0;
                  pipe_line[l][pos+1]=convert(instr[j+1]);
                  pipe_line[l][pos+2]=convert(instr[j+1]);
                  pipe_line[l][pos+3]=0;
                  /***********************************************************************************************************************/
                  log_count++;//4 is Logical instruction so we increment it by 1.
                   result=reg[convert(instr[j+2])] & reg[convert(instr[j+3])];
                   /***********************************************************************************************************************/
                   reg[convert(instr[j+1])]=result;
                   break;

            case '5':
                   //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback
                   /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it.
                   while(!find_col){
                       for(x=0;x<l;x++){
                           if(pipe_line[x][pos+k]==convert(instr[j+2])||pipe_line[x][pos+k]==convert(instr[j+3])){
                            k++;
                            break;
                        }
                    }
                    //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.
                   stalls+=k-1;
                   pos=pos+k;
                   //Here the dependent instruction is the first register so we keep the value in the matrix.
                   //To know that it is the dependent register.here it is instr[j+1].

                  pipe_line[l][pos]=0;
                  pipe_line[l][pos+1]=convert(instr[j+1]);
                  pipe_line[l][pos+2]=convert(instr[j+1]);
                  pipe_line[l][pos+3]=0;
                  /***********************************************************************************************************************/
                  log_count++;//5 is Logical instruction so we increment it by 1.
                   result=reg[convert(instr[j+2])] | reg[convert(instr[j+3])];
                   /***********************************************************************************************************************/
                   reg[convert(instr[j+1])]=result;
                   break;

            case '6':
                   //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback
                   /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it.
                   while(!find_col){
                       for(x=0;x<l;x++){
                           if(pipe_line[x][pos+k]==convert(instr[j+2])||pipe_line[x][pos+k]==convert(instr[j+3])){
                            k++;
                            break;
                        }
                    }
                    //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.
                   stalls+=k-1;
                   pos=pos+k;
                   //Here the dependent instruction is the first register so we keep the value in the matrix.
                   //To know that it is the dependent register.here it is instr[j+1].

                  pipe_line[l][pos]=0;
                  pipe_line[l][pos+1]=convert(instr[j+1]);
                  pipe_line[l][pos+2]=convert(instr[j+1]);
                  pipe_line[l][pos+3]=0;
                  /***********************************************************************************************************************/
                  log_count++;//6 is Logical instruction so we increment it by 1.
                   result=~reg[convert(instr[j+2])];
                   /***********************************************************************************************************************/
                   reg[convert(instr[j+1])]=result;
                   break;

            case '7':
                   //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback
                   /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it.
                   while(!find_col){
                       for(x=0;x<l;x++){
                           if(pipe_line[x][pos+k]==convert(instr[j+2])||pipe_line[x][pos+k]==convert(instr[j+3])){
                            k++;
                            break;
                        }
                    }
                    //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.
                   stalls+=k-1;
                   //Here the dependent instruction is the first register so we keep the value in the matrix.
                   //To know that it is the dependent register.here it is instr[j+1].
                   
                   pos=pos+k;
                  pipe_line[l][pos]=0;
                  pipe_line[l][pos+1]=convert(instr[j+1]);
                  pipe_line[l][pos+2]=convert(instr[j+1]);
                  pipe_line[l][pos+3]=0;
                  /***********************************************************************************************************************/
                  log_count++;//7 is Logical instruction so we increment it by 1.
                   result=reg[convert(instr[j+2])] ^ reg[convert(instr[j+3])];
                   /***********************************************************************************************************************/
                   reg[convert(instr[j+1])]=result;
                   break;

            case '8':
                   
                   //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback
                   /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it
                   while(!find_col){
                       for(x=0;x<l;x++){
                           if(pipe_line[x][pos+k]==convert(instr[j+2])){
                               k++;
                             break;
                        }
                    }
                    //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.
                   stalls+=k-1;
                   //Here the dependent instruction is the first register so we keep the value in the matrix.
                   //To know that it is the dependent register.here it is instr[j+1].
                   
                  pos=pos+k;
                  pipe_line[l][pos]=0;
                  pipe_line[l][pos+1]=convert(instr[j+1]);
                  pipe_line[l][pos+2]=convert(instr[j+1]);
                  pipe_line[l][pos+3]=0;
                  /***********************************************************************************************************************/
                  data_count++;//8 is load instruction so we increment data count  by 1.
                   z=reg[convert(instr[j+2])] + convert(instr[j+3]);
                   int con=convert(data[2*z]);
                   if(con>=10&&con<=15){
                   	   con=con-16;
				   }
		   /**********************************************************************************************************************/		   
                   reg[convert(instr[j+1])]=16*con+convert(data[2*z+1]);
                   //Here the data z is the address from which we have to load the information into the first operand register, address z means z+1 line in the 
                   //data file so we read the two bits at z+1 line in data as dat[2z] and data[2z+1] and we convert them into integers and then to hexa decimal
                   //and store it into the first register operand.
                   /***********************************************************************************************************************/
                   break;

            case '9':
                   //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback
                   /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it;
                   while(!find_col){
                       for(x=0;x<l;x++){
                           if(pipe_line[x][pos+k]==convert(instr[j+2])){
                               k++;
                              break;
                        }
                    }
                   //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.
                   stalls+=k-1;
                   //Here the dependent instruction is the first register so we keep the value in the matrix.
                   //To know that it is the dependent register.here it is instr[j+1].
                  pos=pos+k;
                  pipe_line[l][pos]=0;
                  pipe_line[l][pos+1]=0;
                  pipe_line[l][pos+2]=0;
                  pipe_line[l][pos+3]=0;
                  /***********************************************************************************************************************/
                  data_count++; //9 is a store instruction so we increment the data count by 1
                   z=reg[convert(instr[j+2])] + convert(instr[j+3]);
                   arr=convert_to_hexa(reg[convert(instr[j+1])]);
                   /***********************************************************************************************************************/
                   //Here the data z is the address in which we have to store the information from the first operand register, address z means z+1 line in the 
                   //data file so we read the two bits at z+1 line in data as dat[2z] and data[2z+1] and we convert them into integers and then to hexa decimal
                   //and store it into the data file.
                   /***********************************************************************************************************************/
                   data[2*z]=arr[0];
                   data[2*z+1]=arr[1];
                   /***********************************************************************************************************************/
                   
                   break;

            case 'a':
            /***********************************************************************************************************************/
                  pipe_line[l][pos+1]=0;
                  pipe_line[l][pos+2]=0;
                  pipe_line[l][pos+3]=0;
                  pipe_line[l][pos+4]=0;
                  pos+=3;
                  stalls+=2;  //this case is for jump instruction so we increment contr_count by 1 and we change the i(pc) value to target address and inc stalls
                              //by 2;
             /***********************************************************************************************************************/                 
                  p=merge(instr[j+1],instr[j+2]);
                  i=i+4*p;
                  contr_count++;
             /***********************************************************************************************************************/     
                  break;

            case 'b':
                  
                  //find_col is used to find the pos where we need to keep decode,and execute,mem,writeback
                  /***********************************************************************************************************************/
                   find_col=false;
                   k=1;
                   //until we find a column,we go to the next col to find it.
                  while(!find_col){
                      for(x=0;x<l;x++){
                          if(pipe_line[x][pos+k]==convert(instr[j+1])){
                              k++;
                              break;
                          }
                       }
                   //the above while loop is used to find the relative position k to know the stalls and pos of id.
                   if(x==l)
                      find_col=true;
                   }
                   //so the number of stalls is incremented by k-1.
                   stalls+=k-1;
                   //Here the dependent instruction is the first register so we keep the value in the matrix.
                   //To know that it is the dependent register.here it is instr[j+1].
                  pos=pos+k;
                  pipe_line[l][pos]=0;
                  pipe_line[l][pos+1]=0;
                  pipe_line[l][pos+2]=0;
                  pipe_line[l][pos+3]=0;
                  pos+=2;
                  stalls+=2;
                  /***********************************************************************************************************************/
                  if(reg[convert(instr[j+1])]==0){
                       p=merge(instr[j+2],instr[j+3]);
                       i=i+4*p;
                  }  //this case is for BEQZ so we check if register is 0 or not if it is zero we increase the pc to its target address
                  contr_count++; 
                  /***********************************************************************************************************************/
                  break;

            case 'f':  // this case is for halt so we do nothing as we exit from code
                   break;

        }
        l++;  // we increment the row in pipe_line matrix;
    }

     instr_count++;
     halt_count++;  //incrementing halt and total instr count by 1 as we didn't count halt instr from above loop
    
       FILE* fptr;
     fptr=fopen("DCache_Output.txt","w");   // to open the file Dcache.txt to write our new data values in the given format
    i=0;
    while(i<512){
        char s[3];
        s[0]=data[i];
        s[1]=data[i+1];
        fputs(s,fptr);
        fprintf(fptr,"\n");
        i+=2;
    } // we write two bits of information in each line and then we write total of 256 lines
    fclose(fptr);  //to close the file
    pos=pos+5;     //counting halt cycles also
    float cpi=((float)pos/instr_count);  //calculating CPI value as no.of cycles/instr cout

    
    fptr=fopen("Output.txt","w");
    fprintf(fptr,"Total number of instructions executed: %d\n",instr_count);
    fprintf(fptr,"Number of instructions in each class\n");
    fprintf(fptr,"Arithmetic instructions              : %d\n",arith_count);
    fprintf(fptr,"Logical instructions                 : %d\n",log_count);
    fprintf(fptr,"Data instructions                    : %d\n",data_count);
    fprintf(fptr,"Control instructions                 : %d\n",contr_count);
    fprintf(fptr,"Halt instructions                    : %d\n",halt_count);
    fprintf(fptr,"Cycles Per Instruction               : %f\n",cpi);
    fprintf(fptr,"Total number of stalls               : %d\n",stalls);
    fprintf(fptr,"Data stalls (RAW)                    : %d\n",stalls-2*contr_count);
    fprintf(fptr,"Control stalls                       : %d\n",2*contr_count);
      // We print each of the calculated information to an output file Output.txt
    fclose(fptr);

      return 0;

}

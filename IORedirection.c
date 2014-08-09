#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#define CREATE_FLAGS_O (O_WRONLY | O_CREAT | O_TRUNC)//flags for output file with truncate.
#define CREATE_FLAGS_A (O_WRONLY | O_CREAT | O_APPEND)//flags for output file with append.
#define CREATE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)//flags for input file. 

//Redirect the input file to the stdin of the process.   
int RedirectInput(char *in_file)
{
         //control whether the file is exist or not. 
	    if(access(in_file, F_OK) == -1){
                printf("bash: %s: No such file or directory\n",in_file);
                exit(1);}
        //if the input file is exist.
        else{
                   //control whether the file is readable or not. 
                if(access(in_file, R_OK) == -1){
                     printf("bash: %s: Permission denied\n",in_file);
                     exit(1);}
                  //open the input file to read and redirect it to the stdin of the process.
                else{
	                 freopen(in_file, "r", stdin); 
                     return 1;}
                }
  
}

//Redirect the output file to the stdout of the process. 
int RedirectOutput(char *out_file,int isAppend)
{
	int fd;
       if(isAppend){//open output file with append.
                  fd = open(out_file, CREATE_FLAGS_A, CREATE_MODE);}
       else{             //open output file with truncate.
                  fd = open(out_file, CREATE_FLAGS_O, CREATE_MODE);}
                  
       //control whether the file is writable or not.          
           if(access(out_file, W_OK) == -1){
                    printf("bash: %s: Permission denied\n",out_file);
                    exit(1);}
       //redirect output file to the stdout of the process.
           else{
                    dup2(fd,STDOUT_FILENO);
	                close(fd);
    	            return 1;}
}

//Redirect the input and output file of the process if they is not null.
void RedirectIO(char *in_file,char *out_file,int isAppend)
{

     if(in_file != NULL){
            RedirectInput(in_file);}
     if(out_file != NULL){
            RedirectOutput(out_file,isAppend);}

}


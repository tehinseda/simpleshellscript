#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "InternalCommands.c"
#include "IORedirection.c"
#include "SystemCommands.c"

#define MAX_BUFFER 128                        // max line buffer
#define MAX_ARGS 32                             // max # args
#define SEPARATORS " \t\n"                    // token separators

//construct a struct to create a process.
struct Process{

    char *command[MAX_ARGS];//name of command of a process.
	char *out_file;//output file of a process >.
	char *in_file;//input file of a process <.
	int isAppend;//output file of a process with append >>.

};


struct Process process[MAX_ARGS];//create an array to keep all processes.
char line[MAX_BUFFER];//get line as a string for listjobs and history.
int p;//keep # of processes in the process[] array.
int isBackground;//which indicates whether a process is background or not,if it is 1 that process  is background,else 0.

/*execute each process with childs!*/
void executeProcess() {

  // make 2 pipes; each has 2 fds

  int pipes[4];
  pipe(pipes); // sets up 1st pipe->pipes(0):read,pipes(1):write
  pipe(pipes + 2); // sets up 2nd pipe->pipes(2):read,pipes(3):write

pid_t pid1,pid2,pid3;//pid value for each child.
int ex1,ex2,ex3;//exit status for each child.

 
 if(process[0].command[0] != NULL){//command of first process not null.
  // fork the first child.
 pid1=fork();//1.child.

                if (pid1 == 0)               /*--------------------1.child entry--------------------*/
{
            //redirect input and output file of first process.
                    RedirectIO(process[0].in_file,process[0].out_file,process[0].isAppend);

             // replace stdout with write part of 1st pipe.
                     if(process[0].out_file == NULL && process[1].command[0] != NULL){
                         dup2(pipes[1], 1);

            // close all pipes !
                        close(pipes[0]);close(pipes[1]);close(pipes[2]);close(pipes[3]);}

                    if(checkInternalCommand(process[0].command) == 0){//execute builtin commands if exists.
                        checkSystemCommand(process[0].command);}//else execute system commands.
 }                                             /*----------------------1.child exit----------------------*/


                else
    {/*--------parent continue..--------*/
  
if(process[1].command[0] != NULL){//command of second process not null.
// fork the second child.
pid2=fork();//2.child.

           if (pid2 == 0)                 /*--------------------2.child entry--------------------*/
{
             //redirect output file of second process.
                     RedirectIO(process[1].in_file,process[1].out_file,process[1].isAppend);
	 
	         // replace stdin with read part of 1st pipe
	                     dup2(pipes[0], 0);

	        // replace stdout with write part of 2nd pipe
                     if(process[1].out_file == NULL && process[2].command[0] != NULL){
	                     dup2(pipes[3], 1);}

	         // close all pipes !
                        close(pipes[0]);close(pipes[1]);close(pipes[2]);close(pipes[3]);

                         checkSystemCommand(process[1].command);//else execute system commands.
}                                               /*----------------------2.child exit----------------------*/


                 else
	{/*--------parent continue..--------*/

 if(process[2].command[0] != NULL){//command of third process not null.
// fork the third child .
pid3=fork();//3.child.

	     if (pid3== 0)                    /*--------------------3.child entry--------------------*/
{
            //redirect output file of second process.
                    RedirectIO(process[2].in_file,process[2].out_file,process[2].isAppend);
                    
	        // replace stdin with input read of 2nd pipe
                       dup2(pipes[2], 0);

	       // close all pipes!
                       close(pipes[0]);close(pipes[1]);close(pipes[2]);close(pipes[3]);

                        checkSystemCommand(process[2].command);//else execute system commands.

 }                                           /*----------------------3.child exit----------------------*/
  /*--------parent continue..--------*/
}}
}}
}
    
  // only the parent gets here..
  
  close(pipes[0]);close(pipes[1]);close(pipes[2]);close(pipes[3]);
     
   //if the process is background, parent dont wait for its childrens to finish and add jobs to the list.
   
     if(isBackground){/* add pid to some list to track jobs */
              addJob(pid1,line);	           
        }  
  //if the process is foreground, parent waits for 1-2 or 3 children to finish.
      if(!isBackground)     {
                       if(p==0){//there is 1 child of parent.
                        waitpid(pid1, &ex1, 0);}
                       else if(p==1){//there is 2 child of parent.
                        waitpid(pid1, &ex1, 0);
                        waitpid(pid2, &ex2, 0);}
                       else if(p==2){//there is 3 child of parent.
                        waitpid(pid1, &ex1, 0);
                        waitpid(pid2, &ex2, 0);
                        waitpid(pid3, &ex3, 0);}
                   
                   }
}


/*create each process with its command name,output file,input file!*/
int createProcess(char *args[])
{  
    	isBackground=0;//initially process is foreground.
    	p=0;//index value of process array to indicate each process.
        int i = 0;//index for command line arguments.
        int k =0;//index to create the command name of a process.
 
 //initialize all process index in the process[] array.       
 int a,b;
 for(a=0;a<MAX_ARGS;a++){
    process[a].in_file=NULL;
    process[a].out_file=NULL;
    process[a].isAppend=0;

for(b=0;b<MAX_ARGS;b++){
    process[a].command[b]=NULL;
}
} 

//lets start to create each process.

	for(;args[i]!=NULL;i++)
	{ 
		        if(strcmp(args[i], "<") == 0){//determine input file of process.             		
              		          if(args[i+1]==NULL || i==0){
		                            printf("bash: syntax error near unexpected token '<'\n");
		                            return 1;}
		                      process[p].in_file = args[++i];
		        }
		        else if( strcmp(args[i], ">") == 0){//determine output file of process.
			                  if(args[i+1]==NULL || i==0){
		                            printf("bash: syntax error near unexpected token '>'\n");
		                            return 1;}
		                       process[p].out_file = args[++i];
		        }
               else if( strcmp(args[i], ">>") == 0){//determine output file of process.
                               if(args[i+1]==NULL || i==0){
		                            printf("bash: syntax error near unexpected token '>>'\n");
		                            return 1;}
		                       process[p].out_file = args[++i];
                               process[p].isAppend=1;//output file will be append.
		        }
	           else if( strcmp(args[i], "&") == 0 ){//this process is background process.
		                    if(args[i+1]!=NULL || i==0){
		                            printf("bash: syntax error near unexpected token `&'\n");
		                            return 1;}
		                            isBackground=1;
		        }
		      else if( strcmp(args[i], "|") == 0 ){//pipe will be show that a new process will be.
		                    if(args[i+1]==NULL || i==0){
		                            printf("bash: syntax error near unexpected token '|'\n");
		                            return 1;}
			                p++;//icrease # of processes in the process[] array by one.
			                k=0;//initially k=0 to create command name for new process.
		       }
		       else{
		                            process[p].command[k++] = args[i];//else that arguman will be command name of process.
		                            process[p].command[k]=NULL;//the last index of command name will be null.
		       }

	}

//lets execute each process.
 executeProcess();

return 0;

}


//main function!
int main ()
{
    char buf[MAX_BUFFER];                      // line buffer
    char * args[MAX_ARGS];                     // pointers to arg strings
    char ** arg;                                          // working pointer thru args
    char * prompt = "cse333sh: " ;          // shell prompt

       system("clear");//first clear the screen.
       
/* keep reading input until "exit" command or eof of redirected input */
     
while (!feof(stdin)) { 
    
/* get command line from input */
  
        fputs (prompt, stdout);                // write prompt.
        if (fgets (buf, MAX_BUFFER, stdin )) { // read a line.
        
         	  strcpy(line,buf); //get a copy of line for history and listjobs.  
         	  
     int i; //initially all arguments of the args array will be null for error controls.   	  
         	  for(i=0;i<MAX_ARGS;i++){
                     args[i]=NULL;}

/* tokenize the input into args array */
            
             arg = args;
            *arg++ = strtok(buf,SEPARATORS);   // tokenize input.
             while ((*arg++ = strtok(NULL,SEPARATORS))); // last entry will be NULL.
                                              

                  if (args[0]) {                     // if there's anything there


                            if (!(!strcmp(args[0],"history") &&args[1]==NULL) && strcmp(args[0],"!")){ //every time update history except history or ! command.
                                         updateHistory(line);}//parent update history.
	 
                                     if (!strcmp(args[0],"!")){//if command !;the process will be repeated.
                                             if(checkInternalCommand(args)==1)//if no any error.
                                             {
		                                            strcpy(buf,repeatHistory(atoi(args[1])));//get the process as a buffer in the history with index=args[1].
                                                    strcpy(line,buf);//get copy of that process for listjobs.so line will not be !number anymore in listjobs.
                                                                                                        
                                                    /*again tokenize the input into args array and create process again.*/
 		                                            arg = args;
                                                    *arg++ = strtok(buf,SEPARATORS);   // tokenize input
                                                    while ((*arg++ = strtok(NULL,SEPARATORS)));// last entry will be NULL.
            
                                                                if (!strcmp(args[0],"list_jobs")){//if repeated command is list jobs,we need update each background job status.
                                                                        updateListjobs();
                                                                        createProcess(args);
                                                                        clearListjobs();}   
                                                               else if (!strcmp(args[0],"exit") || !strcmp(args[0],"kill") || !strcmp(args[0],"cd") || !strcmp(args[0],"clr")){
                                                                        //parent interests in exit,kill,clr and cd builtin command.                                                             
                                                                        checkInternalCommand(args);}
                                                               else{
                                                                        createProcess(args);}//else normally create a process again.
                                               }
                                       }
         
                                   else if (!strcmp(args[0],"list_jobs")){//parent and child interest in list_jobs builtin command.
                                                   updateListjobs();//parent update the list of background jobs according to their status before listjobs.
                                                   createProcess(args);//child print the list of background jobs with its output and input redirection.
                                                   clearListjobs();}//parent clear the list of background jobs according to their status after listjobs.
        
                                  else if (!strcmp(args[0],"exit") || !strcmp(args[0],"kill") || !strcmp(args[0],"cd") || !strcmp(args[0],"clr")){
                                      //parent interests in exit,kill,clr and cd builtin command.
                                                   checkInternalCommand(args);}
           
	                              else{
                                                   createProcess(args);}// create process for other commands.
                                                                                         //(other builtin commands history,dir and listjobs is printed by child.)

            }
        }
    }

    return 0; 
 

}

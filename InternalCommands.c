#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#define AMPERSAND "&"                         //for background jobs the listjobs include only name of the process except &
#define MAX_BUFFER 128                        // max line buffer
#define MAX_HISTORY 10                        // max history
//prototypes
void changedir(char*  dir); 
void listdir(char*  dir);
void listJobs();       
void killWithIndex(int index) ;  
void killWithPid(pid_t pid) ;  
void listHistory(); 
char* repeatHistory(int number); 
void Exit();

/*Create a data structure queue to keep the history*/
int historyid=1;//each history has an id.and max 10 recent history will be kept.
struct History{   
    char line[MAX_BUFFER];//history name.
	struct History *nexthistory;//each history keeps the next history.
};
typedef struct History history;
typedef history *Historyptr;
Historyptr headhistory=NULL,tailhistory=NULL;//head and tail of the history queue.

/*Create a data structure linklist to list the background jobs*/
int counter=1;//determine the index value of the job and when the finished list is cleart, the counter will be again 1.
struct Job{
    pid_t pid;//pid of job.
    int index;//index value of job.
    char name[MAX_BUFFER];//name of the background process.
    int flag;//indicates the status of the job, either running,finished or cleared.
	struct Job *nextjob;//each job keeps the next background job.
};
typedef struct Job job;
typedef job *Jobptr;
Jobptr topjob=NULL,rearjob=NULL;//top and rear of the job list.

/*check Internal command and child try to execute with any builtin command*/
int checkInternalCommand(char *args[]){
                 if (!strcmp(args[0],"cd")) { // "cd" command    
                              
                            if(args[2]!=NULL){//usage error.
		                             fprintf(stdout,"Usage: cd <directory>\n");
			                         return -1;}
                            changedir(args[1]);//args[1]=<directory>
                            return 1;
                            
                }
                 else if (!strcmp(args[0],"dir")){   // "dir" command
                 
                             if(args[2]!=NULL){//usage error.
		                             fprintf(stdout,"Usage: dir <directory>\n");
			                         exit(1);}
		                      listdir(args[1]);//args[1]=<directory>
                              return 1;
                              
                }               
                 else if (!strcmp(args[0],"list_jobs")){  // "list_jobs" command
                 
                               if(args[1]!=NULL){//usage error.
		                             fprintf(stdout,"Usage: list_jobs\n");
			                         exit(1);}
		                       listJobs();
                               return 1;
                               
                }
		        else if (!strcmp(args[0],"kill")){  // "kill" command
		        
		                    if(args[1]==NULL || args[2]!=NULL){//usage error.
		                             fprintf(stdout,"Usage: kill %%num or kill num\n");
			                         return -1;}
			                //kill a job with its index value.         
		                    else if(strchr(args[1], '%')!=NULL){
		                            if(args[1][1]!='\0'){		   
		                                int index=args[1][1]-'0';//index value after %.to get integer value '0' is used.
		                                killWithIndex(index);}
		                            else{//usage error.
		                                fprintf(stdout,"Usage: kill %%num\n");}}
		                      //kill a job with its pid value.             
		                    else {
		                                 pid_t pid= atoi(args[1]);//convert pid string to int.
		                                 killWithPid(pid); }
                            return 1;
                            
                }
                else if (!strcmp(args[0],"history")){  // "history" command
                   
                              if(args[1]!=NULL){//usage error.
		                             fprintf(stdout,"Usage: history\n");
			                         exit(1);}
		                                listHistory();
                                        return 1;
                                        
                }
		        else if (!strcmp(args[0],"!")){  // "history" command
		        
		                           if(args[1]==NULL || args[2]!=NULL){//usage error.
		                                fprintf(stdout,"Usage: ! number\n");
			                            return -1;}
			                            
			                       else if(headhistory == NULL){//no history is inserted to the queue.
			                            printf("There is no history.\n");
			                            return -1;}
			                            
		                           else if(atoi(args[1])!=-1 && (atoi(args[1])<=0 || atoi(args[1])>=historyid)){//range history error.		      
		                                fprintf(stdout,"There is no such command id=%s in the history.\n",args[1]);
			                            return -1;}
                                   return 1;
                                   
                }
                else if (!strcmp(args[0],"clr")) { // "clr" command
                                        system("clear");
                                        return 1;
                }
                else if (!strcmp(args[0],"exit")){   // "exit" command
		                                Exit();//before exit from shell,check whether background job exists or not.
		                                return -1;
                }
              
	return 0;//no found available buitin jobs.
}

/*------------------------------------------------cd------------------------------------------------------*/

void changedir(char* dir)
{/*change directory if possible*/
    char *cwd;
            if(dir=='\0'){               //if user entered nothing after cd.
                cwd=getenv("PWD"); //change to current directory.
                printf("%s\n",cwd);}
            else{
                    if(chdir(dir)==-1){ //if chdir function returned error.  
                           printf("bash: cd: %s: No such file or directory\n",dir);}
	                else{
                            if ((cwd = getcwd(NULL, 64)) == NULL) {//get the current working directory.
                                    perror("pwd");
                                    exit(1);}
                                    
                            setenv("PWD",cwd,1);//set again PWD after chdir.
                            printf("%s\n", cwd); //== printf("pwd=%s\n",getenv("PWD"));
                            free(cwd); /* free memory allocated by getcwd() */
         
	                        }
                    }
} 

/*------------------------------------------------dir------------------------------------------------------*/
void listdir(char* dir)
{/*list the contents of directory */
         if(dir=='\0'){             //if user entered nothing after dir.
                system("ls");}   //list the contents of the current directory.
         else{
                if(chdir(dir)==-1){     //change directory to <directory> to list the contents of that.
                         printf("dir: cannot access %s: No such file or directory\n",dir);}
	            else{
                        system("ls");     //list the contents of <directory>
                        chdir(getenv("PWD"));}//set again cwd , take the old value of the cwd from PWD because that is not changed.
                 }
         exit(0);//kill the child.
}

/*----------------------------------------------list_jobs---------------------------------------------------*/
//---------add a new Background job to the list.*/
void addJob(pid_t pid,char line[])
{

      char *token;//get name of the command except &.
                 token = strtok(line, AMPERSAND);

Jobptr newjob;
           newjob=malloc(sizeof(job));//create a new job.

            if(newjob!=NULL){
                strcpy((newjob->name),token);//name of the job.
                newjob->pid=pid;//pid value of the job.
                newjob->index=counter;//index value of the job.
                newjob->nextjob=NULL;//keep the next job.
                newjob->flag=1;//initially flag=1 that shows that it is running.

                        if(topjob==NULL){//if there is no job,initially it is top.
	                            topjob=newjob;}
                        else{//else add to the rear of the list.
	                            rearjob->nextjob=newjob;}

	                    rearjob=newjob;//the rear will be newjob.
             }
   
counter++;//give an id for each job.when there is no background job in the list,the counter will be 1.
}

//-----------check the status of the jobs whether it is terminated or still running.*/
void updateListjobs()
{

int status;//exit status of the process.
Jobptr currentjob=topjob;

    while(currentjob!=NULL){
        /*if the waitpid return the pid value of the job that means the process exit.else -1 is returned.*/
            if((currentjob->pid == waitpid(currentjob->pid, &status, WNOHANG))){//WNOHANG=dont block or suspend the parent process.
                currentjob->flag=0;}//flag=0 shows that the running process will be finished process.
                
                currentjob=currentjob->nextjob;//check all process in the list until it is null.
      }  
}

/*----------list Background jobs*/
void listJobs()
{
 Jobptr currentjob=topjob;
        if(currentjob == NULL){//if topjob is null,no any job.
                printf("There is no background job..\n");}

        else{
            //print the running background jobs in the list.
                printf("Running:\n");//Running!
                        while(currentjob!=NULL){
    
                                if(currentjob->flag == 1){//if flag=1,it is running.
                                    printf("  [%d]   %s  (Pid=%ld)\n",currentjob->index,currentjob->name,(long)(currentjob->pid)); }
    
                         currentjob=currentjob->nextjob;//check all process in the list until it is null.
                          }
            //print the finished background jobs in the list.               
                currentjob=topjob;
                printf("Finished:\n");//Finished!
                        while(currentjob!=NULL){
    
                                if(currentjob->flag == 0){//if flag=0,it is finished.
                                    printf("  [%d]   %s  (Pid=%ld)\n",currentjob->index,currentjob->name,(long)(currentjob->pid)); }
    
                        currentjob=currentjob->nextjob;//check all process in the list until it is null.
                          }
              }
exit(0);//kill the child.
}

/*-----------clear the list*/
void clearListjobs()
{
Jobptr currentjob=topjob;
int controlback=0;//control whether there is still running job or not.
            while(currentjob!=NULL){
    
                if(currentjob->flag == 0){//flag=0 finished jobs.
                            currentjob->flag=-1;}//make all finished jobs' flag -1 because of dont print second time in the finished list.
                else if(currentjob->flag == 1){//if there is any job thats flag is one.
                            controlback=1;}//that means there is still running backgrounds!
       
             currentjob=currentjob->nextjob; 
            }
            
Jobptr temp;
currentjob=topjob;
        if(controlback==0){//if there isnot still running backgrounds lets clear the list and reset the counter and pointers.!

                while(currentjob!=NULL){
                            temp=currentjob;//keep the value of current in temp.
                            currentjob=currentjob->nextjob;//current go next job.
                            free(temp);//free it.
                }

counter=1;//when no background job exists, the job number should reset to 1.
topjob=NULL;//reset topjob to null.
rearjob=NULL;//reset rearjob to null.
        }
          
}   

/*----------------------------------------------kill---------------------------------------------------*/
/*----search the index value of the process to kill it*/
void killWithIndex(int index)
{
Jobptr tmp = topjob;//start with topjob to find the appropriate index value.

    while (tmp != NULL)
    {
        if (tmp->index == index && tmp->flag==1)//if there is a  running process with such index value kill it.
        {
            tmp->flag=0;//that running process fall in the finished list.
            kill(tmp->pid,SIGKILL);//kill that process with sending kill signal to its pid value.
            fprintf(stderr, "  [%d]\tTerminated\t\%s\n" ,tmp->index,tmp->name);
            return;
        }
        tmp=tmp->nextjob;//continue search the list.
    }
    fprintf(stderr, "There is no such running process with index = %d\n" , index);//no found any process.
}
/*----search the pid value of the process to kill it*/
void killWithPid(pid_t pid)
{
Jobptr tmp = topjob;;//start with topjob to find the appropriate pid value.

    while (tmp != NULL)
    {
        if (tmp->pid == pid && tmp->flag==1)//if there is a  running process with such pid value kill it.
        {
            tmp->flag=0;//that running process fall in the finished list.
            kill(tmp->pid,SIGKILL);//kill that process with sending kill signal to its pid value.
            fprintf(stderr, "  [%d]\tTerminated\t\%s\n" ,tmp->pid,tmp->name);
            return;
        }
        tmp=tmp->nextjob;//continue search the list.
    }
    fprintf(stderr, "There is no such running process with pid = %d\n" , pid);//no found any process.
}

/*----------------------------------------------history---------------------------------------------------*/
/*-----------update history*/
void updateHistory(char buf[])
{
Historyptr newhistory,temphistory;
                   newhistory=malloc(sizeof(history));//create a new history record.

         if(newhistory!=NULL){
                strcpy((newhistory->line),buf);//get the last entered command by user.
                newhistory->nexthistory=NULL;//keep the next history.

                     if(headhistory==NULL){//if there is no any record,initially it is head.
	                    headhistory=newhistory;}
                    else{
	                    tailhistory->nexthistory=newhistory;}//else add to the tail of the queue.
            
           tailhistory=newhistory;//the tail of the queue will be new record.
        }
	
        if(historyid>MAX_HISTORY){//maximum 10 most recent history will be kept.
                temphistory=headhistory;//oldest record will be deleted from the queue.
                headhistory=headhistory->nexthistory;//head history will be next.
                free(temphistory);//free the 1.history.   
        }
        else{
	            historyid++;}//give an id for each job.when there is no background job in the list,the counter will be 1.
}

/*------------list history*/
void listHistory()
{
Historyptr currenthistory=headhistory;
int id=1;
        if(currenthistory == NULL){//if head is null,no any history.
                printf("There is no history..\n");}

        else{
                printf("ID\tCMD\n");
                while(currenthistory!=NULL){
                           printf("%d.\t%s",id,currenthistory->line);//print the history with id.
                
                currenthistory=currenthistory->nexthistory;//continue to print history.
                id++;//each history record has an id value.

                }
        }
exit(0);//kill the child.
}

/*---------repeat the any command in the history!*/
char* repeatHistory(int number)
{
Historyptr currenthistory=headhistory;
int id=1;

      if(number == -1){//if "! -1" is entried, repeat the last command.
          return tailhistory->line;//tail keeps the last command.
     }
     else{//else search for the command that's id is equal to number.
            while(currenthistory!=NULL){

               if(number==id){//if we find that number =  id of the command.
                return currenthistory->line;}//repeat that command.
            currenthistory=currenthistory->nexthistory;//continue to search the id.
            id++;//increase id by one if it is not found.
             }
       }
    return NULL;
}

/*----------------------------------------------exit---------------------------------------------------*/
void Exit()
{
Jobptr tmp = topjob;
int controlback=0;//control whether there is still running job or not.
         updateListjobs();
    while (tmp != NULL)
    {
        if (tmp->flag==1)//if there is any job thats flag is one.
        {
            controlback=1;//that means there is still running backgrounds!  
        }
        tmp=tmp->nextjob;
    }

     if(controlback){//notify the user that there are background processes still running.
            fprintf(stderr, "There are background processes still running.\n"); 
    }
    else{//if there is no background jobs directly exit from the shell.
        exit(0);}
   		
}




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

extern int errno;        // system error number.
 
void syserr(char* );    // error report and abort routine. 

/*check System command and child try to execute with execl or execvp*/
int checkSystemCommand(char *args[]){

 	char *cmd=args[0];//file name or path name of the command.


      	if(strchr(cmd, '/')!=NULL)//if it is an absolute path,execl will be run.
      	{/*cmd=absolute path. like /bin/ls
      	    arg[0]=again path. like /bin/ls or ls
      	    others:options of the command. like -l,-a,-B
      	    the last parameter=NULL.*/
                execl(cmd,args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11],
                                    args[12],args[13],args[14],args[15],args[16],args[17],args[18],args[19],args[20],args[21],args[22],
                                    args[23],args[24],args[25],args[26],args[27],args[28],args[29],args[30],args[31],NULL);
                    
		        syserr(cmd);//if child is fail to execute a command,raise error.
	   }
 

	  else{
	      /*cmd=name of the command. like ls
      	    args:array of the commands. like ls -l,-a,-B
      	    the last parameter=NULL.*/
  		        execvp(cmd,args);
  		        
		        syserr(cmd);//if child is fail to execute a command,raise error.
	 }


	return 0;
}

void syserr(char * msg)   // report error code and abort.
{
   fprintf(stderr,"%s: %s\n", strerror(errno), msg);//error.
   exit(1);//kill the child.
}

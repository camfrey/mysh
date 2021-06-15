#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glob.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utmpx.h>
#include <pthread.h>
#include "user.h"
#include "sh.h"

#define READ_END 0
#define WRITE_END 1
void insert(char *name);
void print(bool front2back);
void freeList();
void delete(char *nameBuffer);

pid_t	pid,pidPipe,last_pid;
char 	prompt[PROMPTMAX] = " ";
int		noclobber = 0;
user_t *head;
pthread_mutex_t mLock;

static void *mythread(void *param)
{
	int i = 0;
	const char *name=param;
	struct utmpx *up;
	user_t *temp;
	temp = head;
    while(1) {
    	setutxent();			/* start at beginning */
    	while (up = getutxent()){ /* get an entry */
			if(pthread_mutex_lock(&mLock) != 0){
				perror("Mutex lock error");
			}
			temp = head;	
			while(temp!=NULL){
				//printf("Hi there : %s",temp->name);
				if ( up->ut_type == USER_PROCESS && strcmp(up->ut_user,temp->name)==0){	/* only care about users */
        			printf("\n%s has logged on %s from %s\n", up->ut_user, up->ut_line, up ->ut_host);
				}
				temp = temp->next;
			}
			if(pthread_mutex_unlock(&mLock) != 0){
				perror("Mutex lock error");
			}
      	}
    // printf("PRE SLEEP\n");
    sleep(20);
	}
}

void fileRedirect(char *type,char *file);

void sig_handler(int sig)
{
	switch(sig){
		pid_t child;
		int   status;
		char *cwd;
		case SIGINT:
			cwd = getcwd(NULL, 0);
			fprintf(stdout, "\n%s [%s] >> ",prompt,cwd);
			fflush(stdout);
			free(cwd);
			break;
		case SIGTSTP:
			cwd = getcwd(NULL, 0);
			fprintf(stdout, "\n%s [%s] >> ",prompt,cwd);
			fflush(stdout);
			free(cwd);
			break;
		case SIGCHLD:
			child = waitpid(pid, &status, 0);
			printf("\n");
			break;
	}
}
  
int main(int argc, char **argv, char **envp){

	char	buf[MAXLINE];
	char    *arg[MAXARGS];  // an array of tokens
	char    *ptr;
    char    *pch;
	char	lwd[MAXLINE];
	int	status = 0;
	int i, arg_no;
	bool redirect = false;
	bool pipeBool = false;
	bool pipeBuiltIn = false;
	bool pthread_b = 0;
	int redirectIndex = 0;
	int pipeIndex = 0;
	int pipefd[2];
	char *eofStatus;
	pthread_t tid1;

    signal(SIGINT, sig_handler);//Signal Handlers
	signal(SIGTSTP, sig_handler);//Ignore CTRL Z and SIGTERM
	signal(SIGTERM, SIG_IGN);
	signal(SIGCHLD,sig_handler);
	char *cwd = getcwd(NULL, 0);
	fprintf(stdout, "%s [%s] >> ",prompt,cwd);	/* print prompt (printf requires %% to print %) */
	fflush(stdout);
	free(cwd);

	while ((eofStatus = fgets(buf, MAXLINE, stdin)) != NULL){
		if (strlen(buf) == 1 && buf[strlen(buf) - 1] == '\n')
			goto nextprompt;  					// "empty" command line
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; 			/* replace newline with null */	
		
		//// parse command line into tokens (stored in buf)
		redirectIndex = pipeIndex = 0;
		redirect = pipeBool = pipeBuiltIn = false;
		arg_no = 0;
        pch = strtok(buf, " ");
        while (pch != NULL && arg_no < MAXARGS){
			arg[arg_no] = pch;
			if(arg[arg_no][0] == '>' || arg[arg_no][0] == '<'){
				redirect = true;
				redirectIndex = arg_no;
			}
			else if(arg[arg_no][0] == '|'){
				pipeBool = true;
				pipeIndex = arg_no;
				if (pipe(pipefd) == -1) {
					perror("Pipe Error");
					goto nextprompt;
				}
			}
			arg_no++;
        	pch = strtok (NULL, " ");
        }
		arg[arg_no] = (char *) NULL;

		if (arg[0] == NULL){ // "blank" command line
			goto nextprompt;
		} 

		// print tokens
		// for (i = 0; i < arg_no; i++)
		//   printf("arg[%d] = %s\n", i, arg[i]);

        if (strcmp(arg[0], "pwd") == 0) { // built-in command pwd 
			printf("Executing built-in [pwd]\n");
	        ptr = getcwd(NULL, 0);
            printf("%s\n", ptr);
            free(ptr);
	    }
		else if(strcmp(arg[0],"exit") == 0){ // built-in command exit
			printf("Exiting shell\n");
			if(pthread_b){
				printf("Free pthread and linked list\n");
				pthread_cancel(tid1);
				pthread_join(tid1,NULL);
				freeList();
			}
			exit(0);
		}
		else if(strcmp(arg[0], "which") == 0) { // built-in command which
			struct pathelement *p, *tmp;
            char *cmd;          
			printf("Executing built-in [which]\n");
			if (arg[1] == NULL) {  // "empty" which
		    	printf("which: Too few arguments.\n");
		    	goto nextprompt;
            }
			p = get_path();
           /***/
			tmp = p;
			while (tmp) {      // print list of paths
		    	printf("path [%s]\n", tmp->element);
		    	tmp = tmp->next;
            }
           /***/
            cmd = which(arg[1], p);
            if (cmd) {
		    	printf("%s\n", cmd);
                free(cmd);
            }
			else{
				printf("%s: Command not found\n", arg[1]);// argument not found
			}
			while (p) {   // free list of path values
		    	tmp = p;
		    	p = p->next;
		    	free(tmp->element);
		    	free(tmp);
            }
	    }
		else if(strcmp(arg[0], "where") == 0){//Where Built in function
			struct pathelement *p, *tmp;
            char *cmd;
                    
			printf("Executing built-in [where]\n");

			if (arg[1] == NULL) {  // "empty" where
		    	printf("where: Too few arguments.\n");
		    	goto nextprompt;
            }

			p = get_path();
           /***/
			tmp = p;
			while (tmp) {      // print list of paths
		    	printf("path [%s]\n", tmp->element);
		    	tmp = tmp->next;
            }
           /***/

            where(arg[1],p);

			while (p) {   // free list of path values
		    	tmp = p;
		    	p = p->next;
		    	free(tmp->element);
		    	free(tmp);
            }
		}
		else if(strcmp(arg[0], "cd") == 0){	//built in command cd
			if(arg[1] == NULL){				//if no arguments passed change to home directory
				char *temp = getcwd(NULL, 0);
				for(int i = 0; i < MAXLINE; i++){
					lwd[i] = temp[i];
					if(temp[i]=='\0') break;	
				}
				if(chdir(getenv("HOME")) == -1)
					perror("CD Error");
				free(temp);
				// if(errno > 0) perror("CD Error");
				// errno=0;
			}
			else if(strcmp(arg[1],"..") == 0){ //Switch to above directory
				char *temp = getcwd(NULL, 0);
				for(int i = 0; i < MAXLINE; i++){
					lwd[i] = temp[i];
					if(temp[i]=='\0') break;	
				}
				if(chdir("..") == -1)
					perror("CD Error");
				free(temp);
				// if(errno > 0) perror("CD Error");
				// errno=0;
			}
			else if(strcmp(arg[1],"-") == 0){ //Switch to directory previously in
				chdir(lwd);
			}
			else{ //Switch to previous 
				char *temp = getcwd(NULL, 0);
				for(int i = 0; i < MAXLINE; i++){
					lwd[i] = temp[i];
					if(temp[i]=='\0') break;	
				}
				if(chdir(arg[1]) == -1)
					perror("CD Error");
				free(temp);
				// if(errno > 0) perror("CD Error");
				// errno=0;
			}
		}
		else if(strcmp(arg[0], "prompt") == 0){ // built-in command prompt
			printf("Executing built-in [prompt]\n");
			if(arg[1] == NULL){
				printf("Input a prompt:"); // if no prompt is given, asks for a prompt
				if(fgets(buf, PROMPTMAX, stdin) != NULL){
					for(int i = 0; i < PROMPTMAX; i++){ //copies string to buf, without the newline character.
						if(buf[i] == '\n'){
							continue;
						}
						else if(buf[i]=='\0'){
							break;
						}	
						else{
							prompt[i] = buf[i];
						}
						
					}
					printf("\n");
				}
			}
			else{
				for(int i = 0; i < PROMPTMAX; i++){ // copies prompt give as argument to buf
					prompt[i] = arg[1][i];
					if(arg[1][i] == '\0') break;	
				}
			}
		}
		else if(strcmp(arg[0], "pid") == 0){ // built in command PID
			printf("Executing built-in [pid]\n");
			pid = getpid();
			printf("PID is: %d\n",pid);
		}
		else if(strcmp(arg[0], "list") == 0){	//Built in command list	
			printf("Executing built-in [list]\n");
			struct dirent *thing;
			if(arg[1]==NULL){ // lists cwd
				cwd = getcwd(NULL,0);
				DIR *directory = opendir(cwd);
				thing = readdir(directory);
				while(thing){
					printf("%s\n",thing->d_name);
					thing = readdir(directory);
				}
				closedir(directory);
				free(cwd);
			}
			else{
				for(int i = 1; arg[i]!=NULL; i++){ 	//lists directory given as argument
					char thingy[80];
					if(arg[i][0!='/']){
						DIR *directory = opendir(arg[i]);
						thing = readdir(directory);
						printf("%s:\n",arg[i]);
						while(thing){
							printf("%s\n",thing->d_name);
							thing = readdir(directory);
						}
						closedir(directory);
					}
					else{ 							// relative pathnames
						strcpy(thingy, "./");
						strcat(thingy,arg[i]);
						DIR *directory = opendir(thingy);
						thing = readdir(directory);
						printf("%s:\n",arg[i]);
						while(thing){
							printf("%s\n",thing->d_name);
							thing = readdir(directory);
						}
						closedir(directory);
					}
					
				}
			}
			if(errno > 0) perror("List Error");
			// errno=0;
		}
		else if(strcmp(arg[0], "kill") == 0){ // built in function kill
			int ret;
			printf("Executing built-in [where]\n");
			if(arg[1] == NULL){
				printf("Too few arguments\n");
				goto nextprompt;
			} 
			else if(arg[1][0] == '-' && arg[2] != NULL){ // changes type of kill signal before killing the PID given as arg2
				// printf("SLAUGHTERED\n");
				char temp[3];
				int i = 1;
				while(arg[1][i]!='\0'){
					temp[i-1]=arg[1][i];
					i++;
				}
				temp[i] = '\0';
				ret = kill((pid_t)atoi(arg[2]),atoi(temp));
				if(ret == EINVAL)
					printf("INVALID SIGNAL\n");
				else if(ret == EPERM)
					printf("IMPROPER PERMISSIONS\n");
				else if(ret == ESRCH)
					printf("%s: Does not exist\n",arg[2]);
			} 
			else{ // kill if just given a PID as arg1
				// printf("SLAUGHTERED\n");
				ret = kill((pid_t)atoi(arg[1]),SIGKILL);
				if(ret == EINVAL)
					printf("INVALID SIGNAL\n");
				else if(ret == EPERM)
					printf("IMPROPER PERMISSIONS\n");
				else if(ret == ESRCH)
					printf("%s: Does not exist\n",arg[1]);
			}
			if(errno > 0) perror("Kill Error");
			// errno=0;
		}
		else if(strcmp(arg[0], "printenv") == 0){ // built in function printenv
			printf("Executing built-in [printenv]\n");
			if(arg[1] == NULL){
				int i = 0;
				while(__environ[i] != NULL){
					printf("%s\n",__environ[i++]);
				}
			}
			else if(arg_no == 2)
				printf("%s\n",getenv(arg[1]));
			else 
				printf("printenv: Too many arguments.\n");
		}
		else if(strcmp(arg[0], "setenv") == 0){ //Built in command setenv
			printf("Executing built-in [setenv]\n");
			if(arg[1] == NULL){
				int i = 0;
				while(__environ[i] != NULL){
					printf("%s\n",__environ[i++]);
				}
			}
			else if(arg_no == 2)
				setenv(arg[1],"",0);
			else if(arg_no == 3)
				setenv(arg[1],arg[2],1);
			else
				printf("printenv: Too many arguments.\n");
			if(errno > 0) perror("Setenv Error");
			// errno=0;
		}
		else if(strcmp(arg[0], "noclobber") == 0){
			printf("Executing built-in [noclobber]\n");
			noclobber = !noclobber;
			printf("%d\n",noclobber);
		}
		else if(strcmp(arg[0], "watchuser") == 0){
			printf("Executing built-in [watchuser]\n");
			char *user = arg[1];
			if(!pthread_b){
				pthread_mutex_init(&mLock,NULL);
				pthread_create(&tid1, NULL, mythread, "Thread 1");
				pthread_b=true;
			}
			if(arg[2]==NULL){
				if(pthread_mutex_lock(&mLock) != 0){
					perror("Mutex lock error");
				}
				insert(user);
				// print(false);
				printf("watchuser activated: %s\n", user);
				if(pthread_mutex_unlock(&mLock) != 0){
					perror("Mutex lock error");
				}
			}
			else if(strcmp(arg[2],"off")==0){
				if(pthread_mutex_lock(&mLock) != 0){
					perror("Mutex lock error");
				}
				delete(user);
				// print(false);
				printf("watchuser removed: %s\n", user);
				if(pthread_mutex_unlock(&mLock) != 0){
					perror("Mutex lock error");
				}
			}
		}
		else {  // external command
			if ((pid = fork()) < 0) {
				printf("fork error\n");
			}
			else if (pid == 0) {		/*Right hand side of pipe when applicable child */
			                // an array of aguments for execve()
				char    *execargs[MAXARGS]; 
		    	glob_t  paths;
                int     csource;
				int 	j = 0;
				char    **p;
				struct pathelement *path, *tmp;
				char *cmd;
				int commandIndex = 0;
				if(pipeBool){
					commandIndex = pipeIndex + 1;
					close(STDIN_FILENO);
					dup(pipefd[READ_END]);
					if(arg[pipeIndex][1] == '&'){
						close(STDERR_FILENO);
						dup(pipefd[READ_END]);
					}
					close(pipefd[READ_END]);
					close(pipefd[WRITE_END]);
				}
				// printf("Command in arg array: %s\n",arg[commandIndex]);
				switch(arg[commandIndex][0]){
					case '/': //For full path length
						execargs[j] = malloc(strlen(arg[commandIndex])+1);
						strcpy(execargs[0],arg[commandIndex]);
						break;
					case '.': //For paths that start with dot
						if(arg[commandIndex][1] == '.'){ //for paths that start with ..
							execargs[j] = malloc(strlen(arg[commandIndex])+1);
							arg[commandIndex] += 2;
							cwd = getcwd(NULL,0);
							strcpy(execargs[0],strcat(dirname(cwd),arg[commandIndex]));
							free(cwd);
						}
						else if(arg[commandIndex][1] == '/'){ // For paths that start with only one dot
							execargs[j] = malloc(strlen(arg[commandIndex])+1);
							arg[commandIndex] += 1;
							cwd = getcwd(NULL,0);
							strcpy(execargs[0],strcat(cwd,arg[commandIndex]));
							free(cwd);
						}
						else{
							printf("Incorrect input\n");
							exit(EXIT_FAILURE);
						}
						break;
					default: //for commands that search using which
						path = get_path();
						cmd = which(arg[commandIndex], path);
						while (path) {   // free list of path values
							tmp = path;
							path = path->next;
							free(tmp->element);
							free(tmp);
						}
						// printf("%s\n",cmd);
						// printf("%s\n",execargs[j]);  // copy command
						if (cmd) {
							// printf("%s\n", cmd);
							execargs[j] = malloc(strlen(cmd)+1);
							strcpy(execargs[0], cmd);
							free(cmd);
						}
						else{
							printf("%s: Command not found\n", arg[commandIndex]);// argument not found
							exit(EXIT_SUCCESS);
						}
				}
				j = 1;
				// printf("%s\n",execargs[0]);
				for (i = pipeIndex + 1*pipeBool + 1; i < arg_no; i++){ // check arguments and add to argument array that is sent to the command
					printf("i is %d\n",i);
					if (strchr(arg[i], '*') != NULL) { // wildcard!
						csource = glob(arg[i], 0, NULL, &paths);
						if (csource == 0) {
							for (p = paths.gl_pathv; *p != NULL; ++p) {
								execargs[j] = malloc(strlen(*p)+1);
								strcpy(execargs[j], *p);
								j++;
							}
							globfree(&paths);
						}
					}
					else if(strcmp(arg[i],"&") == 0 || arg[i][0] == '>' || arg[i][0] == '<' ){
						break;
					}
					else if(arg[i][0] == '-'){
						execargs[j] = malloc(strlen(arg[i])+1);
						execargs[j++] = arg[i];
					}
					else if(arg[i]!=NULL){
						execargs[j] = malloc(strlen(arg[i])+1);
						execargs[j++] = arg[i];
					}
				}
				execargs[j] = NULL;
				i = 0;
				// for (i = 0; i < j; i++)
				// 	printf("exec arg [%s]\n", execargs[i]);
				printf("Executing: [%s]\n", execargs[0]);
				if(access(execargs[0],X_OK) != 0){
					perror("Access Error");
					// errno = 0;
					exit(EXIT_FAILURE);
				}
				if(redirect)
					fileRedirect(arg[redirectIndex],arg[redirectIndex+1]);
				execve(execargs[0], execargs, NULL);
				printf("couldn't execute: %s", buf);
				exit(127);
			}
			if(pipeBool){
				if ((pidPipe = fork()) < 0) {
					printf("fork error\n");
				}
			}
			if(pidPipe == 0 && pipeBool && !pipeBuiltIn){		/*Left hand child of pipe */
			                // an array of aguments for execve()
				char    *execargs[MAXARGS]; 
		    	glob_t  paths;
                int     csource;
				int 	j = 0;
				char    **p;
				struct pathelement *path, *tmp;
				char *cmd;
				int commandIndex = 0;
				if(pipeBool){
					close(STDOUT_FILENO);
					dup(pipefd[WRITE_END]);
					if(arg[pipeIndex][1] == '&'){
						close(STDERR_FILENO);
						dup(pipefd[WRITE_END]);
					}
					close(pipefd[READ_END]);
					close(pipefd[WRITE_END]);
				}
				else{
					printf("Something went very wrong\n");
					exit(-1);
				}
				switch(arg[commandIndex][0]){
					case '/': //For full path length
						execargs[j] = malloc(strlen(arg[commandIndex])+1);
						strcpy(execargs[0],arg[commandIndex]);
						break;
					case '.': //For paths that start with dot
						if(arg[commandIndex][1] == '.'){ //for paths that start with ..
							execargs[j] = malloc(strlen(arg[commandIndex])+1);
							arg[commandIndex] += 2;
							cwd = getcwd(NULL,0);
							strcpy(execargs[0],strcat(dirname(cwd),arg[commandIndex]));
						}
						else if(arg[commandIndex][1] == '/'){ // For paths that start with only one dot
							execargs[j] = malloc(strlen(arg[commandIndex])+1);
							arg[commandIndex] += 1;
							cwd = getcwd(NULL,0);
							strcpy(execargs[0],strcat(cwd,arg[commandIndex]));
						}
						else{
							printf("Incorrect input\n");
							exit(EXIT_FAILURE);
						}
						break;
					default: //for commands that search using which
						path = get_path();
						cmd = which(arg[commandIndex], path);
						while (path) {   // free list of path values
							tmp = path;
							path = path->next;
							free(tmp->element);
							free(tmp);
						}
						// printf("%s\n",cmd);
						// printf("%s\n",execargs[j]);  // copy command
						if (cmd) {
							// printf("%s\n", cmd);
							execargs[j] = malloc(strlen(cmd)+1);
							strcpy(execargs[0], cmd);
							free(cmd);
						}
						else{
							printf("%s: Command not found\n", arg[commandIndex]);// argument not found
							exit(EXIT_SUCCESS);
						}
				}
				j = 1;
				// printf("%s\n",execargs[0]);
				for (i = 1; i < pipeIndex; i++){ // check arguments and add to argument array that is sent to the command
					if (strchr(arg[i], '*') != NULL) { // wildcard!
						csource = glob(arg[i], 0, NULL, &paths);
						if (csource == 0) {
							for (p = paths.gl_pathv; *p != NULL; ++p) {
								execargs[j] = malloc(strlen(*p)+1);
								strcpy(execargs[j], *p);
								j++;
							}
							globfree(&paths);
						}
					}
					else if(strcmp(arg[i],"&") == 0 || arg[i][0] == '>' || arg[i][0] == '<' ){
						break;
					}
					else if(arg[i][0] == '-'){
						execargs[j] = malloc(strlen(arg[i])+1);
						execargs[j++] = arg[i];
					}
					else if(arg[i]!=NULL){
						execargs[j] = malloc(strlen(arg[i])+1);
						execargs[j++] = arg[i];
					}
				}
				execargs[j] = NULL;
				i = 0;
				// for (i = 0; i < j; i++)
				// 	printf("exec arg [%s]\n", execargs[i]);
				printf("Executing: [%s]\n", execargs[0]);
				if(access(execargs[0],X_OK) != 0){
					perror("Access Error");
					// errno = 0;
					exit(EXIT_FAILURE);
				}
				if(redirect)
					fileRedirect(arg[redirectIndex],arg[redirectIndex+1]);
				execve(execargs[0], execargs, NULL);
				printf("couldn't execute: %s", buf);
				exit(127);
			}
			/* parent */

			if(pipeBool && (strcmp(arg[0],"pwd") == 0 || strcmp(arg[0],"exit") == 0 ||  \
			strcmp(arg[0],"which") == 0 || strcmp(arg[0],"where") == 0 || \
			strcmp(arg[0],"cd") == 0 || strcmp(arg[0],"prompt") == 0 || \
			strcmp(arg[0],"pid") == 0 || strcmp(arg[0],"list") == 0 || \
			strcmp(arg[0],"kill") == 0 || strcmp(arg[0],"printenv") == 0 || \
			strcmp(arg[0],"setenv") == 0 || strcmp(arg[0],"noclobber") == 0)){
				printf("Pipe with built in on left\n");
				pipeBuiltIn = true;
				close(STDOUT_FILENO);
				dup(pipefd[WRITE_END]);
				if(arg[pipeIndex][1] == '&'){
					close(STDERR_FILENO);
					dup(pipefd[WRITE_END]);
				}
				printf("Pipe with built in on left\n");
				close(pipefd[READ_END]);
				close(pipefd[WRITE_END]);
			}
			else if(pipeBool){
				close(pipefd[WRITE_END]);
				close(pipefd[READ_END]);
			}
		  	//If the last argument is &, don't immediatly wait
			if(strcmp(arg[arg_no - 1],"&") == 0){
				last_pid = pid;
				printf("Child PID: [%d]\n",pid);
			}
			else if(!pipeBool){
				if ((pid = waitpid(pid, &status, 0)) < 0)
					printf("waitpid error");
			}
			else{
				while(wait(NULL) > 0);
			}

			if (WIFEXITED(status) && WEXITSTATUS(status) != 0) //S&R p. 239 
				printf("child terminates with (%d)\n", WEXITSTATUS(status));//Print errors with non 0 exit codes

        }

    nextprompt:
		pid = waitpid(pid,&status,WNOHANG);
		cwd = getcwd(NULL, 0);
		fprintf(stdout, "%s [%s] >> ",prompt,cwd);//Print a new prompt
		fflush(stdout);
		free(cwd);
	}
	if(eofStatus == NULL){ 					// Catches the EOF character from ctrl+D
		printf("\nType exit instead\n");
		clearerr(stdin);					//Clear stdin so the while loop can continue using stdin
		goto nextprompt;
	}
	exit(0);
}

void fileRedirect(char *type,char *file){
	int fd;
	if(noclobber){
		if(strcmp(type,">") == 0){
			if( access(file, F_OK ) == 0 ) {
				// file exists
				printf("refused: noclobber is active and file already exists");
				exit(0);
			} 
			else {
				// file doesn't exist
				fd = open(file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP);
				close(STDOUT_FILENO);
				dup(fd);
				close(fd);
			}
		}
		else if(strcmp(type,">>") == 0){
			if( access(file, F_OK ) == -1 ) {
				// file exists
				printf("refused: noclobber is active and file doesn't exist");
				exit(0);
			} 
			else {
				// file doesn't exist
				fd = open(file,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP);
				close(STDOUT_FILENO);
				dup(fd);
				close(fd);
			}
		}
		else if(strcmp(type,">&") == 0){
			if( access( file, F_OK ) == 0 ) {
				// file exists
				printf("refused: noclobber is active and file already exists");
				exit(0);
			} 
			else {
				// file doesn't exist
				fd = open(file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP);
				close(STDOUT_FILENO);
				dup(fd);
				close(STDERR_FILENO);
				dup(fd);
				close(fd);
			}
		}
		else if(strcmp(type,">>&") == 0){
			if( access(file, F_OK ) == -1 ) {
				// file exists
				printf("refused: noclobber is active and file doesn't exist");
				exit(0);
			} 
			else {
				// file doesn't exist
				fd = open(file,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP);
				close(STDOUT_FILENO);
				dup(fd);
				close(STDERR_FILENO);
				dup(fd);
				close(fd);
			}
		}
		else if(strcmp(type,"<") == 0){
			printf("STDIN redirection\n");
			fd = open(file,O_RDWR,S_IRUSR|S_IWUSR|S_IRGRP);
			close(STDIN_FILENO);
			dup(fd);
			close(fd);
		}
	}
	else{
		if(strcmp(type,">") == 0){
			fd = open(file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP);
			close(STDOUT_FILENO);
			dup(fd);
			close(fd);
		}
		else if(strcmp(type,">>") == 0){
			fd = open(file,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP);
			close(STDOUT_FILENO);
			dup(fd);
			close(fd);
		}
		else if(strcmp(type,">&") == 0){
			fd = open(file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP);
			close(STDOUT_FILENO);
			dup(fd);
			close(STDERR_FILENO);
			dup(fd);
			close(fd);
		}
		else if(strcmp(type,">>&") == 0){
			fd = open(file,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP);
			close(STDOUT_FILENO);
			dup(fd);
			close(STDERR_FILENO);
			dup(fd);
			close(fd);
		}
		else if(strcmp(type,"<") == 0){
			printf("STDIN redirection\n");
			fd = open(file,O_RDWR,S_IRUSR|S_IWUSR|S_IRGRP);
			close(STDIN_FILENO);
			dup(fd);
			close(fd);
		}
	}
}
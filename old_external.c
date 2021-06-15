				// if(arg[0][0] == '/'){
				// 	execargs[j] = malloc(strlen(arg[0])+1);
				// 	strcpy(execargs[0],arg[0]);
				// 	//printf("%s\n",execargs[0]);  // copy command
				// 	j = 1;
				// 	for (i = 1; i < arg_no; i++){ // check arguments  
				// 		if (strchr(arg[i], '*') != NULL) { // wildcard!
				// 			csource = glob(arg[i], 0, NULL, &paths);
				// 			if (csource == 0) {
				// 				for (p = paths.gl_pathv; *p != NULL; ++p) {
				// 					execargs[j] = malloc(strlen(*p)+1);
				// 					strcpy(execargs[j], *p);
				// 					j++;
				// 				}
				// 				globfree(&paths);
				// 			}
				// 		}
				// 		else if(arg[i][0] == '-'){
				// 			execargs[j] = malloc(strlen(arg[i])+1);
				// 			execargs[j++] = arg[i];
				// 		}
				// 		else if(arg[i]!=NULL){
				// 			execargs[j] = malloc(strlen(arg[i])+1);
				// 			execargs[j++] = arg[i];
				// 		}
				// 	}
				// 	execargs[j] = NULL;
				// 	i = 0;
				// 	// for (i = 0; i < j; i++)
				// 	// 	printf("exec arg [%s]\n", execargs[i]);
				// 	printf("Executing external [%s]\n", execargs[i]);
				// 	if(access(execargs[0],X_OK) != 0) exit(5);
				// 	execve(execargs[0], execargs, NULL);
				// 	printf("couldn't execute: %s", buf);
				// 	exit(127);				
				// }
				// else if(arg[0][0] == '.'){

				// }
				// else{
				// 	path = get_path();
				// 	cmd = which(arg[0], path);
				// 	// printf("%s\n",cmd);
				// 	execargs[j] = malloc(strlen(arg[0])+1);
					
				// 	// printf("%s\n",execargs[j]);  // copy command
				// 	if (cmd) {
				// 		// printf("%s\n", cmd);
				// 		strcpy(execargs[0], cmd);
				// 		free(cmd);
				// 	}
				// 	else{
				// 		printf("%s: Command not found\n", arg[0]);// argument not found
				// 	}
				// 	j = 1;
				// 	for (i = 1; i < arg_no; i++){ // check arguments  
				// 		if (strchr(arg[i], '*') != NULL) { // wildcard!
				// 			csource = glob(arg[i], 0, NULL, &paths);
				// 			if (csource == 0) {
				// 				for (p = paths.gl_pathv; *p != NULL; ++p) {
				// 					execargs[j] = malloc(strlen(*p)+1);
				// 					strcpy(execargs[j], *p);
				// 					j++;
				// 				}
							
				// 				globfree(&paths);
				// 			}
				// 		}
				// 		else if(arg[i][0] == '-'){
				// 			execargs[j] = malloc(strlen(arg[i])+1);
				// 			execargs[j++] = arg[i];
				// 		}
				// 		else if(arg[i]!=NULL){
				// 			execargs[j] = malloc(strlen(arg[i])+1);
				// 			execargs[j++] = arg[i];
				// 		}
				// 	}
				// 	execargs[j] = NULL;
				// 	i = 0;
				// 	// for (i = 0; i < j; i++)
				// 	// 	printf("exec arg [%s]\n", execargs[i]);
				// 	printf("Executing: [%s]\n", execargs[i]);
				// 	while (path) {   // free list of path values
				// 		tmp = path;
				// 		path = path->next;
				// 		free(tmp->element);
				// 		free(tmp);
				// 	}
				// 	// access(execargs[0],X_OK);
				// 	execve(execargs[0], execargs, NULL);
				// 	printf("couldn't execute: %s", buf);
				// 	exit(127);
				// }
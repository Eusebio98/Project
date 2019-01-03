/* Belfiore Eusebio matricola: 046001782 */

/* SERVER */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>

// definition node list
typedef struct struct_list {
    char *file_path;
    struct struct_list *next;
    int is_directory; // flag if 1 file_path indicates a directory, if 0 file_path indicates a file
} ListNode;

typedef ListNode* List; // definition list

// declaration of functions
void MakeNullList(List *lis);
void InserTailList(List *lis, char *elem, int flag_dir);
void DeleteItemList(char *elem);
void PrintList(List lis);
void ls_directory(List *lis, char *arg);
int number_of_core();
void *thread_function(void *arg);
void AddList(List lis, List *main_list);
List FindDir();
void listpath(char *path);
void *connection_thread(void *arg);
void sendList(List lis, int sock, char *str);
void download(char *str, int sock);

// declaration of global variables
List list;
pthread_mutex_t work_mutex; // this mutex protects the global list 
pthread_mutex_t mutex_check; // this mutex protects the integer vector exit_check 
int *exit_check; // pointer to the first element of a vector of integers

int main(void) {    

    int servsock; // connection socket
    int clisock; // comunication socket
    struct sockaddr_in baddr;
    pthread_t wt; // id worker thread

    servsock = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM --> TCP, AF_INTET --> internet domain
    if(servsock == -1) {
	perror("Connection error in socket()\n");
	exit(EXIT_FAILURE);
    }

    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(7777);
    baddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // binding of socket
    if(bind(servsock, (struct sockaddr *)&baddr, sizeof(baddr)) == -1) {
	perror("Binding socket error\n");
	exit(EXIT_FAILURE);
    }

    // backlog socket = 20
    if(listen(servsock, 20) == -1) {
	perror("Error in function listen()\n");
	exit(EXIT_FAILURE);
    }

    listpath("/home/eusebio/Scrivania"); // listing of /home 
    
    // now the global list contains all indexed files available from possible clients

    while(1) {

	// new connection
	clisock = accept(servsock, NULL, NULL); 
	if (clisock == -1) {
	    perror("Error in function accept()\n");
	    continue;
	}

	printf("New connection\n");

	// creation of a thread for each connection socket
	if(pthread_create(&wt, NULL, connection_thread, (void *)&clisock) != 0) { 
	    perror("Thread creation failed\n");
	    close(clisock); 
	}

    } 

}

void *connection_thread(void *arg) {

    int sock = (*(int *)arg); // comunication socket id
    int len = 0;
    char username[33], password[33];
    char str1[1000], str2[33]; // support variables
    int user_found = 0; 

    // server receives username and password from client

    strcpy(username, "\nInsert username: ");
    write(sock, (void *)username, strlen(username));
    len = read(sock, (void *)username, 32);
    if(len > 0)
	username[len-1]='\0';

    strcpy(password, "Insert password: ");
    write(sock, (void *)password, strlen(password));
    len = read(sock, (void *)password, 32);
    if(len > 0)
	password[len-1]='\0';

    FILE *f = NULL;
    f = fopen("/home/user_pass.txt", "rb"); // open the file that contains the list of authorized usernames and their passwords

    if(f == NULL) {
        printf("Impossible to open /home/user_pass.txt\n"); // maybe the file doesn't exist!!
        printf("Client disconnected\n");
	strcpy(str1, "Impossible to open /home/user_pass.txt\n");	 
	write(sock, (void *)str1, strlen(str1));
        close(sock);
        pthread_exit(NULL);
    }

    while(fscanf(f, "%s%s", str1, str2) == 2) {
	if(strncmp(str1, username, strlen(str1)) == 0 && strncmp(str2, password, strlen(str2)) == 0 && 
           strlen(str1) == strlen(username) && strlen(str2) == strlen(password) ) {
	    user_found=1; // username and password found
	    break;
        }
    }

    fclose(f);
 
    // username and password not found
    if(user_found == 0) {
	strcpy(str1, "You don't have right permission... Closing connection\n");	 
	write(sock, (void *)str1, strlen(str1));
        printf("Client disconnected\n");
	close(sock);
	pthread_exit(NULL);
    }

    // user logged in correctly
    sprintf(str1, "\nWelcome %s\n", username);
    write(sock, (void *)str1, strlen(str1));

    // login response error 
    len = read(sock, (void *)str1, 99);
    if(len > 0)
        str1[len]='\0';
    if(len == 4 && strncmp(str1, "ok", 2) != 0) {
	printf("Client disconnected\n");
	close(sock);
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&work_mutex);
    sendList(list, sock, "/home/eusebio/Scrivania"); // send the complete list to the client
    pthread_mutex_unlock(&work_mutex);

    while(1) {

        len = read(sock, (void *)str1, 999);
	if(len > 0)
	    str1[len-1]='\0';

	// send to the client the list of paths containing the keyword sent by client 
	if(len > 8 && strncmp(str1, "search ", 7) == 0) {		
	    strcpy(str1, &str1[7]);
	    pthread_mutex_lock(&work_mutex);
	    sendList(list, sock, str1);
	    pthread_mutex_unlock(&work_mutex);
	}

	// download file relative to the path sent by the client
	else if(len > 10 && strncmp(str1, "download ", 9) == 0) {		
	    strcpy(str1, &str1[9]);	
	    download(str1, sock); 
	}

	// exit from while
	else if(len == 5 && strncmp(str1, "exit", 4) == 0) 
	    break;

	// invalid command 
        else {
	    sprintf(str1, "Invalid command\n");
	    write(sock, (void *)str1, strlen(str1));
	}

    }      

    printf("Client disconnected\n");
    close(sock); // closing of comunication socket   

}

// function that sends the file passed as argument to the client as a stream of characters
void download(char *str, int sock) {

    char str_ok[16];
    char buffer[512]; 
    int n = 1;
    int f; // file descriptor
    int len = 0;

    f = open(str, O_RDONLY); 

    // error in file opening --> sending 0 to indicate download error
    if(f == -1) {
	sprintf(str, "%d", 0);
	write(sock, (void *)str, strlen(str));
	// ok from client
        len = read(sock, (void *)str_ok, 15);
	return;
    }

    while (1) {

    	n = read(f, &buffer, 512);

	if(n>0) {
	    write(sock, (void *)buffer, n); // read from file
	    // ok from client
	    len = read(sock, (void *)str_ok, 15);
	} 

	else {
	    // sending escape sequence to indicate to the client that the sending is finished
            sprintf(str_ok, "escape_1234");
            write(sock, (void *)str_ok, strlen(str_ok));
            // ok from client
            len = read(sock, (void *)str_ok, 15);
	    break;
	}
    }

    close(f);

}

// function that sends a list of paths to the client ... the paths containing the string passed as argument
void sendList(List lis, int sock, char *str) {

    int count = 0;
    int i = 0;
    char str_ok[16];
    int len = 0;

    while(lis != NULL) {
        for(i=0; i<strlen(lis->file_path); i++) 
    	    if(strncmp(&(lis->file_path)[i], str, strlen(str)) == 0) {
		count++;
                write(sock, (void *)lis->file_path, strlen(lis->file_path)); // send file pathname to client
		// ok from client
		len = read(sock, (void *)str_ok, 15);
		break;
	    }
	lis=lis->next;
    }

    // sending 0 to indicate no file found
    if(count == 0) {
	sprintf(str, "%d", count);
	write(sock, (void *)str, strlen(str));
	// ok from client
        len = read(sock, (void *)str_ok, 15);    
    }
	
    else {
        // sending escape sequence to indicate to the client that the sending is finished
        sprintf(str_ok, "escape_1234");
        write(sock, (void *)str_ok, strlen(str_ok));
        // ok from client
        len = read(sock, (void *)str_ok, 15);
    }   

}

// function that lists a path passed as argument
void listpath(char *path) {

    const int n_core = number_of_core();
    pthread_t thread_id[n_core];
    exit_check=(int *)malloc(n_core*sizeof(int)); // vector of integers containing the flags related to the exit condition of threads
    int i=0;

    // printf("\n--> Number of CPU cores is %d\n", n_core);

    MakeNullList(&list);
    ls_directory(&list, path); // listing of directory passed as argument

    // initialization of the mutex
    if (pthread_mutex_init(&work_mutex, NULL) != 0) {
	printf("Mutex work_mutex initialization failed\n"); 
	exit(EXIT_FAILURE);
    } 
    if (pthread_mutex_init(&mutex_check, NULL) != 0) {
	printf("Mutex mutex_check initialization failed\n"); 
	exit(EXIT_FAILURE);
    }

    // initialization of the vector of the exit variables
    for(i=0; i<n_core; i++)
        exit_check[i]=0;

    // creation of as many threads as the cores of the CPU
    for(i=0; i<n_core; i++) {
	if(pthread_create(&thread_id[i], NULL, thread_function, (void *)(intptr_t)i) != 0) { 
	    printf("Thread creation failed\n");
	    exit(EXIT_FAILURE); 
	}	
    }

    // main thread waits for threads before exiting
    for(i=0; i<n_core; i++) {
	if(pthread_join(thread_id[i],NULL) != 0) {
	    printf("Thread join failed\n");
	    exit(EXIT_FAILURE);
	}
    }

    // destruction of mutexes
    pthread_mutex_destroy(&work_mutex);
    pthread_mutex_destroy(&mutex_check);

}


// thread handeler that looks for a directory in the list, deletes it and calls ls_directory funcion
void *thread_function(void *arg) {

    int exit=0; // exit variable 
    int sum=0;
    const int n_core = number_of_core();
    List lis; // list will contain the result of the search of directory in the global list
    List subdir; // subdir will contain the result of ls_directory of found directory     

    while(!exit) {

	pthread_mutex_lock(&work_mutex);

	lis=FindDir(); // find first directory in the list

	if(lis!=NULL) {

	    DeleteItemList(lis->file_path); // delete the directory from the list without releasing lis memory
	    pthread_mutex_unlock(&work_mutex);

	    // enter the value 0 in exit_check[] so that the other threads do not exit and can check the global list again
	    pthread_mutex_lock(&mutex_check);
	    exit_check[(intptr_t)arg]=0;
            pthread_mutex_unlock(&mutex_check);

	    // create local list of file and subdirectory
	    subdir=NULL;
	    ls_directory(&subdir, lis->file_path);
	    free(lis); // now I can release lis memory

	    pthread_mutex_lock(&work_mutex);
            AddList(subdir, &list); // concatenate global list with local list
            pthread_mutex_unlock(&work_mutex);

	  
    	} else {

	    pthread_mutex_lock(&mutex_check); // lock on mutex_check
	    exit_check[(intptr_t)arg]=1; // set exit_check related to thread to 1 
	    
	    sum=0;

	    // check if the other threads have finished their execution
	    for(int i=0; i<n_core; i++)
	        if(exit_check[i] == 1) 
		    sum++;

	    if(sum == n_core) 
		exit=1; // leave the loop
	    else 
		exit=0; // not leave the loop

	    pthread_mutex_unlock(&mutex_check);
	    pthread_mutex_unlock(&work_mutex);
	}
	
    }

    pthread_exit(NULL);

}

// function that concatenates a list passed as a parameter to the global list
void AddList(List lis, List *main_list) {

    if((*main_list) == NULL) {
	*main_list=lis;
	return;
     }

    if((*main_list)->next == NULL) {
	(*main_list)->next=lis;
	return;
    } else
	AddList(lis,&(*main_list)->next);

}

// function that finds the first directory in the list
List FindDir() {

    List lis=list;
    while (lis != NULL) {
	if(lis->is_directory==1) 	
		return lis;
        lis=lis->next;
    }
    return NULL;  
  
}

// function that initializes a list
void MakeNullList(List *lis) {
    *lis = NULL;
}

// function that queues a string in a list
void InserTailList(List *lis, char *elem, int flag_dir) { 

    List paux, last; 
    paux = (List)malloc(strlen(elem)*sizeof(char)+sizeof(int)+sizeof(List)); // allocation of memory to contain list element 
    if (paux==NULL) 
        exit(EXIT_FAILURE); 
    paux->file_path = elem; 
    paux->next = NULL; 
    paux->is_directory=flag_dir;
    if (*lis == NULL)
        *lis = paux;
    else { 
        last = (*lis); 
        while (last->next != NULL)  
            last = last->next; 
        last->next = paux; 
    }

} 

// function that given a path deletes the element from the list without freeing memory
void DeleteItemList(char *elem) {

    List prec= NULL;
    List corr= NULL;
    int trovato;

    if((list->next)==NULL)
	list=NULL;

    if(list!=NULL) {
	if(strcmp(list->file_path,elem)==0) {
	    prec=list;
	    list=list->next;
	    //free(prec);
	} else {
	    prec=list;
	    corr=prec->next;
	    trovato=0;
	    while(corr!=NULL && !trovato) {
	        if(strcmp(corr->file_path,elem)==0) {
		    trovato=1;
		    prec->next=corr->next;
		    //free(corr);
	        } else {
		    prec=prec->next;
		    corr=prec->next;
	        }
	    }
        }
    }

}

// function that prints a list of strings
void PrintList(List lis) {
    
    printf("\n");
    if(lis==NULL) {
	printf("No file found\n\n");
	return;
    }
    while (lis != NULL) {
        printf("FILE -> %s\n", lis->file_path);
        lis = lis->next;
    }
    printf("\n");
    
}

// function that inserts the files contained in a directory into a list
void ls_directory(List *lis, char *arg) {
    
    char *str;
    
    struct dirent *de;  // Pointer for directory entry
    DIR *dr = opendir(arg); // opendir() returns a pointer of DIR type.
    
    // opendir returns NULL if couldn't open directory
    if (dr == NULL) {
        printf("\nCould not open the following directory --> %s\n", arg);
        exit(EXIT_FAILURE);
    }
    
    // for readdir()
    while ((de = readdir(dr)) != NULL) {
        
        // don't add the current and the upper directory to the list 
        if(strcmp(de->d_name,".")!=0 && strcmp(de->d_name,"..")!=0) {
	    str=NULL;
            // concatenate the path of the folder passed as an argument with the files inside it
	    str = (char *)malloc((strlen(arg)+strlen(de->d_name)+2)*sizeof(char));
	    strcpy(str,"");
	    strcat(str, arg);
	    // if argument was root directory "/" I don't have to add slash
	    if(strcmp(str,"/")!=0)
	    	strcat(str, "/");
	    strcat(str, de->d_name);

	    // don't add the following directory ---> "/proc" and "/run" because of problems of run time listing 
	    if(strcmp(str,"/proc")!=0 && strcmp(str,"/run")!=0) {
		
	        if(de->d_type == DT_DIR)
	            InserTailList(lis, str, 1); // set the flag to know it's a directory
	        else 
	            InserTailList(lis, str, 0); // it's not a directory
	
	    }

 	    str=NULL;
	    free(str);
         }
    }
    
    closedir(dr);
    
}

// function that returns the number of CPU cores
int number_of_core() {
    
    char str[32]; // string that allows to search for the "cpu cores" field
    int numbers_core=0; // number of core
    
    FILE *f=NULL;
    f=fopen("/proc/cpuinfo", "rb");
    
    if(f==NULL) {
        printf("\nThe /proc/cpuinfo file was not opened correctly\n");
        exit(EXIT_FAILURE);
    }
    
    while(fscanf(f, "%s", str)==1) {
        // searching for the string "cpu"
        if(strcmp(str,"cpu")==0){
            fscanf(f, "%s", str);
            // searching for the string "cores
            if(strcmp(str,"cores")==0) {
                fscanf(f, "%s", str); // string that contains ":"
                fscanf(f, "%d", &numbers_core); //now numbers_core contains the number of the core
                fclose(f);
                return numbers_core;
            }
        }
    }
    
    return 0;
    
}


/* Belfiore Eusebio matricola: 046001782 */

/* This program realizes what is required in the delivery of the SO project. The algorithm requires an initial path to be passed as an argument to the program. The main thread lists the initial path keeping the result in a global list and then creates a number of threads equal to the number of cores present in the CPU. The threads access the list through a mutually exclusive access (with mutex) and take charge of the first directory they find, eliminating it from the global list, listing it and adding the result to the global list. Empty directories are simply deleted as their listing will be empty. The threads before leaving their loop check that the other threads have finished their execution. The mechanism is implemented through an array of integers, whose size is equal to the number of CPU cores. Each thread arrived at the end of the list sets the element relative to its index of this array equal to 1 and checks if all the other elements of this array are equal to 1. If it were so it means that there are no more directories in the list and all threads can terminate. Otherwise the thread returns to the loop to check if another thread has added additional directories to the global list. */

/* Some directories to be listed may require root permissions, so it's recommended to run the program only as root. */

/* IMPORTANT ON ROOT LISTING!! --> the following directorties --> "/proc" and "/run" cause run-time problems in listing of subdirectories and files, so I exclude them from the listing of root.. */

/* During compilation, the program returns the following warning: 

warning: assignment makes integer from pointer without a cast [-Wint-conversion]
argv[1][strlen(argv[1])-1]=NULL;

it can be safely ignored. */

/* The path passed as argument can terminate indifferently with or without the '/' character. */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

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
void PrintList();
void ls_directory(List *lis, char *arg);
int number_of_core();
void *thread_function(void *arg);
void AddList(List lis, List *main_list);
List FindDir();

// declaration of global variables
List list;
pthread_mutex_t work_mutex; // this mutex protects the global list 
pthread_mutex_t mutex_check; // this mutex protects the integer vector exit_check 
int *exit_check; // pointer to the first element of a vector of integers


int main(int argc, char **argv) {
    
    const int n_core = number_of_core();
    pthread_t thread_id[n_core];
    exit_check=(int *)malloc(n_core*sizeof(int)); // vector of integers containing the flags related to the exit condition of threads
    int i=0;

    printf("\n--> Number of CPU cores is %d\n", n_core);
	
    // if the last character of the argument is '/' I remove it
    if(strlen(argv[1])!=1 && argv[1][strlen(argv[1])-1] == '/') 
	argv[1][strlen(argv[1])-1]=NULL;

    MakeNullList(&list);
    ls_directory(&list, argv[1]); // listing of directory passed as argument

    printf("\n--> Listing of directory and subdirectory of %s\n", argv[1]);

    // initialization of the mutex
    if (pthread_mutex_init(&work_mutex, NULL) != 0) {
	printf("Mutex initialization failed\n"); 
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

    printf("\n--> Scanning of dir and subdir in progress...\n");

    // main thread waits for threads before exiting
    for(i=0; i<n_core; i++) {
	if(pthread_join(thread_id[i],NULL) != 0) {
	    printf("Thread join failed\n");
	    exit(EXIT_FAILURE);
	}
    }

    PrintList(list);    

    // destruction of mutexes
    pthread_mutex_destroy(&work_mutex);
    pthread_mutex_destroy(&mutex_check);
    
    exit(EXIT_SUCCESS);
    
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
void PrintList() {
    
    List lis=list;
    printf("\n");
    while (lis != NULL) {
        printf("FILE -> %s %d\n", lis->file_path, lis->is_directory);
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


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
    int is_directory;
} ListNode;

typedef ListNode* List; // definition list

List list;
int current_number_of_thread=0;
pthread_mutex_t work_mutex; // the mutex protects the list and the variable current_number_of_thread

// declaration of functions
void MakeNullList();
void InserTailList(char *elem, int flag_dir);
void DeleteItemList(char *elem);
void PrintList();
void ls_directory(char *arg);
int number_of_core();
void *thread_function(void *arg);
void main_thread(List *lis, int n_core);

int main(int argc, char **argv) {
    
    const int n_core = number_of_core();

    // initialization of the mutex
    if (pthread_mutex_init(&work_mutex, NULL) != 0) {
	printf("Mutex initialization failed\n"); 
	exit(EXIT_FAILURE);
    }    

    MakeNullList();
    ls_directory(argv[1]); // listing of working directory 

    main_thread(&list, n_core); // recursive listing
    
    printf("\nNumber of CPU cores is %d\n", n_core);
    
    exit(EXIT_SUCCESS);
    
}

// main thread recursively searches the directories and creates the relative threads
void main_thread(List *lis, int n_core) {

   pthread_t thread_id;

    if (*lis != NULL) {

	// check if the member of list is a directory
        if((*lis)->is_directory==1) {

	    if(current_number_of_thread<n_core) {
	        current_number_of_thread++;
		printf("Create thread %d / %d", current_number_of_thread, n_core);
		if (pthread_create(&thread_id, NULL, thread_function,(void *)((*lis)->file_path)) != 0) { 
		    printf("Thread creation failed\n");
		    exit(EXIT_FAILURE); 
		}
		sleep(3);
	    }
	}

	main_thread(&(*lis)->next, n_core);
    } 

}

// thread handeler that deletes the directory from the list and calls ls_directory funcion
void *thread_function(void *arg) {
     
    //pthread_mutex_lock(&work_mutex);
    DeleteItemList((char *)arg);
    ls_directory((char *)arg);
    PrintList();
    current_number_of_thread--;
    //pthread_mutex_unlock(&work_mutex);

}

// function that initializes a list
void MakeNullList() {
    list = NULL;
}

// function that queues a string in a list
void InserTailList(char *elem, int flag_dir) { 

    List paux, last; 
    paux = (List)malloc(sizeof((strlen(elem)+1)*sizeof(char))); 
    if (paux==NULL) 
        exit(EXIT_FAILURE); 
    paux->file_path = elem; 
    paux->next = NULL; 
    paux->is_directory=flag_dir;
    if (list == NULL)
        list = paux;
    else { 
        last = list; 
        while (last->next != NULL)  
            last = last->next; 
        last->next = paux; 
    }

} 

// function that given a path deletes the element from the list
void DeleteItemList(char *elem) {

    List prec;
    List corr;
    int trovato;

    if(list!=NULL) {
	if(strcmp(list->file_path,elem)==0) {
	    prec=list;
	    list=list->next;
	    free(prec);
	} else {
	    prec=list;
	    corr=prec->next;
	    trovato=0;
	    while(corr!=NULL && !trovato) {
	        if(strcmp(corr->file_path,elem)==0) {
		    trovato=1;
		    prec->next=corr->next;
		    free(corr);
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
        printf("%s %d\n", lis->file_path, lis->is_directory);
        lis = lis->next;
    }
    printf("\n");
    
}

// function that inserts the files contained in a directory into a list
void ls_directory(char *arg) {
    
    char *str;
    
    struct dirent *de;  // Pointer for directory entry
    DIR *dr = opendir(arg); // opendir() returns a pointer of DIR type.
    
    // opendir returns NULL if couldn't open directory
    if (dr == NULL) {
        printf("Could not open current directory\n");
        exit(EXIT_FAILURE);
    }
    
    // for readdir()
    while ((de = readdir(dr)) != NULL) {
        
        // don't add the current and the upper directory to the list
        if(strcmp(de->d_name,".")!=0 && strcmp(de->d_name,"..")!=0) {
            // concatenate the path of the folder passed as an argument with the files inside it
	    str = (char *)malloc((strlen(arg)+strlen(de->d_name)+2)*sizeof(char));
	    strcat(str, arg);
	    strcat(str, "/");
	    strcat(str, de->d_name);
		
	    if(de->d_type == DT_DIR)
	        InserTailList(str,1); // set the flag to know it's a directory
	    else 
	        InserTailList(str,0); // it's not a directory
	
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

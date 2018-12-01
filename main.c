
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


//definition node list
typedef struct struct_list {
    char *file_path;
    struct struct_list *next;
    int is_directory;
} ListNode;

typedef ListNode* List; //definition list

List list;

//declaration of functions
void MakeNullList();
void InserTailList(char *elem, int flag_dir);
void PrintList();
void ls_directory(char *arg);
void ls_recursive();
int number_of_core();


int main(int argc, char **argv) {
    
    const int n_core = number_of_core();
    //struct stat *buffer;
    
    MakeNullList();
    ls_directory(argv[1]);
    ls_recursive();	

    PrintList();
    
    printf("\nIl numero di core della CPU e' %d\n", n_core);
    
    exit(EXIT_SUCCESS);
    
}

// function that list recursive file in directory
void ls_recursive() {
	
    List lis=list;

    while (lis != NULL) {
	// check if the member of list is a directory
        if(lis->is_directory==1) {
	    ls_directory(lis->file_path); // add files and directory inside it in the list			
	}
        lis = lis->next;
   }

}

//function that initializes a list
void MakeNullList() {
    list = NULL;
}

//function that queues a string in a list
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


//function that prints a list of strings
void PrintList() {
    
    List lis=list;
    printf("\n");
    while (lis != NULL) {
        printf("%s %d\n", lis->file_path, lis->is_directory);
        lis = lis->next;
    }
    printf("\n");
    
}

//function that inserts the files contained in a directory into a list
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
            // searching for the string "cores"
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


#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

//definition node list
typedef struct struct_list {
    char *file_path;
    struct struct_list *next;
} ListNode;

typedef ListNode* List; //definition list

List list;

//declaration of functions
void MakeNullList();
void InserTailList(char *elem);
void VisitList();
void ls_directory(char *arg);
int number_of_core();

int main(int argc, char **argv) {
    
    const int n_core = number_of_core();
    
    MakeNullList();
    ls_directory(argv[1]);
    VisitList();
    
    printf("\nIl numero di core della CPU e' %d\n", n_core);
    
    exit(EXIT_SUCCESS);
    
}


//function that initializes a list
void MakeNullList() {
    list = NULL;
}

//function that queues a string in a list
void InserTailList(char *elem) { 

    List paux, last; 
    paux = (List)malloc(sizeof((strlen(elem)+1)*sizeof(char))); 
    if (paux==NULL) 
        exit(EXIT_FAILURE); 
    paux->file_path = elem; 
    paux->next = NULL; 
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
void VisitList() {
    
    List lis=list;
    printf("\n");
    while (lis != NULL) {
        printf("%s\n", lis->file_path);
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
        
        // concatenate the path of the folder passed as an argument with the files inside it
        str = (char *)malloc((strlen(arg)+strlen(de->d_name)+2)*sizeof(char));
        strcat(str, arg);
        strcat(str, "/");
        strcat(str, de->d_name);
	InserTailList(str);
        str=NULL;
        free(str);
        
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

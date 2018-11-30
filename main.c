
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

//definition node list
typedef struct struct_list {
    char *file_path;
    struct struct_list *next;
} ListNode;

typedef ListNode* List; //definition list

List list;

//declaration of functions
void MakeNullList (List *lis);
void InserTailList(List *lis, char *elem);
void VisitList(List lis);
void ls_directory(List *lis, char *arg);
void number_of_core();

int main(int argc, char **argv) {
    
    int const n_max_thread = number_of_core();
    
    MakeNullList(&list);
    ls_directory(&list,argv[1]);
    VisitList(list);
    
    number_of_core();

    exit(EXIT_SUCCESS);
    
}


//function that initializes a list
void MakeNullList (List *lis) {
    *lis = NULL;
}

//function that queues a string in a list
void InserTailList(List *lis, char *elem){
    
    if (*lis == NULL) {
        *lis = (List)malloc((strlen(elem)+1)*sizeof(char));
        (*lis)->file_path = elem;
        (*lis)->next = NULL;
    } else
        InserTailList(&(*lis)->next, elem);
    
}

//function that prints a list of strings
void VisitList(List lis) {
    
    printf("\n");
    while (lis != NULL) {
        printf("%s\n", lis->file_path);
        lis = lis->next;
    }
    printf("\n");
    
}

//function that inserts the files contained in a directory into a list
void ls_directory(List *lis, char *arg) {
    
    struct dirent *de;  // Pointer for directory entry
    DIR *dr = opendir(arg); // opendir() returns a pointer of DIR type.
    
    // opendir returns NULL if couldn't open directory
    if (dr == NULL) {
        printf("Could not open current directory" );
        exit(EXIT_FAILURE);
    }
    
    // for readdir()
    while ((de = readdir(dr)) != NULL)
        InserTailList(lis, de->d_name);
    
    closedir(dr);
    
}

void number_of_core() {

   FILE *cpuinfo = fopen("/proc/cpuinfo", "rb");
   char *arg = 0;
   size_t size = 0;
    
   while(getdelim(&arg, &size, 0, cpuinfo) != -1)
      puts(arg);
 
   free(arg);
   fclose(cpuinfo);
    
}

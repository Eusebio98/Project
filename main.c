
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

//definizione nodo lista
typedef struct struct_list {
    char *file_path;
    struct struct_list *next;
} ListNode;

typedef ListNode* List; //definizione lista

List list;

//dichiarazione di funzioni
void MakeNullList (List *lis);
void InserisciCodaLista(List *lis, char *elem);
void VisitaLista(List lis);
void ls_directory(List *lis, char *arg);


int main(int argc, char **argv) {
    
    MakeNullList(&list);
    ls_directory(&list,argv[1]);
    VisitaLista(list);

    exit(EXIT_SUCCESS);
    
}


//funzione che inizializza una lista
void MakeNullList (List *lis) {
    *lis = NULL;
}

//funzione che inserisce una stringa in una lista
void InserisciCodaLista(List *lis, char *elem){
    
    if (*lis == NULL) {
        *lis = (List)malloc((strlen(elem)+1)*sizeof(char));
        (*lis)->file_path = elem;
        (*lis)->next = NULL;
    } else
        InserisciCodaLista(&(*lis)->next, elem);
    
}

//funzione che stampa una lista di stringhe
void VisitaLista(List lis) {
    
    printf("\n");
    while (lis != NULL) {
        printf("%s\n", lis->file_path);
        lis = lis->next;
    }
    printf("\n");
    
}

//funzione che inserisce in una lista i file contenuti in una directory
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
        InserisciCodaLista(lis, de->d_name);
    
    closedir(dr);
    
}

/* Belfiore Eusebio matricola: 046001782 */

/* CLIENT */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>

void print_file(int clisock);

int main(void) {

    int clisock; // socket id
    struct sockaddr_in baddr;
    char buffer[1000];
    int len;
    char file_name[256]; // file name
    FILE *f = NULL; // file descriptor 
    char user[33];
    int integer = 0;
    int i;
    int file; // file descriptor
    int n;
	
    clisock = socket(AF_INET, SOCK_STREAM, 0);
    if(clisock == -1) {
	perror("Error in function socket()\n");
	exit(EXIT_FAILURE);
    }
	
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(7777);
    baddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
    // connect to the server
    if(connect(clisock, (struct sockaddr *)&baddr, sizeof(baddr)) == -1) {
	perror("Error in function connect()\n");
	exit(EXIT_FAILURE);
    }

    len = read(clisock, (void *)buffer, 199);
    if(len > 0)
	buffer[len] = '\0';
    printf("%s", buffer);

    // user interts username --> check if there is a ' ' in string --> if so user must insert another time username
    while(1) {
	integer = 0; // check variable
        fgets(buffer, 199, stdin); // insert username 
	for(i=0; i<strlen(buffer); i++) 
	    if(strncmp(&(buffer)[i], " ", 1) == 0) {
		printf("Error --> You cannot insert space\nInsert username: ");
		integer = 1;	
	        break;
	    }
	if(integer == 0)
	    break; // leave while
    }

    write(clisock, (void *)buffer, strlen(buffer)); // send username
    strcpy(user, buffer);

    len = read(clisock, (void *)buffer, 199);
    if(len > 0)
        buffer[len] = '\0';
    printf("%s", buffer);

    // user interts password --> check if there is a ' ' in string --> if so user must insert another time password
    while(1) {
	integer = 0; // check variable
        fgets(buffer, 199, stdin); // insert password 
	for(i=0; i<strlen(buffer); i++) 
	    if(strncmp(&(buffer)[i], " ", 1) == 0) {
		printf("Error --> You cannot insert space\nInsert password: ");
		integer = 1;	
	        break;
	    }
	if(integer == 0)
		break; // leave while
    }

    write(clisock, (void *)buffer, strlen(buffer)); // send password

    // response from server
    len = read(clisock, (void *)buffer, 199);
    if(len > 0)
        buffer[len] = '\0';

    printf("%s", buffer);

    // file /home/user_pass.txt error in server
    if(strncmp("Impossible to open /home/user_pass.txt", buffer, 38) == 0) { 
        close(clisock);
	exit(EXIT_FAILURE);
    }

    // user or password wrong
    else if(strncmp("You don't have right permission...", buffer, 34) == 0) {

	while(1) {
	    printf("\nDo you want to sign in? [Y/N] --> ");
	    scanf("%s%*c", buffer);
	    if( strlen(buffer) == 1 && (strncmp(buffer, "Y", 1) == 0 || strncmp(buffer, "N", 1) == 0) )
		break;
	    printf("Command error\n");
	}
	
    	write(clisock, (void *)buffer, strlen(buffer)); // send option	

	// user wants to sign in
	if(strlen(buffer) == 1 && strncmp(buffer, "Y", 1) == 0) {

	    len = read(clisock, (void *)buffer, 199);
    	    if(len > 0)
                buffer[len] = '\0';

	    if(strncmp("User found, you cannot sign in...", buffer, 33) == 0) { 
	        // user found, impossible to sign in
		printf("%s", buffer);
		printf("\nClosing connection\n");
	        close(clisock);
	        exit(EXIT_FAILURE);
            }

	    if(strncmp("User correctly sign in...", buffer, 25) == 0) {
		// user sign in correctly
		printf("%s", buffer);
		printf("Welcome %s", user);
	    }
	    
	}
		
	// user doesn't want to sign in
	else if(strlen(buffer) == 1 && strncmp(buffer, "N", 1) == 0) {
	    // user doesn't want to register, closing connection and client
	    printf("\nClosing connection\n");
	    close(clisock);
	    exit(EXIT_FAILURE);	
	}
	
    }

    // user logged in correctly
    sprintf(buffer, "ok");
    write(clisock, (void *)buffer, strlen(buffer)); // send ok to server

    printf("\n--- LIST OF FILE ---\n"); 
    print_file(clisock); // list of file_path from server

    while(1) {

	printf("\n-------------------------\n");
	printf("Syntax menu options: \n1) search <file>\n2) download <file_path>\n3) upload <file_path>\n4) exit\nOption: ");
	sprintf(buffer, " ");
	fgets(buffer, 999, stdin); // user interts command
    	write(clisock, (void *)buffer, strlen(buffer)); // send command

	// closing client
	if(strlen(buffer) == 5 && strncmp(buffer, "exit", 4) == 0) {
	    close(clisock);
	    exit(EXIT_SUCCESS);
	}

	// search funcion 
	if(strlen(buffer) > 8 && strncmp(buffer, "search ", 7) == 0) {

	    printf("\n--- SEARCH RESULTS ---\n");
	    print_file(clisock);

        }

	// download function
	else if(strlen(buffer) > 10 && strncmp(buffer, "download ", 9) == 0) {

	    len = read(clisock, (void *)buffer, 1000);
    	    if(len > 0)
                buffer[len] = '\0';

	    // if server sends 0 --> error in file download 
	    if(strlen(buffer) == 1 && strncmp(buffer, "0", 1) == 0) {
		printf("\nError in dowload, maybe wrong path!\n");
		sprintf(buffer, "ok");
	        write(clisock, (void *)buffer, strlen(buffer)); // send ok to server
	    }

	    else {

		printf("\n--> File will be saved in working directory\n\n");	
		
		while(1) {

		    printf("------------------------\n");
		    printf("Insert name of file: ");
		    scanf("%s%*c", file_name);		    

		    // check if file already exist
		    if((f = fopen(file_name, "r")) != NULL) {	    
		        printf("Error, file already exist !! \n");	
			fclose(f);	
		    }

		    else
			break;
		}	

		// create and open file in writing
		f = fopen(file_name, "w");
		
		if(f == NULL) {
		    printf("\nError in name file\n");
		    close(clisock);			
		    exit(EXIT_FAILURE);
		}

		// if server sends 1 -> starts download after ok
		sprintf(buffer, "ok");
	        write(clisock, (void *)buffer, strlen(buffer)); // ok to server 
                
		while(1) {

	            len = read(clisock, (void *)buffer, 1000);
    	            if(len > 0)
                        buffer[len] = '\0';

		    // check escape sequence
	            if(strncmp(buffer, "escape_1234", 11) == 0) {
		        sprintf(buffer, "ok");
	                write(clisock, (void *)buffer, strlen(buffer)); // send ok to server and exit while
	                break;
	            }

		    fprintf(f, "%s", buffer); // write on file
		    sprintf(buffer, "ok");
	            write(clisock, (void *)buffer, strlen(buffer)); // send ok to server

	        }
			
		fclose(f);
		printf("\n--> File downloaded correctly\n");		

	    }
		
	}

	// upload function	
	else if(strlen(buffer) > 8 && strncmp(buffer, "upload ", 7) == 0) {

	    file_name[0]= '\0';
	    strcpy(file_name, &buffer[7]);
	    file_name[strlen(file_name)-1] = '\0'; // delete carriage return

	    // ok from server
	    len = read(clisock, (void *)buffer, 1000);	
	
	    // open file
	    file = open(file_name, O_RDONLY); 

            // error in file opening --> sending 0 to indicate upload error
            if(file == -1) {
	        printf("\nError in file opening, maybe file doesn't exist\n");
	        sprintf(buffer, "0");
	        write(clisock, (void *)buffer, strlen(buffer));
            }	

	    else {

		sprintf(buffer, "1");
	        write(clisock, (void *)buffer, strlen(buffer));

		len = read(clisock, (void *)buffer, 15); // ok from server

		while(1) {
		    printf("\n------------------------\n");
		    printf("File sending, it will be saved in server in /home\nInsert name of file: ");
		    scanf("%s%*c", buffer);
		    write(clisock, (void *)buffer, strlen(buffer));
			
                    len = read(clisock, (void *)buffer, 15);

		    // check if file name is ok by the server		   
		    if(strncmp(buffer, "ok", 2) == 0)
			break;
		    else
			printf("File name already exist in server in /home\n");		
		}
	
		while (1) {

    	            n = read(file, &buffer, 512);

	            if(n>0) {
	                write(clisock, (void *)buffer, n); // read from file
	                // ok from server
	                len = read(clisock, (void *)buffer, 15);
	            }  

	            else {
	                // sending escape sequence to indicate to the server that the sending is finished
                        sprintf(buffer, "escape_1234");
                        write(clisock, (void *)buffer, strlen(buffer));
                        // ok from server
                        len = read(clisock, (void *)buffer, 15);
	                break;
	            }
                }

		printf("\nFile uploaded correctly\n");	

	        // client receive new list with file uploaded
		printf("\n--- LIST OF FILE ---\n"); 
	        print_file(clisock);
	        close(file);	

	    } 		
	
	}

	// invalid command
	else {
	    len = read(clisock, (void *)buffer, 1000);
    	    if(len > 0)
                buffer[len] = '\0';
	    printf("\n%s", buffer);
        }

    }

}

// funcion that allow to print from server the list of file_path of server's /home
void print_file(int clisock) {

    char buffer[1000];
    int len;

    while(1) {

	len = read(clisock, (void *)buffer, 1000);
    	if(len > 0)
            buffer[len] = '\0';

	// if server sends 0 --> no file found
        if(strncmp(buffer, "0", 1) == 0) {
	    printf("No file found\n");
	    sprintf(buffer, "ok");
	    write(clisock, (void *)buffer, strlen(buffer)); // send ok to server and exit while
	    break;
	}

	// check escape sequence
	if(strncmp(buffer, "escape_1234", 11) == 0) {
	    sprintf(buffer, "ok");
	    write(clisock, (void *)buffer, strlen(buffer)); // send ok to server and exit while
	    break;
	}
	
	printf("%s\n", buffer);
	sprintf(buffer, "ok");
	write(clisock, (void *)buffer, strlen(buffer)); // send ok to server

    }    	

}

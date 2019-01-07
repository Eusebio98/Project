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

int main(void) {

    int clisock; // socket id
    struct sockaddr_in baddr;
    char buffer[1000];
    int len;
    char file_name[256]; // file name
    FILE *f; // file descriptor 
    char user[33];
	
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
    fgets(buffer, 199, stdin); // user interts username
    write(clisock, (void *)buffer, strlen(buffer)); // send username
    strcpy(user, buffer);

    len = read(clisock, (void *)buffer, 199);
    if(len > 0)
        buffer[len] = '\0';
    printf("%s", buffer);
    fgets(buffer, 199, stdin); // user interts password
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

    printf("\n--- LIST OF FILE ---\n"); // server sends list of file 

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

    while(1) {

	printf("\nSyntax menu options: \n1) search file\n2) download <file>\n3) upload <file>\n4) exit\nOption: ");
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

	// download function
	else if(strlen(buffer) > 10 && strncmp(buffer, "download ", 9) == 0) {

	    len = read(clisock, (void *)buffer, 1000);
    	    if(len > 0)
                buffer[len] = '\0';

	    // if server sends 0 --> error in file download 
	    if(strlen(buffer) == 1 && strncmp(buffer, "0", 1) == 0) {
		printf("\nError in dowload, maybe wrong path!\n");
		sprintf(buffer, "ok");
	        write(clisock, (void *)buffer, strlen(buffer)); // send ok to server and exit while
	    }

	    else {

		printf("\nInsert name of file: ");
		scanf("%s%*c", file_name);
		printf("File will be saved in working directory\n");

		// open file passed as argument
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
		printf("\nFile downloaded correctly\n");		

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

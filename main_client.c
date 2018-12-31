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

int main(void) {

    int clisock; // socket id
    struct sockaddr_in baddr;
    char buffer[1000];
    int len;
	
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

    if(strncmp("You don't have right permission... Closing connection", buffer, 53) == 0 || 
       strncmp("Impossible to open /home/user_pass.txt", buffer, 38) == 0) { 
	// username or password wrong or file /home/user_pass.txt error in server
        close(clisock);
	exit(EXIT_FAILURE);
    }

    // user logged in correctly
    sprintf(buffer, "ok");
    write(clisock, (void *)buffer, strlen(buffer)); // send ok to server

    while(1) {

	printf("\n-- LIST OF FILE --\n");

	while(1) {
	    len = read(clisock, (void *)buffer, 1000);
    	    if(len > 0)
                buffer[len] = '\0';

	    // check escape sequence
	    if(strncmp(buffer, "escape_1234", 11) == 0) {
		sprintf(buffer, "ok");
	        write(clisock, (void *)buffer, strlen(buffer)); // send ok to server
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

	    // invalid command
	    else {
		len = read(clisock, (void *)buffer, 1000);
    	    	if(len > 0)
                    buffer[len] = '\0';
		printf("\n%s", buffer);
            }

	}

    }

}



/*
*  C Implementation: nameClient
*
* Description:
*
*
* Author: Emilio Garcia <github@emiliogq.com>, (C) 2015
*
* Copyright: See COPYING file that comes with this distribution
*
*/

#include "nameClient.h"

/**
 * Function that sets the field addr->sin_addr.s_addr from a host name
 * address.
 * @param addr struct where to set the address.
 * @param host the host name to be converted
 * @return -1 if there has been a problem during the conversion process.
 */
 int setaddrbyname(struct sockaddr_in *addr, char *host)
 {
 	struct addrinfo hints, *res;
 	int status;

 	memset(&hints, 0, sizeof(struct addrinfo));
 	hints.ai_family = AF_INET;
 	hints.ai_socktype = SOCK_STREAM;

 	if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
 		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
 		return -1;
 	}

 	addr->sin_addr.s_addr = (((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr);

 	freeaddrinfo(res);

 	return 0;
 }

/**
 * Function that gets the dns_file name and port options from the program
 * execution.
 * @param argc the number of execution parameters
 * @param argv the execution parameters
 * @param reference parameter to set the host name.
 * @param reference parameter to set the port. If no port is specified
 * the DEFAULT_PORT is returned.
 */
 int getProgramOptions(int argc, char* argv[], char *host, int *_port)
 {
 	int param;
 	*_port = DEFAULT_PORT;

  // We process the application execution parameters.
 	while((param = getopt(argc, argv, "h:p:")) != -1){
 		switch((char) param){
 			case 'h':
 			strcpy(host, optarg);
 			break;
 			case 'p':
				// Donat que hem inicialitzat amb valor DEFAULT_PORT (veure common.h)
				// la variable port, aquest codi nomes canvia el valor de port en cas
				// que haguem especificat un port diferent amb la opcio -p
 			*_port = atoi(optarg);
 			break;
 			default:
 			printf("Parametre %c desconegut\n\n", (char) param);
 			return -1;
 		}
 	}

 	return 0;
 }

/**
 * Shows the menu options.
 */
 void printa_menu()
 {
		// Mostrem un menu perque l'usuari pugui triar quina opcio fer

 	printf("\nAplicatiu per la gestió d'un DNS Server\n");
 	printf("  0. Hola mon!\n");
 	printf("  1. Llistat dominis\n");
 	printf("  2. Consulta domini\n");
 	printf("  3. Alta Ip\n");
 	printf("  4. Alta Ips\n");
 	printf("  5. Modificacio Ip\n");
 	printf("  6. Baixa Ip\n");
 	printf("  7. Baixa Domini\n");
 	printf("  8. Sortir\n\n");
 	printf("Escolliu una opcio: ");
 }

/**
 * Function that sends a list request receives the list and displays it.
 * @param s The communications socket.
 */
 void process_list_operation(int s)
 {
 	char buffer[DNS_TABLE_MAX_SIZE];
 	int msg_size;
 	short op_code;

  //TODO: uncomment sendOpCodeMSG(s, MSG_LIST_RQ); //remember to implement sendOpCode in common.c
    sendOpCodeMSG(s, MSG_LIST_RQ);

 	memset(buffer, '\0', DNS_TABLE_MAX_SIZE);
  //TODO: rebre missatge LIST
    msg_size = recv(s, buffer, DNS_TABLE_MAX_SIZE, 0);
  //TODO: Descomentar la següent línia
    op_code = ldshort(buffer);


	printf("\nDATA RECEIVED \n");
	printf("---------------\n");
	printf("Operation code: %d \n",op_code);
    printDNSTableFromAnArrayOfBytes(buffer+sizeof(short), msg_size);
 }

/**
 * Function that process the menu option set by the user by calling
 * the function related to the menu option.
 * @param s The communications socket
 * @param option the menu option specified by the user.
 */
 void process_menu_option(int s, int option)
 {

 	
 	char buffer[MAX_BUFF_SIZE];
    memset(buffer, 0, sizeof(buffer));


 	switch(option){
 		case MENU_OP_HELLO:
            process_HELLO_RQ(s, buffer);
 			break;

        case MENU_OP_LIST:
            process_list_operation(s);
            break;
 		case MENU_OP_DOMAIN_RQ:
            process_DOMAIN_RQ(s, buffer);
 			break;

 		case MENU_OP_ADD_DOMAIN_IP:
            process_ADD_DOMAIN_IP_RQ(s, buffer);
 			break;

 		case MENU_OP_ADD_DOMAIN_IPS:
            process_ADD_DOMAIN_IPS_RQ(s, buffer);
 			break;
        case MENU_OP_CHANGE:
            process_CHANGE_DOMAIN_IP_RQ(s, buffer);
            break;
 		case MENU_OP_FINISH:
      		sendOpCodeMSG(s,MSG_FINISH);
 			break;

 		default:
            printf("Invalid menu option\n");
 	}
 }

void getDomain(char *domain)
{

	printf("Enter a domain por favor: ");
	scanf("%s",domain);

}

void getIPs(char* buffer,int bufferSize, char *stringIP){


 	int offset;
 	struct in_addr ip;

	offset = sizeof(short);


	do{
		ip = ldaddr(buffer+offset);
		strcat(stringIP,inet_ntoa(ip));
		strcat(stringIP," ");
		offset += sizeof(struct in_addr);
	}while(offset < bufferSize);

}

void process_HELLO_RQ(int s, char *buffer){
    short op_code;

    sendOpCodeMSG(s,MSG_HELLO_RQ);

    recv(s, buffer, MAX_BUFF_SIZE, 0);

    op_code = ldshort(buffer);

    printDataReceived(op_code,buffer);
}

void process_DOMAIN_RQ(int s, char *buffer){
    
    char domain[NAME_LENGTH];
    char ips[MAX_BUFF_SIZE];

    int bufferSize = 0;
    short op_code;
    short typeError;

    getDomain(domain);

    bufferSize = sizeof(short)+strlen(domain);

    stshort(MSG_DOMAIN_RQ,buffer);

    strcpy(buffer+sizeof(short),domain);

    send(s, buffer, sizeof(short)+strlen(domain),0);

    memset(buffer,0,MAX_BUFF_SIZE);

    bufferSize=recv(s,buffer,MAX_BUFF_SIZE,0);

    op_code = ldshort(buffer);

    if (op_code == MSG_DOMAIN){
        
        memset(ips, 0, MAX_IPS);
        getIPs(buffer,bufferSize,ips);
        // This function expects a buffer+sizeof(short).
        // For that, we substract by two in stringIP.
        printDataReceived(op_code,ips-2);
        
    }
    // There is an error
    else {
        typeError = ldshort(buffer+2);
        if (typeError == ERR_2){
            printDataReceived(op_code,"Inexistent domain"-2);
        }
    }
}

void process_ADD_DOMAIN_IP_RQ(int s, char *buffer){
    
    short opCode, typeError;
    char domain[NAME_LENGTH];
    char stringIP[15];
    struct _IP *ipEntry;

    int bufferSize = 0;

    getDomain(domain);

    stshort(MSG_ADD_DOMAIN,buffer);
    bufferSize = sizeof(short);

    strcpy(buffer+bufferSize,domain);
    bufferSize += strlen(domain)+1;

    memset(stringIP,0,15);
    printf("Enter an IP por favor: ");
    scanf("%s",stringIP);

    ipEntry = malloc(sizeof(struct _IP));

    inet_aton(stringIP,&(ipEntry->IP));
    staddr(ipEntry->IP,buffer+bufferSize);
    bufferSize += sizeof(struct in_addr);

    free(ipEntry);

    send(s,buffer,bufferSize,0);

    memset(buffer,'\0',MAX_BUFF_SIZE);

    recv(s,buffer,4,0);

    opCode = ldshort(buffer);

    if (opCode == MSG_OP_OK){
        printDataReceived(opCode,"IP added successfully."-2);
    }
    else{
        typeError = ldshort(buffer+2);
        if (typeError == ERR_3){
            printDataReceived(opCode,"MAX_IPS limit exceeded"-2);
        }
    }
}

void process_ADD_DOMAIN_IPS_RQ(int s, char *buffer){
    
    short opCode, typeError;
    int IPcount = 0;
    int bufferSize = 0;
    char moreIP = 'Y';
    char domain[NAME_LENGTH];

    struct _IP *ipEntry;
    char stringIP[15];
    getDomain(domain);

    stshort(MSG_ADD_DOMAIN, buffer);
    bufferSize = sizeof(short);

    strcpy(buffer+bufferSize,domain);
    bufferSize += strlen(domain)+1;

    while((moreIP == 'Y' || moreIP == 'y') && IPcount < 10)
    {
        printf("Enter an IP por favor: ");
        scanf("%s",stringIP);
       
        if (strlen(stringIP) > 0){
            ipEntry = malloc(sizeof(struct _IP));
            inet_aton(stringIP,&(ipEntry->IP));
            staddr(ipEntry->IP,buffer+bufferSize);
            bufferSize += sizeof(struct in_addr);
            IPcount++;

            printf("You have entered %d IP(s). You can enter %d IP(s) more.\n", IPcount, 10-IPcount);

            printf("Do you want enter other IP (Y/N): ");
            scanf(" %c",&moreIP);
            memset(stringIP,'\0',15);
        }
    }
    //while(moreIP == 'Y' && strlen(stringIP) > 0 && IPcount < 10);
    free(ipEntry);
    send(s,buffer,bufferSize,0);

    recv(s,buffer,4,0);

    opCode = ldshort(buffer);

    if (opCode == MSG_OP_OK){
        printDataReceived(opCode,"IP added successfully."-2);
    }
    else{
        typeError = ldshort(buffer+2);
        if (typeError == ERR_3){
            printDataReceived(opCode,"MAX_IPS limit exceeded"-2);
        }
    }
}

void process_CHANGE_DOMAIN_IP_RQ(int s, char *buffer){

    short opCode, typeError;
    
    int bufferSize = 0;

    char domain[NAME_LENGTH];

    struct _IP *ipEntry;
    char stringIP[15];
    getDomain(domain);

    stshort(MSG_CHANGE_DOMAIN, buffer);
    bufferSize = sizeof(short);

    strcpy(buffer+bufferSize,domain);
    bufferSize += strlen(domain)+1;

    printf("Enter the old IP por favor: ");
    scanf("%s",stringIP);

    ipEntry = malloc(sizeof(struct _IP));
    inet_aton(stringIP,&(ipEntry->IP));
    staddr(ipEntry->IP,buffer+bufferSize);
    bufferSize += sizeof(struct in_addr);

    memset(stringIP,'\0',15);

    printf("Enter the new IP por favor: ");
    scanf("%s",stringIP);

    ipEntry = malloc(sizeof(struct _IP));
    inet_aton(stringIP,&(ipEntry->IP));
    staddr(ipEntry->IP,buffer+bufferSize);
    bufferSize += sizeof(struct in_addr);

    free(ipEntry);
    
    send(s,buffer,bufferSize,0);


    recv(s,buffer,4,0);

    opCode = ldshort(buffer);

    if (opCode == MSG_OP_OK){
        printDataReceived(opCode,"IP changed successfully."-2);
    }
    else{
        typeError = ldshort(buffer+2);
        if (typeError == ERR_1 || typeError == ERR_2){
            printDataReceived(opCode,"Unsuccessfully changed."-2);
        }
    }


}

 int main(int argc, char *argv[])
 {
	int port; // variable per al port inicialitzada al valor DEFAULT_PORT (veure common.h)
	char host[MAX_HOST_SIZE]; // variable per copiar el nom del host des de l'optarg
	int option = 0; // variable de control del menu d'opcions
	int ctrl_options;
	int clientSocket;

	ctrl_options = getProgramOptions(argc, argv, host, &port);

	// Comprovem que s'hagi introduit un host. En cas contrari, terminem l'execucio de
	// l'aplicatiu
	if(ctrl_options<0){
		perror("No s'ha especificat el nom del servidor\n\n");
		return -1;
	}

 	//TODO: setting up the socket for communication

	clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port); // Server Port
	setaddrbyname(&address,host); // Server address

	socklen_t addrlen = sizeof(address);

	// int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)

	connect(clientSocket, (struct sockaddr *)&address, addrlen);

	do{
		printa_menu();
		  // getting the user input.

		scanf("%d",&option);
		printf("\n\n");
		process_menu_option(clientSocket, option);

	  }while(option != MENU_OP_FINISH); //end while(opcio)
    // TODO

	  close(clientSocket);

	  return 0;
	}



/*
*  C Implementation: nameServer
*
* Description:
*
*
* Author: Emilio Garcia <github@emiliogq.com>, (C) 2015
*
* Copyright: See COPYING file that comes with this distribution
*
*/

#include "nameServer.h"



/* Reads a line ended with \n from the file pointer.  */
/* Return: a line ended not with an EOL but with a 0 or NULL if the end of the
file is reached */


struct _DNSEntry* searchDomain(char *domain, struct _DNSEntry* firstEntry){

  struct _DNSEntry *currentEntry;

  currentEntry = firstEntry;

  while(currentEntry != NULL && strcmp(currentEntry->domainName,domain) != 0 ){
    currentEntry = currentEntry->nextDNSEntry;
  }

  return currentEntry;
}

struct _IP* searchIP(struct in_addr IPtoSearch, struct _IP* firstIP)
{
  return firstIP;
}

void putIPInBuffer(char *buffer, int *offset, struct _IP* currentIP)
{
  staddr(currentIP->IP,buffer+(*offset));
  *offset += sizeof(struct in_addr);
}



char *readLine(FILE *file, char *line, int sizeOfLine)
{

  int line_length;

  if (fgets(line, sizeOfLine, file) != NULL)
  {
    line_length = strlen(line)-1;
    line[line_length] = 0;
  }
  else
  {
    line = NULL;
  }

  return line;
}


/**
 * Creates a DNSEntry variable from the content of a file line and links it
 * to the DNSTable.
 * @param line the line from the file to be parsed
 * @param delim the character between tokens.
 */
struct _DNSEntry* buildADNSEntryFromALine(char *line, char *token_delim)
{

  char *token;
  struct _IP *ip_struct = malloc(sizeof(struct _IP));
  struct _IP *last_ip_struct;
  struct _DNSEntry* dnsEntry = malloc(sizeof(struct _DNSEntry));
  int firstIP = 1;


  //getting the domain name
  token = strtok(line, token_delim);
  strcpy(dnsEntry->domainName, token);
  dnsEntry->numberOfIPs = 0;

  //getting the Ip's
  while ((token = strtok(NULL, token_delim)) != NULL)
  {
    ip_struct = malloc(sizeof(struct _IP));
    inet_aton((const char*)token, &(ip_struct->IP));
    ip_struct->nextIP = NULL;
    (dnsEntry->numberOfIPs)++;
    if (firstIP == 1)
    {
      dnsEntry->first_ip = ip_struct;
      last_ip_struct = ip_struct;
      firstIP = 0;
    }
    else
    {
      last_ip_struct->nextIP = ip_struct;
      last_ip_struct = ip_struct;
    }
  }

    return dnsEntry;
}

/* Reads a file with the dns information and loads into a _DNSTable structure.
Each line of the file is a DNS entry.
RETURNS: the DNS table */
struct _DNSTable* loadDNSTableFromFile(char *fileName)
{
  FILE *file;
  char line[1024];
  struct _DNSEntry *dnsEntry;
  struct _DNSEntry *lastDNSEntry;
  struct _DNSTable *dnsTable = malloc(sizeof(struct _DNSTable));
  int firstDNSEntry = 1;

  file = fopen(fileName, "r");
  if (file==NULL)
  {
    perror("Problems opening the file");
    printf("Errno: %d \n", errno);
  }
  else
  {
    //reading the following entries in the file
    while(readLine(file, line, sizeof(line)) != NULL)
    {
      dnsEntry = buildADNSEntryFromALine(line, " ");
      dnsEntry->nextDNSEntry = NULL;
      if (firstDNSEntry == 1)
      {
        dnsTable->first_DNSentry = dnsEntry;
        lastDNSEntry = dnsEntry;
        firstDNSEntry = 0;
      }
      else
      {
        lastDNSEntry->nextDNSEntry = dnsEntry;
        lastDNSEntry = dnsEntry;
      }
    }


    fclose(file);
  }

  return dnsTable;
}


/**
 * Calculates the number of bytes of the DNS table as a byte array format.
 * It does not  include the message identifier.
 * @param dnsTable a pointer to the DNSTable in memory.
 */
int getDNSTableSize(struct _DNSTable* dnsTable)
{
  int table_size = 0;
  int numberOfIPs_BYTES_SIZE = sizeof(short);


  struct _DNSEntry *dnsEntry;

  dnsEntry = dnsTable->first_DNSentry;
  if(dnsEntry != NULL)
  {
    do
    {
      table_size +=  ( strlen(dnsEntry->domainName) + SPACE_BYTE_SIZE +
        numberOfIPs_BYTES_SIZE + (dnsEntry->numberOfIPs * sizeof (in_addr_t)) );
    }while((dnsEntry=dnsEntry->nextDNSEntry) != NULL);
  }


  return table_size;
}



/*Return a pointer to the last character copied in next_DNSEntry_ptr + 1 */
/**
 * Converts the DNSEntry passed as a parameter into a byte array pointed by
 * next_DNSEntry_ptr. The representation will be
 * domain_name\0number_of_ips[4byte_ip]*].
 * @param dnsEntry the DNSEntry to be converted to a Byte Array.
 * @param next_DNSEntry_ptr a pointer to Byte Array where to start copying
 * the DNSEntry. The pointer moves to the end of the ByteArray representation.
 */
void dnsEntryToByteArray(struct _DNSEntry* dnsEntry, char **next_DNSEntry_ptr)
{

  struct _IP* pIP;

  fflush(stdout);

  strcpy(*next_DNSEntry_ptr, dnsEntry->domainName);
  //we leave one 0 between the name and the number of IP's of the domain
  *next_DNSEntry_ptr += (strlen(dnsEntry->domainName) + 1);
  stshort(dnsEntry->numberOfIPs, *next_DNSEntry_ptr);
  *next_DNSEntry_ptr += sizeof(short);
  if((pIP = dnsEntry->first_ip) != NULL)
  {
    do
    {
      staddr(pIP->IP, *next_DNSEntry_ptr);
      *next_DNSEntry_ptr += sizeof(in_addr_t);
    }while((pIP = pIP->nextIP) != NULL);
  }

}


/*Dumps the dnstable into a byte array*/
/*@Return a pointer to the byte array representing the DNS table */
/*@param dnsTable the table to be serialized into an array of byes */
/*@param _tableSize reference parameter that will be filled with the table size*/
char *dnsTableToByteArray(struct _DNSTable* dnsTable, int *_tableSize)
{
  int tableSize = getDNSTableSize(dnsTable);
  *_tableSize = tableSize;

  char *dns_as_byteArray = malloc(tableSize);
  char *next_dns_entry_in_the_dns_byteArray_ptr = dns_as_byteArray;
  struct _DNSEntry *dnsEntry;


  bzero(dns_as_byteArray, tableSize);

  dnsEntry = dnsTable->first_DNSentry;
  do
  {
    dnsEntryToByteArray(dnsEntry, &next_dns_entry_in_the_dns_byteArray_ptr);
  }while((dnsEntry=dnsEntry->nextDNSEntry) != NULL);

  return dns_as_byteArray;

}

/**
 * Function that gets the dns_file name and port options from the program
 * execution.
 * @param argc the number of execution parameters
 * @param argv the execution parameters
 * @param reference parameter to set the dns_file name.
 * @param reference parameter to set the port. If no port is specified
 * the DEFAULT_PORT is returned.
 */
int getProgramOptions(int argc, char* argv[], char *dns_file, int *_port)
{
  int param;
   *_port = DEFAULT_PORT;

  // We process the application execution parameters.
	while((param = getopt(argc, argv, "f:p:")) != -1){
		switch((char) param){
			case 'f':
				strcpy(dns_file, optarg);
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

void process_HELLO_RQ_msg(int sock, char *buffer)
{
  char msg[MAX_BUFF_SIZE];

  strcpy(msg,"Hello world");
  // Clean buffer. \0 means 0 byte.
  memset(buffer, '\0', MAX_BUFF_SIZE);

  stshort(MSG_HELLO, buffer);

  strcpy(buffer+2,msg);

  send(sock, buffer, sizeof(short)+strlen(msg)+1,0);

  memset(buffer, 0, MAX_BUFF_SIZE);
}

/**
 * Function that generates the array of bytes with the dnsTable data and
 * sends it.
 * @param s the socket connected to the client.
 * @param dnsTable the table with all the domains
 */
void process_LIST_RQ_msg(int sock, struct _DNSTable *dnsTable)
{
  char *dns_table_as_byteArray;
  char *msg;
  int dns_table_size;
  int msg_size;

  msg_size = sizeof(short);

  dns_table_as_byteArray = dnsTableToByteArray(dnsTable, &dns_table_size);

  msg_size += dns_table_size;

  msg = malloc(msg_size);
  //TODO: set the operation code and the table data
  //TODO: send the message
  stshort(MSG_LIST, msg);
  memcpy(msg+2,dns_table_as_byteArray,dns_table_size);
  send(sock, msg, msg_size,0);
  free(msg);
}


void process_DOMAIN_RQ_msg(int sock, char *buffer, struct _DNSTable *dnsTable)
{
    char domainNameToSearch[NAME_LENGTH];
    // OpCode
    int offset = sizeof(short);
    struct _DNSEntry *currentEntry;
    struct _IP *currentIP;

    memset(domainNameToSearch, 0,sizeof(domainNameToSearch));
    strcpy(domainNameToSearch,buffer+offset);

    currentEntry = searchDomain(domainNameToSearch, dnsTable->first_DNSentry);

    // If we've found the right answer....

    if (currentEntry != NULL) {

        stshort(MSG_DOMAIN,buffer);

        currentIP = currentEntry->first_ip;

        while(currentIP != NULL)
        {
          putIPInBuffer(buffer,&offset,currentIP);
          currentIP = currentIP->nextIP;
        }

        send(sock,buffer,offset,0);
    }
    else {
      sendOpCodeError(sock, ERR_2);
    }

    memset(buffer, '\0', MAX_BUFF_SIZE);

}




void process_ADD_DOMAIN_RQ_msg(int s, char *buffer, struct _DNSTable* dnsTable, int bufferSize)
{
  


  char domain[NAME_LENGTH];

  struct _DNSEntry *previousEntry;
  struct _DNSEntry *currentEntry;
  
  struct _IP *newIP;

  int num_IP;
  int domainFound = 0;
  int offset = sizeof(short);
  
  strcpy(domain, buffer+offset);
  offset += strlen(domain) + 1;
  num_IP = (bufferSize - offset) / sizeof(struct in_addr);

  currentEntry = dnsTable->first_DNSentry;
  while(currentEntry != NULL && domainFound == 0){
    if(strcmp(domain,currentEntry->domainName) == 0){
        domainFound = 1;
    }
    else{
        previousEntry = currentEntry;
        currentEntry = currentEntry->nextDNSEntry;
    }
  }

  if (domainFound == 0){
    currentEntry = malloc(sizeof(struct _DNSEntry));
    strcpy(currentEntry->domainName,domain);
    currentEntry->first_ip = NULL;
    currentEntry->numberOfIPs = 0;
    previousEntry->nextDNSEntry = currentEntry;
    currentEntry->nextDNSEntry = NULL;
  }


  if (currentEntry->numberOfIPs < MAX_IPS){
    while((currentEntry->numberOfIPs < MAX_IPS) && (num_IP > 0)){
        newIP = malloc(sizeof(struct _IP));
        newIP->IP = ldaddr(buffer+offset);
        offset += sizeof(struct in_addr);
        newIP->nextIP = currentEntry->first_ip;
        currentEntry->first_ip = newIP;
        currentEntry->numberOfIPs++;
        num_IP--;
    }

    sendOpCodeMSG(s,MSG_OP_OK);
  }
  else
  {
    sendOpCodeError(s,ERR_3);
  }


}

void process_CHANGE_DOMAIN_IP(int s, char *buffer, struct _DNSTable* dnsTable){

  char domain[NAME_LENGTH];

  struct _DNSEntry *currentEntry;
  
  struct in_addr oldIP;
  struct _IP *currentIP;

  int bufferSize = sizeof(short);
  
  strcpy(domain, buffer+bufferSize);
  bufferSize += strlen(domain) + 1;


  currentEntry = dnsTable->first_DNSentry;
  while(currentEntry != NULL && (strcmp(domain, currentEntry->domainName) != 0)){
    currentEntry = currentEntry->nextDNSEntry;
  }
  if (currentEntry == NULL) sendOpCodeError(s,ERR_2);
  else {
      
      oldIP = ldaddr(buffer+bufferSize);
      bufferSize += sizeof(struct in_addr);

      currentIP = currentEntry->first_ip;
      
      while(currentIP != NULL && currentIP->IP.s_addr != oldIP.s_addr){
        currentIP = currentIP->nextIP;
      }

      if (currentIP != NULL)
      {
        currentIP->IP = ldaddr(buffer+bufferSize);
        sendOpCodeMSG(s,MSG_OP_OK);
      }
      else
      {
        sendOpCodeError(s,ERR_1);
      }

  }

}
/**
 * Receives and process the request from a client.
 * @param s the socket connected to the client.
 * @param dnsTable the table with all the domains
 * @return 1 if the user has exit the client application therefore the
 * connection whith the client has to be closed. 0 if the user is still
 * interacting with the client application.
 */
int process_msg(int sock, struct _DNSTable *dnsTable)
{
  unsigned short op_code;
  int done = 0;
  int bufferSize;

  char buffer[MAX_BUFF_SIZE];

  memset(buffer,'\0',MAX_BUFF_SIZE);
  
  bufferSize = recv(sock,buffer,MAX_BUFF_SIZE,0);
  op_code = ldshort(buffer);

  switch(op_code)
  {
    case MSG_HELLO_RQ:
      process_HELLO_RQ_msg(sock, buffer);
      break;
    case MSG_LIST_RQ:
      process_LIST_RQ_msg(sock, dnsTable);
      break;

    case MSG_DOMAIN_RQ:
      process_DOMAIN_RQ_msg(sock, buffer, dnsTable);
      break;

    case MSG_ADD_DOMAIN:
      process_ADD_DOMAIN_RQ_msg(sock,buffer,dnsTable,bufferSize);
      break;

    case MSG_CHANGE_DOMAIN:
      process_CHANGE_DOMAIN_IP(sock,buffer,dnsTable);
      break;
    case MSG_FINISH:
      //TODO
      done = 1;
      break;
    default:
      perror("Message code does not exist.\n");
  }

  return done;
}



int main (int argc, char * argv[])
{
  struct _DNSTable *dnsTable;
  int port ;
  char dns_file[MAX_FILE_NAME_SIZE] ;
  int finish = 0;
  int serverSocket, serverSocket2;
  int pid;

  getProgramOptions(argc, argv, dns_file, &port);

  dnsTable = loadDNSTableFromFile(dns_file);
  printDNSTable(dnsTable);

  //TODO: setting up the socket for communication
  serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;

  socklen_t addrlen = sizeof(address);

  // bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
  bind(serverSocket, (struct sockaddr *)&address, addrlen);
  // int listen(int sockfd, int backlog)
  listen(serverSocket, MAX_QUEUED_CON);

  while(!finish){
    serverSocket2 = accept(serverSocket, (struct sockaddr *)&address, &addrlen);
    pid = fork();
    if (pid == 0){
      close(serverSocket);
      do {
          finish = process_msg(serverSocket2, dnsTable);
      }while(!finish);
      close(serverSocket2);
    }    
  }
  
  if (pid != 0) close(serverSocket);

  return 0;
}



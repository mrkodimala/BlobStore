#include <winsock2.h>
#include <StdAfx.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <conio.h>

int getsocket()
{
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		return -1;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		return -1;
	}
	free(p_int);

	return hsock;
}


///////////////////////////////////////////////////////////////////////
//////////////////START OF BLOB FUNCTIONS/////////////////////////////
//////////////////////////////////////////////////////////////////////


void sendFile(struct sockaddr_in my_addr,char *filename)
{
	char buffer[1024];
	int buffer_len = 1024;
	char character;
	int bytecount;
	int c;
	FILE *file = fopen(filename, "rb+");
	//FILE *dest = fopen(filename2, "ab");
	int bufferoffset = 0;
	int flag = 0;
	while (true){
		if (flag == 1)
			break;
		int hsock = getsocket();
		if (connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR){
			fprintf(stderr, "Error connecting the socket\n", WSAGetLastError());
			return;
		}
		
		if (!feof(file))
		{
			int offset = 0;
			while (!feof(file))
			{
				fread(&character, sizeof(char), 1, file);
				buffer[offset++] = character;
				if (offset == 1020)
					break;
			}
			buffer[offset] = '\0';
			printf("\noffset=%d\n", offset);
			printf("\n%s\n", buffer);
			getchar();
		}
		else{
			printf("\n\nEnd of the file is reached");
			strcpy(buffer, "$end$");
			flag = 1;
		}

		if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			return;
		}
		//printf("Sent bytes %d\n", bytecount);
		//Sleep(100);
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			return;
		}
		//printf("replied buffer: %s\n", buffer);
		closesocket(hsock);
	}
}

void sendCommand(struct sockaddr_in my_addr, char *command);
char GlobalFileName[32];

void Display(struct sockaddr_in my_addr,char *buffer)
{
	char temp[200];
	int i;
	int offset = 0;
	int screennumber = buffer[1] - '0';
	for (i = 2; buffer[i] != '$'; i++)
		temp[offset++] = buffer[i];
	temp[offset] = '\0';
	//printf("\nIn the Display function\n%s\nNo=%d\n", temp,screennumber);
	i++;
	int min = buffer[i] - '0';
	i++; i++;
	int max = buffer[i] - '0';
	int flag = 1;
	if (screennumber == 1)
	{
		while (flag)
		{
			system("cls");
			printf("%s\n", temp);
			//printf("min=%d\tmax=%d\n", min, max);
			printf("\n\tEnter you option\n\t");
			int option;
			scanf("%d", &option);
			if (option >= min&&option <= max)
			{
				flag = 0;
				char temp[64] = "$mainscreen$";
				char temp2[10];
				itoa(option, temp2, 10);
				strcat(temp, temp2);
				sendCommand(my_addr, temp);
			}
			else{
				printf("Enter Valid option\n");
				getchar();
			}
		}
	}
	else if (screennumber == 2)
	{
		while (flag)
		{
			system("cls");
			printf("%s\n", temp);
			//printf("min=%d\tmax=%d\n", min, max);
			printf("\n\tEnter you option\n\t");
			int option;
			scanf("%d", &option);
			if (option >= min&&option <= max)
			{
				flag = 0;
				char temp[64] = "$login$";
				if (option == 1)
				{
					strcat(temp, "loginuser$");
				}
				else{
					strcat(temp, "adduser$");
				}
				printf("\n\tEnter the UserName\n\t");
				char username[32];
				gets(username);
				gets(username);
				strcat(temp, username);
				sendCommand(my_addr, temp);
			}
			else{
				printf("Enter Valid option\n");
				getchar();
			}
		}
	}
	else if (screennumber == 3)
	{
		while (flag)
		{
			system("cls");
			printf("%s\n", temp);
			printf("\n\tEnter you option\n\t");
			int option;
			scanf("%d", &option);
			if (option >= min&&option <= max)
			{
				flag = 0;
				char temp[64] = "$files$";
				char filename[32];
				if (option == 1)
				{
					strcat(temp, "addfile$");
					printf("\n\tEnter Filename - File must be in this folder\n\t");
					gets(filename);
					gets(filename);
					strcat(temp, filename);
				}
				else{
					strcat(temp, "viewfiles$");
				}
				sendCommand(my_addr, temp);
				sendFile(my_addr, filename);
				sendCommand(my_addr, "$opened$");
			}
			else{
				printf("Enter Valid option\n");
				getchar();
			}
		}
	}
	else if (screennumber == 4)
	{
		while (flag)
		{
			system("cls");
			printf("%s", temp);
			printf("Enter your FileNo\n\t");
			int option;
			scanf("%d", &option);
			char buffer[1024];
			if (option >= min&&option <= max)
			{
				printf("\n\tEnter 1 to Delete\n\tEnter 2 to Download\n\t");
				int internaloption;
				scanf("%d", &internaloption);
				if (internaloption == 1)
				{
					flag = 0;
					strcpy(buffer, "$filesoptions$delete$");
					char temp[2]; temp[0] = option + 48; temp[1] = '\0';
					strcat(buffer, temp);
					sendCommand(my_addr, buffer);
				}
				else if (internaloption == 2)
				{
					flag = 0;
					strcpy(buffer, "$filesoptions$download$");
					char temp[2]; temp[0] = option + 48; temp[1] = '\0';
					strcat(buffer, temp);
					sendCommand(my_addr, buffer);
				}
			}
			else{
				printf("\nInvalid option");
				getchar();
			}
		}
	}
	else if (screennumber == 5)
	{
		system("CLS");
		printf("%s", temp);
		int i = 10;
		int offset = 0;
		for (; temp[i] != '\0'&&temp[i] != '\n'; i++)
		{
			GlobalFileName[offset++] = temp[i];
		}
		GlobalFileName[offset] = '\0';
		printf("%s", GlobalFileName);
		FILE *file = fopen(GlobalFileName, "wb");
		fclose(file);
		char buffer[32] = "$senddatablock$";
		sendCommand(my_addr, buffer);
		getchar();
		getchar();
	}
	
}



void WriteDataToFile(char *buffer, struct sockaddr_in my_addr)
{
	//printf("in the write to the file %s\n", buffer);
	if (!strcmp(buffer, "end$$$"))
	{
		system("CLS");
		printf("File Downloaded Successfully\n");
		getchar();
		getchar();
		sendCommand(my_addr, "$opened$");
		return;
	}
	FILE *file = fopen(GlobalFileName, "ab");
	char data[1020];
	memset(&data, 0, sizeof(data));
	int length = strlen(buffer);
	for (int i = 0; i<1020 && buffer[i] != '\0'; i++)
	{
		data[i] = buffer[i];
	}
	if (ftell(file) > 0)
	{
		fseek(file, -1, SEEK_SET);
	}
	fwrite(&data, sizeof(data), 1, file);
	fclose(file);
	sendCommand(my_addr, "$senddatablock$");
}

//////////////////////////////////////////////////////
//END OF BLOB FUNCTIONS///////////////////////////////
/////////////////////////////////////////////////////

void RespondtoServer(struct sockaddr_in my_addr,char *buffer)
{

	//printf("\nIn the Responfunction\n%s\n", buffer);
	char ch = buffer[0];
	//printf("\n%c\n", ch);
	if (buffer[0] == '@')
	{
		Display(my_addr, buffer);
	}
	else if (buffer[0] == '$')
	{
		if (strcmp(buffer, "$sendFile"))
		{
			printf("%s", buffer);
			getchar();
			sendCommand(my_addr, "$opened$");
		}
	}
	else{
		//printf("Write to the file  is called");
		WriteDataToFile(buffer, my_addr);
	}
}



void sendCommand(struct sockaddr_in my_addr, char *command)
{
	int bytecount;
	char buffer[1024];
	int buffer_len=1024;
	strcpy(buffer, command);
	int hsock = getsocket();
	if (connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR){
		fprintf(stderr, "Error connecting the socket\n", WSAGetLastError());
		return;
	}
	
	if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	printf("Sent bytes %d\n", bytecount);
	//Sleep(100);
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	closesocket(hsock);
	char tempbuffer[1024];
	int i;
	for (i = 0; buffer[i] != '#'; i++)
		tempbuffer[i] = buffer[i];
	tempbuffer[i] = '\0';
	//printf("\n\n%s", tempbuffer);
	RespondtoServer(my_addr,tempbuffer);
}


void socket_client()
{

	//The port and address you want to connect to
	int host_port= 1101;
	char* host_name="127.0.0.1";

	//Initialize socket support WINDOWS ONLY!
	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "Could not find sock dll %d\n",WSAGetLastError());
		return;
	}

	//Initialize sockets and set any options

	//Connect to the server
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = inet_addr(host_name);

	//if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
	//	fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
	//	goto FINISH;
	//}

	//Now lets do the client related stuff
	//sendFileName(my_addr, "$filename$Copy.txt");
	//sendFile(my_addr,"ReadMe.txt","dest.txt");
	sendCommand(my_addr, "$opened$");
	getchar();
}
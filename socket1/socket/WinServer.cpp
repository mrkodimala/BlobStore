#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
struct node
{
	char msg[128];
	int msg_id;
	node *next;
}*flist,*alist,*printid;

struct bufserv{
	
		int userId;
		int forumId;
		int msgId;
		int commentId;
		int choice;
		char *forumname;
		char msg[128];
}buf1;

bool flag=true;
int mid = 0;
int count1 =0;
char *Data[100];
int count=1;
int values[100];
DWORD WINAPI SocketHandler(void*);
void replyto_client(char *buf, int *csock);












///////////////////////////////////////////////////////////////////////////
////////START OF BLOB FUNCTIONS AND VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////


char GlobalFileName[256];
const int USERSOFFSET = 131072;
int CURRENT_USER;
int DATABLOCK;
int CURRENTFILE;
int CURRENTFILENUMBER;
struct DataBuffer{
	char Buffer[1024];
};


struct User{
	char username[28];
	int files_offset;
};

struct UserData{
	struct User users[31];
	int count;
	char unuser[28];
};

struct BitVector{
	unsigned int values[16384];
};

struct File{
	char name[28];
	int offset;
};

struct FilesCollection{
	struct File files[31];
	int files_count;
	int next_collection;
};

struct DataBlock{
	char data[1020];
	int nextoffset;
};

int GiveFreeBlock()
{
	int s = 1;
	int flag = 0;
	int count = 0;
	int j = 0;
	struct BitVector b;
	FILE *file = fopen("blob.bin", "r+b");
	fread(&b, sizeof(struct BitVector), 1, file);
	for (int i = 0; i < 16384; i++)
	{
		int v = b.values[i];
		j = 1;
		s = 1;
		if (v < 2147483647)
		{
			//printf("%d\t", v);
			do{
				flag = v&s;
				count++;
				if (flag == 0)
				{
					b.values[i] = v^s;
					printf("\nB value=%d\n", b.values[i]);
					fseek(file, 0, SEEK_SET);
					fwrite(&b, sizeof(struct BitVector), 1, file);
					fclose(file);
					return USERSOFFSET + (count*1024);
				}
				s = s << 1;
				j++;
			} while (j < 32);
		}
		else{
			count = count + 31;
		}
	}
	fclose(file);
	return 0;
}


void FreeBlock(int blockno)
{
	FILE *file = fopen("blob.bin","rb+");
	struct BitVector b;
	fread(&b, sizeof(struct BitVector), 1, file);
	blockno = blockno - USERSOFFSET;
	blockno = blockno / 1024;
	int d = blockno / 32;
	int r = blockno % 32;
	unsigned int s = 1;
	unsigned int v = b.values[d];
	int i;
	if (d == 0)
		i = 2;
	else
		i = 0;
	for (; i <= r; i++)
	{
		s = s << 1;
	}
	b.values[d] = v^s;
	fseek(file, 0, SEEK_SET);
	fwrite(&b, sizeof(struct BitVector), 1, file);
	fclose(file);
}

void socket_server() {

	//The port you want the server to listen on
	int host_port= 1101;

	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "No sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set options
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	//Bind and listen
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	
	/* if you get error in bind 
	make sure nothing else is listening on that port */
	if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
		fprintf(stderr,"Error binding to socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	if(listen( hsock, 10) == -1 ){
		fprintf(stderr, "Error listening %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	//Now lets do the actual server stuff

	int* csock;
	sockaddr_in sadr;
	int	addr_size = sizeof(SOCKADDR);
	
	while(true){
		printf("waiting for a connection\n");
		csock = (int*)malloc(sizeof(int));
		
		if((*csock = accept( hsock, (SOCKADDR*)&sadr, &addr_size))!= INVALID_SOCKET ){
			//printf("Received connection from %s",inet_ntoa(sadr.sin_addr));
			CreateThread(0,0,&SocketHandler, (void*)csock , 0,0);
		}
		else{
			fprintf(stderr, "Error accepting %d\n",WSAGetLastError());
		}
	}

FINISH:
;
}


//Blob objects
void WriteToTheFile(char *buffer)
{
	FILE *file = fopen("blob.bin", "rb+");
	struct DataBlock d,temp;
	struct FilesCollection f;
	struct File binfile;
	strcpy(d.data, buffer);
	d.nextoffset = 0;
	if (DATABLOCK == 0)
	{
		fseek(file, CURRENTFILE, SEEK_SET);
		fread(&f, sizeof(struct FilesCollection), 1, file);
		binfile = f.files[CURRENTFILENUMBER];
		binfile.offset = GiveFreeBlock();
		f.files[CURRENTFILENUMBER] = binfile;
		fseek(file, CURRENTFILE, SEEK_SET);
		fwrite(&f, sizeof(struct FilesCollection), 1, file);
		DATABLOCK = binfile.offset;
		fseek(file, DATABLOCK, SEEK_SET);
		fwrite(&d, sizeof(struct DataBlock), 1, file);
	}
	else{
		fseek(file, DATABLOCK, SEEK_SET);
		fread(&temp, sizeof(struct DataBlock), 1, file);
		temp.nextoffset = GiveFreeBlock();
		//printf("\n\n%s\n\n\n\n%d\n\n\n", temp.data, temp.nextoffset);
		fseek(file, DATABLOCK, SEEK_SET);
		fwrite(&temp, sizeof(struct DataBlock), 1, file);
		DATABLOCK = temp.nextoffset;
		fseek(file, DATABLOCK, SEEK_SET);
		fwrite(&d, sizeof(struct DataBlock), 1, file);
	}
	fclose(file);
}




void DeriveCommand(char *command, char *buffer)
{
	int cmd_offset = 0;
	for (int i = 1; buffer[i] != '$'; i++)
	{
		command[cmd_offset++] = buffer[i];
	}
	command[cmd_offset] = '\0';
}


//Blob objects

void SeparateUserData(char *command, char *commandname, char *username)
{
	int i;
	int offset = 0;
	for (i = 1; command[i] != '$'; i++);
	i++;
	for (; command[i] != '$'; i++)
	{
		commandname[offset++] = command[i];
	}
	commandname[offset] = '\0';
	i++;
	offset = 0;
	for (; command[i] != '$'&&command[i] != '\0'; i++)
	{
		username[offset++] = command[i];
	}
	username[offset] = '\0';
}

void ManageUsersData(char *command)
{
	char commandname[32];
	char username[32];
	SeparateUserData(command, commandname, username);
	printf("\n%s\t%s\n", commandname, username);
	struct UserData u;
	struct User user;
	FILE *file = fopen("blob.bin", "rb+");
	fseek(file, USERSOFFSET, SEEK_SET);
	fread(&u, sizeof(struct UserData), 1, file);
	int flag = 0;
	if (!strcmpi("adduser", commandname))
	{
		strcpy(user.username, username);
		user.files_offset = 0;
		u.users[u.count++] = user;
		CURRENT_USER = u.count - 1;
		fseek(file, USERSOFFSET, SEEK_SET);
		fwrite(&u, sizeof(struct UserData), 1, file);
		fclose(file);
		flag = 1;
	}
	else{
		fclose(file);
		for (int i = 0; i < u.count; i++)
		{
			user = u.users[i];
			if (!strcmp(user.username, username))
			{
				flag = 1;
				CURRENT_USER = i;
			}
		}
	}
	if (flag == 1)
	{
		strcpy(command, "@3\n\n\t1.Add File\n\t2.View Files$1$2#");
	}
	else{
		strcpy(command, "$User Not Found#");
	}

}


int AddFileToTheUser(char *filename,int usernumber)
{
	FILE *file = fopen("blob.bin", "rb+");
	fseek(file, USERSOFFSET, SEEK_SET);
	struct UserData u;
	struct User user;
	struct FilesCollection f;
	struct File binfile;
	fread(&u, sizeof(struct UserData), 1, file);
	user = u.users[usernumber];
	if (user.files_offset == 0)
	{
		user.files_offset = GiveFreeBlock();
		printf("\nuseroffset=%d\n", user.files_offset);
		strcpy(binfile.name, filename);
		binfile.offset = 0;
		DATABLOCK = 0;
		f.files_count = 0;
		f.files[f.files_count++] = binfile;
		CURRENTFILE = user.files_offset;
		CURRENTFILENUMBER = f.files_count - 1;
	}
	else{
		printf("\nuseroffset=%d\n", user.files_offset);
		fseek(file, user.files_offset, SEEK_SET);
		fread(&f, sizeof(struct FilesCollection), 1, file);
		strcpy(binfile.name, filename);
		binfile.offset = 0;
		DATABLOCK = binfile.offset;
		f.files[f.files_count++] = binfile;
		CURRENTFILE = user.files_offset;
		CURRENTFILENUMBER = f.files_count - 1;
	}
	u.users[usernumber] = user;
	fseek(file, USERSOFFSET, SEEK_SET);
	fwrite(&u, sizeof(struct UserData), 1, file);
	fseek(file, user.files_offset, SEEK_SET);
	fwrite(&f, sizeof(struct FilesCollection), 1, file);
	fclose(file);
	return 0;
}

int ViewFilesFromTheUser(int usernumber,char *command)
{
	FILE *file = fopen("blob.bin", "rb");
	fseek(file, USERSOFFSET, SEEK_SET);
	struct UserData u;
	fread(&u, sizeof(struct UserData), 1, file);
	struct User user = u.users[usernumber];
	int filesoffet = user.files_offset;
	printf("\nUsers Files offset=%d\n", user.files_offset);
	if (filesoffet == 0)
	{
		strcpy(command, "$No Files To Show#");
	}
	else{
		fseek(file, filesoffet, 0);
		struct FilesCollection f;
		fread(&f, sizeof(struct FilesCollection), 1, file);
		fclose(file);
		if (f.files_count <= 0)
		{
			strcpy(command, "$No Files To Display#");
			return 0;
		}
		printf("\n\n");
		char buffer[1024] = "@4\n\tFiles List\n\t";
		int i;
		for (i = 0; i < f.files_count; i++)
		{
			char temp[5];
			temp[0] = i+49;
			temp[1] = '\0';
			strcat(buffer, temp);
			strcat(buffer, " . ");
			strcat(buffer, f.files[i].name);
			strcat(buffer, "\n\t");
			printf("%s\n", f.files[i].name);
		}

		strcat(buffer, "$");
		char temp[5];
		temp[0] = 49; temp[1] = '$'; temp[2] = 49 + i; temp[3] = '\0';
		strcat(buffer, temp);
		strcat(buffer,"$#");
		strcpy(command, buffer);
		
	}
	return 0;
}


void DisplayFile()
{
	FILE *file = fopen("blob.bin", "rb");
	fseek(file, CURRENTFILE, SEEK_SET);
	struct FilesCollection f;
	struct File binfile;
	fread(&f, sizeof(struct FilesCollection), 1, file);
	binfile = f.files[CURRENTFILENUMBER];
	int offset = binfile.offset;
	
	while (offset > 0)
	{
		printf("\n\nIn Display function        %d\n\n", offset);
		struct DataBlock d;
		fseek(file, offset, SEEK_SET);
		fread(&d, sizeof(struct DataBlock), 1, file);
		printf("%s", d.data);
		offset = d.nextoffset;
	}
}

int ProcessFileNameInformation(char *command)
{
	int i;
	for (i = 1; command[i] != '$'; i++);
	char buffer[30];
	int buffer_offset = 0;
	i++;
	for (; command[i] != '$'; i++)
	{
		buffer[buffer_offset++] = command[i];
	}
	buffer[buffer_offset] = '\0';
	i++;
	if (!strcmp(buffer, "viewfiles"))
	{
		printf("\nCurrent user=%d\n", CURRENT_USER);
		ViewFilesFromTheUser(CURRENT_USER, command);
		printf("\n\nIn The Process file\n\n%s", command);
	}
	else{
		char filename[32];
		buffer_offset = 0;
		for (; command[i] != '\n'&&command[i] != '\0'; i++)
		{
			filename[buffer_offset++] = command[i];
		}
		filename[buffer_offset] = '\0';
		AddFileToTheUser(filename, CURRENT_USER);
		printf("\n\n%d,%s", CURRENT_USER, filename);
		strcpy(command, "$sendFile#");
	}
	return 0;
}


void DeleteFileFromDatabase(int fileno)
{
	FILE *file = fopen("blob.bin", "rb+");
	fseek(file, USERSOFFSET, SEEK_SET);
	struct UserData u;
	struct User user;
	fread(&u, sizeof(struct UserData), 1, file);
	user = u.users[CURRENT_USER];
	int filesoffset = user.files_offset;
	struct FilesCollection f;
	struct File binfile;
	fseek(file, filesoffset, SEEK_SET);
	fread(&f, sizeof(struct FilesCollection), 1, file);
	binfile = f.files[fileno];
	for (int i = fileno; i < f.files_count; i++)
	{
		f.files[i] = f.files[i + 1];
	}
	f.files_count--;
	fseek(file, filesoffset, SEEK_SET);
	fwrite(&f, sizeof(struct FilesCollection), 1, file);
	int offset = binfile.offset;
	struct DataBlock d;
	while (offset>0)
	{
		fseek(file, offset, SEEK_SET);
		fread(&d, sizeof(struct DataBlock), 1, file);
		FreeBlock(offset);
		offset = d.nextoffset;
	}
	fclose(file);
}


int DATABLOCKOFFSET;
void SendBlock(char *command)
{
	printf("\n\n%d\n\n", DATABLOCKOFFSET);
	
	if (DATABLOCKOFFSET <= 0)
	{
		strcpy(command, "end$$$#");
	}
	else{
		FILE *file = fopen("blob.bin", "rb");
		fseek(file, DATABLOCKOFFSET, SEEK_SET);
		struct DataBlock d;
		fread(&d, sizeof(struct DataBlock), 1, file);
		fclose(file);
		strcpy(command, d.data);
		strcat(command, "#");
		//printf("%s", command);
		DATABLOCKOFFSET = d.nextoffset;
	}
}



void DownloadProcessing(int fileno,char *command)
{
	FILE *file = fopen("blob.bin", "rb");
	fseek(file, USERSOFFSET, SEEK_SET);
	struct UserData u;
	fread(&u, sizeof(struct UserData), 1, file);
	struct User user = u.users[CURRENT_USER];
	struct FilesCollection f;
	fseek(file, user.files_offset, SEEK_SET);
	fread(&f, sizeof(struct FilesCollection), 1, file);
	struct File binfile = f.files[fileno];
	DATABLOCKOFFSET = binfile.offset;
	fclose(file);
	strcpy(command, "@5\nopenfile%");
	strcat(command, binfile.name);
	strcat(command, "$0$0$#");
	printf("\n%s\n", command);
}


void DoFileProcessingInDatabase(char *command)
{
	printf("%s", command);
	int i;
	for (i = 1; command[i] != '$'; i++);
	i++;
	char option[30];
	int j = 0;
	for (; command[i] != '$'; i++)
	{
		option[j++] = command[i];
	}
	option[j] = '\0';
	i++;
	int fileno = command[i] - '1';
	if (!strcmp("delete", option))
	{
		DeleteFileFromDatabase(fileno);
		strcpy(command,"$File Deleted From Database Successfully\n#");
	}
	else if (!strcmp("download", option))
	{
		DownloadProcessing(fileno, command);
	}
}
////////////////////////////////////////////////////////////////////////
////END OF BLOB FUNCTIONS///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
///////////////////START OF CALENDAR STORE//////////////////////////////////
////////////////////////////////////////////////////////////////////////////




void ProcessMainFunction(char *command)
{
	int i;
	for (i = 1; command[i] != '$'; i++);
	i++;
	int option = command[i] - '0';
	if (option == 1)
	{
		strcpy(command, "@2\n\tWelCome To Blob Store\n\t1.Login\n\t2.New User$1$2#");
	}
	else{
		strcpy(command, "@\n\tUnder Construction$0$0#");
	}
}

//Blob objects
int ProcessInputBuffer(char *recv_buff)
{
	if (recv_buff[0] != '$')
	{
		return 0;
	}
	else{
		char command[32];
		DeriveCommand(command, recv_buff);
		if (!strcmp(command, "opened"))
			return 1;
		else if (!strcmp(command, "filename"))
			return 2;
		else if (!strcmp(command, "login"))
			return 3;
		else if (!strcmp(command, "mainscreen"))
			return 4;
		else if (!strcmp(command, "files"))
			return 5;
		else if (!strcmp(command, "end"))
			return 6;
		else if (!strcmp(command, "filesoptions"))
			return 7;
		else if (!strcmp(command, "senddatablock"))
			return 8;
	}
	return -1;
}


void process_input(char *recvbuf, int recv_buf_cnt, int* csock) 
{
	
	char replybuf[1024]={'\0'};
	char buffer[1024];
	memset(&buffer, 0, 1024);
	int k=ProcessInputBuffer(recvbuf);
	if (k == 0)
	{
		WriteToTheFile(recvbuf);
	}
	else if (k == 1)
	{
		printf("Client Opened");
		memset(recvbuf, '\0', sizeof(recvbuf));
		char sendbuffer[256] = "@1\n\n\t1.Blob Store\n\n$1$1$#";
		strcpy(recvbuf, sendbuffer);
	}
	else if (k == 3)
	{
		ManageUsersData(recvbuf);
	}
	else if (k == 4)
	{
		ProcessMainFunction(recvbuf);
	}
	else if (k == 5)
	{
		ProcessFileNameInformation(recvbuf);
	}
	else if (k == 6)
	{
		printf("end block is called");
		strcpy(recvbuf, "$File Read Successfully#");
	}
	else if (k == 7)
	{
		DoFileProcessingInDatabase(recvbuf);
	}
	else if (k == 8)
	{
		SendBlock(recvbuf);
	}
	replyto_client(recvbuf, csock);
	replybuf[0] = '\0';
}


void replyto_client(char *buf, int *csock) {
	int bytecount;
	
	if((bytecount = send(*csock, buf, strlen(buf), 0))==SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		free (csock);
	}
	//printf("replied to client: %s\n",buf);
}

DWORD WINAPI SocketHandler(void* lp){
	int *csock = (int*)lp;

	char recvbuf[1024];
	int recvbuf_len = 1024;
	int recv_byte_cnt;

	memset(recvbuf, 0, recvbuf_len);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		free(csock);
		return 0;
	}
	process_input(recvbuf, recv_byte_cnt, csock);
	return 0;
}
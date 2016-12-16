//////////////////////////////////////////////////////////////////////////
//////File: Serv.cpp
//////Author: Hyman
//////Date: 2016/12/13
//////Description: ���̵߳�socket�����
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "winsock2.h"
#include "stdio.h"
#include "windows.h"
#include "process.h"
//����ws2_32.lib��
#pragma comment(lib,"ws2_32.lib")

#define  BUF_SIZE 30
#define THREAD_SIZE 20

void ErrorHandler(char* message);
unsigned WINAPI ClntHandler(void* param);
//��ſͻ���socket������
SOCKET clntSocks[THREAD_SIZE];
HANDLE mtx;
//�ܵĿͻ������ӵĸ���
int clntCnt=0;

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	SOCKET servSock,clntSock;
	SOCKADDR_IN servAddr,clntAddr;
	int clntAddrSz;
	
	//�洢�������߳̾��
	HANDLE hClntThreadHandles[THREAD_SIZE];
	
	//ͨ���������Ĳ�������˿ں�
	if(argc!=2)
	{
		printf("Usage %s <port>\n",argv[0]);
		exit(1);
	}
	if(WSAStartup(MAKEWORD(2,2),&wsaData)==SOCKET_ERROR)
		ErrorHandler("WSAStartup error");

	servSock=socket(AF_INET,SOCK_STREAM,0);
	if(servSock==INVALID_SOCKET)
		ErrorHandler("socket() error");

	memset(&servAddr,0,sizeof(servAddr));
	servAddr.sin_family=AF_INET;
	servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAddr.sin_port=htons(atoi(argv[1]));

	if(bind(servSock,(const sockaddr*)&servAddr,sizeof(servAddr))==SOCKET_ERROR)
		ErrorHandler("bind() error");

	listen(servSock,5);
	//�������⣬��ʼ״̬Ϊsignaled���������κ��߳�
	mtx=CreateMutex(NULL,FALSE,NULL);
	while(clntCnt<THREAD_SIZE)
	{
		clntAddrSz=sizeof(clntAddr);
		clntSock=accept(servSock,(sockaddr*)&clntAddr,&clntAddrSz);

		WaitForSingleObject(mtx,INFINITE);
		clntSocks[clntCnt]=clntSock;
		ReleaseMutex(mtx);
		//�����߳�
		hClntThreadHandles[clntCnt++]=(HANDLE)_beginthreadex(NULL,0,ClntHandler,&clntSock,0,NULL);
		
	}
	//�ȴ��߳̽���
	WaitForMultipleObjects(THREAD_SIZE,hClntThreadHandles,true,INFINITE);
	//�رջ�����
	CloseHandle(mtx);
	return 0;
}

//���ڴ�����Ϣ��ӡ
void ErrorHandler(char* message)
{
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
//����ͻ���socket��д���̺߳���
unsigned WINAPI ClntHandler(void* param)
{
	
	int i,strLen;
	char buf[BUF_SIZE];
	SOCKET clntSock=*((SOCKET*)param);
	while(1)
	{
		strLen = recv(clntSock,buf,BUF_SIZE,0);
		if(strLen<=0)
			break;
		send(clntSock,buf,strLen,0);
	}
	WaitForSingleObject(mtx,INFINITE);
	for(i=0;i<clntCnt;i++)
	{
		if(clntSocks[i]==clntSock)
		{
			while(i++<clntCnt-1)
				clntSocks[i]=clntSocks[i+1];
			break;
		}
		
	}
	clntCnt--;
	ReleaseMutex(mtx);
	closesocket(clntSock);
	return 0;
}



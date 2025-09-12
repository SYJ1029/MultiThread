#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>


#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

#define INITIALIZEWORD(mainVer, subVer) (WORD)(((BYTE)mainVer << (BYTE)8) + (BYTE)subVer)

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}




int main(int argc, char* argv[])
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	MAKEWORD(1, 2);

	if (WSAStartup(INITIALIZEWORD(2, 2), &wsa) != 0)
		return 1;
	printf("[�˸�] ���� �ʱ�ȭ ����\n");
	printf("%#x, %#x, %s, %s\n", wsa.wVersion, wsa.wHighVersion, wsa.szDescription, wsa.szSystemStatus);

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	printf("[�˸�] ���� ���� ����\n");

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
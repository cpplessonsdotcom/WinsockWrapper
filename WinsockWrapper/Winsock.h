#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma comment(lib, "Ws2_32.lib")

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <string>

class Winsock
{
public:
	Winsock();
	~Winsock();

	/// <summary>
	/// <para> Connect to a server </para>
	/// </summary>
	/// <params name="ip"> Connect to server ip </params>
	/// <params name="port"> Connect to server port </params>
	bool Connect(std::string ip, unsigned int port);

	/// <summary>
	/// <para> Disconnect from server </para>
	/// </summary>
	void Disconnect();

	/// <summary>
	/// <para> Start the socket server </para>
	/// </summary>
	/// <params name="port"> Server port </params>
	bool Start(unsigned int port);

	/// <summary>
	/// <para> Stop the server </para>
	/// </summary>
	void Stop();

	/// <summary>
	/// <para> Send message to server </para>
	/// </summary>
	/// <params name="message"> Message string </params>
	bool Send(std::string message);

	/// <summary>
	/// <para> Wait for message from client </para>
	/// </summary>
	bool Receive(std::string & message);

	std::string GetLastError();

private:
	std::string GetLastSocketError();

	WSADATA _wsaData;
	struct addrinfo *_result = NULL, *_ptr = NULL, _hints;
	SOCKET _connectSocket = INVALID_SOCKET; // For client
	SOCKET _listenSocket = INVALID_SOCKET; // For server
	SOCKET _clientSocket = INVALID_SOCKET; // For server
	static constexpr int _recvbuflen = 512;
	char _recvbuf[_recvbuflen];
	std::string _error;
	std::string _message;
};
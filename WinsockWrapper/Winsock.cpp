#include "Winsock.h"
#include <stdexcept>
#include <codecvt>

Winsock::Winsock()
{
	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0) {
		throw std::runtime_error("WSAStartup failed");
	}
}

Winsock::~Winsock()
{
	WSACleanup();
}

bool Winsock::Connect(std::string ip, unsigned int port)
{
	ZeroMemory(&_hints, sizeof(_hints));

	_hints.ai_family = AF_UNSPEC;
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	if (getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &_hints, &_result) != 0) {
		throw std::runtime_error("getaddrinfo failed");
	}

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	_ptr = _result;

	// Create a SOCKET for connecting to server
	_connectSocket = socket(_ptr->ai_family, _ptr->ai_socktype, _ptr->ai_protocol);

	if (_connectSocket == INVALID_SOCKET) {
		std::string error_message = "Error at socket(): ";
		error_message += WSAGetLastError();
		freeaddrinfo(_result);
		throw std::runtime_error(error_message);
	}

	// Attempt to connect to an address until one succeeds
	for (_ptr = _result; _ptr != NULL; _ptr = _ptr->ai_next) {

		// Create a SOCKET for connecting to server
		_connectSocket = socket(_ptr->ai_family, _ptr->ai_socktype, _ptr->ai_protocol);
		if (_connectSocket == INVALID_SOCKET) {
			//printf("socket failed with error: %ld\n", WSAGetLastError());
			return false;
		}

		// Connect to server.
		if (connect(_connectSocket, _ptr->ai_addr, (int)_ptr->ai_addrlen) == SOCKET_ERROR) {
			closesocket(_connectSocket);
			_connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(_result);

	if (_connectSocket == INVALID_SOCKET) {
		//printf("Unable to connect to server!\n");
		return false;
	}

	return true;
}

void Winsock::Disconnect()
{
	// shutdown the connection since no more data will be sent
	if (shutdown(_connectSocket, SD_SEND) == SOCKET_ERROR) {
		std::string error_message = "shutdown failed with error: ";
		error_message += GetLastSocketError();
		closesocket(_connectSocket);
		throw std::runtime_error(error_message);
	}

	int bytesReceived = 0;

	// Receive until the peer closes the connection
	do {

		bytesReceived = recv(_connectSocket, _recvbuf, _recvbuflen, 0);
		if (bytesReceived > 0)
			_message = "Bytes received: " + std::to_string(bytesReceived);
		else if (bytesReceived == 0)
			_message = "Connection closed";
		else
			_error = "recv failed with error: " + GetLastSocketError();

	} while (bytesReceived > 0);

	// cleanup
	closesocket(_connectSocket);
}

bool Winsock::Start(unsigned int port)
{
	ZeroMemory(&_hints, sizeof(_hints));

	_hints.ai_family = AF_INET;
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_protocol = IPPROTO_TCP;
	_hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	if (getaddrinfo(NULL, std::to_string(port).c_str(), &_hints, &_result) != 0) {
		throw std::runtime_error("getaddrinfo failed");
	}

	// Create a SOCKET for the server to listen for client connections
	_listenSocket = socket(_result->ai_family, _result->ai_socktype, _result->ai_protocol);

	if (_listenSocket == INVALID_SOCKET) {
		std::string error_message = "Error at socket(): " + GetLastSocketError();
		freeaddrinfo(_result);
		throw std::runtime_error(error_message);
	}

	//unsigned long timeout = 5000;
	//setsockopt(_listenSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

	// Setup the TCP listening socket
	if (bind(_listenSocket, _result->ai_addr, (int)_result->ai_addrlen) == SOCKET_ERROR) {
		_error = "bind failed with error: " + GetLastSocketError();
		freeaddrinfo(_result);
		closesocket(_listenSocket);
		return false;
	}

	freeaddrinfo(_result);

	if (listen(_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		_error = "listen failed with error: " + GetLastSocketError();
		closesocket(_listenSocket);
		return false;
	}

	return true;
}

void Winsock::Stop()
{
	// shutdown the connection since we're done
	if (_clientSocket != INVALID_SOCKET)
	{
		if (shutdown(_clientSocket, SD_SEND) == SOCKET_ERROR) {
			std::string error_message = "shutdown failed with error: " + GetLastSocketError();
			closesocket(_clientSocket);
			throw std::runtime_error(error_message);
		}
	}

	// cleanup
	closesocket(_clientSocket);
}

bool Winsock::Send(std::string message)
{
	if (send(_connectSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
		_message = "send failed with error: " + GetLastSocketError();
		closesocket(_connectSocket);
		return false;
	}

	return true;
}

bool Winsock::Receive(std::string & message)
{
	// Accept a client socket
	_clientSocket = accept(_listenSocket, NULL, NULL);
	if (_clientSocket == INVALID_SOCKET) {
		_message = "accept failed with error: " + GetLastSocketError();
		closesocket(_listenSocket);
		return false;
	}

	// No longer need server socket
	closesocket(_listenSocket);

	// Client has now connected to server

	int bytesReceived = 0;

	// Receive until the peer shuts down the connection
	do {

		bytesReceived = recv(_clientSocket, _recvbuf, _recvbuflen, 0);
		if (bytesReceived > 0) {
			_message = "Bytes received: " + bytesReceived;
			message += _recvbuf;
		}
		else if (bytesReceived == 0)
			_message = "Connection closing...";
		else {
			_error = "recv failed with error: " + GetLastSocketError();
			closesocket(_clientSocket);
			return false;
		}

	} while (bytesReceived > 0);

	return true;
}

std::string Winsock::GetLastError()
{
	return _error;
}

std::string Winsock::GetLastSocketError()
{
	wchar_t *s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, WSAGetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPWSTR)&s, 0, NULL);
	std::string converted_str = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s);
	LocalFree(s);
	return converted_str;
}
#include "Winsock.h"
#include <iostream>

#define SOCKET_CLIENT

int main()
{
	Winsock sock;
#ifndef SOCKET_CLIENT
	bool started = sock.Start(8080);
	if (!started)
	{
		std::puts("Starting server failed!\n");
		std::puts(sock.GetLastError().c_str());
		std::puts("Press ENTER to exit program.");
		std::cin.get();
		return 0;
	}
	std::puts("Server is now started.\nPress ENTER to wait for client to connect.\n");
	std::cin.get();
	std::puts("Waiting for client to connect...\n");
	std::string message_from_client;
	bool message_received = sock.Receive(message_from_client);
	if (message_received)
	{
		std::puts(message_from_client.c_str());
		std::puts("\n");
	}
	std::puts("Press ENTER to stop server and exit.\n");
	std::cin.get();
	sock.Stop();
#else
	bool connected = sock.Connect("127.0.0.1", 8080);
	if (!connected)
	{
		std::puts("Connecting to server failed!\n");
		std::puts(sock.GetLastError().c_str());
		std::puts("Press ENTER to exit program.");
		std::cin.get();
		return 0;
	}
	std::puts("Client is now connected to server.\nPress ENTER to send message to server.\n");
	std::cin.get();
	bool send_success = sock.Send("Test message");
	if (!send_success)
	{
		std::puts("Send to server failed.\n");
		std::puts(sock.GetLastError().c_str());
		std::puts("Press ENTER to exit program.");
		std::cin.get();
		return 0;
	}
	std::puts("Message is now sent to server.\n");
	std::puts("Press ENTER to disconnect and exit program.");
	std::cin.get();
	sock.Disconnect();
#endif
	return 0;
}
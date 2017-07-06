#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

const int DEFAULT_SIZE = 128; //Default buffer size 

// Enumumeration for socket type
enum SocketType {
	CLIENT, SERVER
};

// Enumeration for Connection type
enum ConnectionType {
	TCP, UDP
};

class winsock {
protected:
	int version_num1, version_num2; // Determines the WSADATA version numbers
	WSADATA wsa_data;
public:
	void start_DLLS();
	winsock();
};

class MySocket : public winsock {
private: 
	char * Buffer;					// Dynamically allocated RAW buffer for communication
	SOCKET WelcomeSocket;			// Socket used to configuer as TCP/IP server
	SOCKET ConnectionSocket;		// Socket for client/server communication (both TCP/UDP)
	struct sockaddr_in SvrAddr;		// struct to store connection information 
	SocketType mySocket;			// Specifies the type of socket MySocket object is initialized to 
	std::string IPAddr;				// Holds the IPv4 IP address string
	int Port;						// Holds the port number 
	ConnectionType connectionType;	// Defines the Transport Layer protocol being used (TCP/UDP)
	bool bTCPConnect;				// Boolean flag to determine if connection has been established or not
	int MaxSize;	
	SOCKET initialize_socket();
	void bind_socket();
	void listen_socket();// Store the maximum number of bytes the buffer is allocated

public:
	MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);		// Constructor to configure the socket and connection types
	~MySocket();						// Destructor to clean up all dynamically allocated memory 
	void ConnectTCP();					// Enables TCP/IP socket connection (3-way handshake)
	void DisconnectTCP();				// Disconnects an established TCP/IP socket connection (4-way handshake)
	void SendData(const char*, int);	// Transmits a block pf RAW data stored in the internal MySocket Buffer
	int GetData(char*);				// Receives the last block of RAW data stored in the internal MySocket Buffer
	std::string GetIPAddr();			// Returns the IP address within the MySocket object
	void SetIpAddr(std::string);		// Changes the default IP address within the MySocket object
	void SetPort(int);					// Changes the default Port number within the MySocket object
	int GetPort();						// Returns the Port number configured within the MySocket object
	SocketType GetType();				// Returns the default SocketType the MySocket object is configured as 
	void SetType(SocketType);			// Changes the default SocketType within the MySocket object
};

#endif // !MYSOCKET_H

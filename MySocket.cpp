#include "MySocket.h"

void winsock::start_DLLS() 
{
	if ((WSAStartup(MAKEWORD(this->version_num1, this->version_num2), &this->wsa_data)) != 0) {
		std::cout << "Could not start DLLs" << std::endl;
		std::cin.get();
		exit(0);
	}
}

// Base constructor (called for all objects)
// note that in case multiple socket objects are created, start_DLLS()
// is called multiple times. This, however, does not result in errors
// just a slight redundancy
winsock::winsock() 
{
	this->version_num1 = 2;
	this->version_num2 = 2;
	this->start_DLLS();
}

// Initialize a socket and returns it
SOCKET MySocket::initialize_socket() 
{
	int type;
	int protocol;

	switch (connectionType)
	{
	case TCP: 
		type = SOCK_STREAM;
		protocol = IPPROTO_TCP;
		break;
	case UDP:
		type = SOCK_DGRAM;
		protocol = IPPROTO_UDP;
		break;
	default:
		std::cout << "Invalid Connection type!" << std::endl;
		break;
	}

	SOCKET LocalSocket = socket(AF_INET, type, protocol);
	if (LocalSocket == INVALID_SOCKET) {
		WSACleanup();
		std::cout << "Could not initialize socket!" << std::endl;
		std::cin.get();
		exit(0);
	}
	return LocalSocket;
}

//binds the server socket
void MySocket::bind_socket() 
{
	if ((bind(this->WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
		closesocket(this->WelcomeSocket);
		WSACleanup();
		std::cout << "Could not bind to the socket!" << std::endl;
		std::cin.get();
		exit(0);
	}
}

///puts the server socket in listening mode
void MySocket::listen_socket() 
{
	if (listen(this->WelcomeSocket, 1) == SOCKET_ERROR) {
		closesocket(this->WelcomeSocket);
		WSACleanup();
		std::cout << "Could not listen to the provided socket!" << std::endl;
		std::cin.get();
		exit(0);
	}
	else {
		std::cout << "Waiting for client connection" << std::endl;
	}
}

// Constructor
// Configures the socket and connection types, sets the IP Address and Port Number and
// dynamically allocates memory for the Buffer.Note that the constructor should put servers in
// conditions to either accept connections(if TCP), or to receive messages(if UDP).
MySocket::MySocket(SocketType socType, std::string ip, unsigned int port, ConnectionType connType, unsigned int bufferSize)
{
	mySocket = socType;
	IPAddr = ip;
	Port = port;
	connectionType = connType;
	bTCPConnect = false;
	MaxSize = bufferSize;
	Buffer = new char[MaxSize];
	SvrAddr.sin_family = AF_INET; //Address family type internet
	SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
	SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address

	
	if (mySocket == SERVER) {
		WelcomeSocket = initialize_socket(); // Initializes socket to wither UDP or TCP connection based on connectionType
		bind_socket();
		if (connectionType == TCP) {
			listen_socket();
			ConnectTCP();
		}
	}
	if (mySocket == CLIENT) {
		ConnectionSocket = initialize_socket();// Initializes socket to wither UDP or TCP connection based on connectionType
	}
}

// Destructor 
// Cleans up all dynamically allocated memory 
MySocket::~MySocket()
{
	delete[] Buffer;
}

// Enables TCP/IP socket connection (3-way handshake)
void MySocket::ConnectTCP()
{
	if (mySocket == SERVER) {
		if ((this->ConnectionSocket = accept(this->WelcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
			bTCPConnect = false;
			closesocket(this->WelcomeSocket);
			WSACleanup();
			std::cout << "Could not accept incoming connection." << std::endl;
			std::cin.get();
			exit(0);
		}
		else {
			std::cout << "Connection Accepted" << std::endl;
			bTCPConnect = true;
		}
	}
	else if(mySocket == CLIENT) {
		std::cout << "Trying to connect to the server" << std::endl;
		while (!bTCPConnect) {
			if ((connect(this->ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
			else {
				std::cout << "Connection Established" << std::endl;
				bTCPConnect = true;
			}
		}
	}
}

// Disconnects an established TCP/IP socket connection (4-way handshake)
void MySocket::DisconnectTCP()
{
	if (mySocket == SERVER) {
		closesocket(ConnectionSocket);
		closesocket(WelcomeSocket);
		WSACleanup();
		if (connectionType == UDP) {
			closesocket(ConnectionSocket);
			WSACleanup();
		}
	}
	else if (mySocket == CLIENT) {
		closesocket(ConnectionSocket);
		WSACleanup();
	}
}

// Transmits a block pf RAW data stored in the internal MySocket Buffer
void MySocket::SendData(const char* buffer, int size)
{
	if (connectionType == TCP) {
		if (mySocket == SERVER) {
			send(ConnectionSocket, buffer, size, 0);
		}
		else if (mySocket == CLIENT) {
			send(ConnectionSocket, buffer, size, 0);
		}
	}
	else if (connectionType == UDP) {
		(sendto(ConnectionSocket, buffer, size, 0,
			(struct sockaddr *)&SvrAddr, sizeof(SvrAddr)));
	}
}

// Receives the last block of RAW data stored in the internal MySocket Buffer
int MySocket::GetData(char* buffer)
{
	int size = 0;
	memset(Buffer, 0, MaxSize);
	if (connectionType == TCP) {
		if (mySocket == SERVER) {
			size = recv(ConnectionSocket, Buffer, MaxSize, 0);
		}
		else if (mySocket == CLIENT) {
			size = recv(ConnectionSocket, Buffer, MaxSize, 0);
		}
	}
	else if (connectionType == UDP) {
		int SvrAddrLength = sizeof(SvrAddr);
		size = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr *)&SvrAddr, &SvrAddrLength);
	}
	memcpy(buffer, Buffer, MaxSize);
	return  size;
}

// Returns the IP address within the MySocket object
std::string MySocket::GetIPAddr()
{
	return IPAddr;
}

// Changes the default IP address within the MySocket object
void MySocket::SetIpAddr(std::string ip) {
	if (!bTCPConnect) {
		IPAddr = ip;
		SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
	}
}

// Changes the default Port number within the MySocket object
void MySocket::SetPort(int port)
{
	if (!bTCPConnect) {
		Port = port;
		SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
	}
}

// Returns the Port number configured within the MySocket object
int MySocket::GetPort()
{
	return Port;
}

// Returns the default SocketType the MySocket object is configured as 
SocketType MySocket::GetType()
{
	return mySocket;
}

// Changes the default SocketType within the MySocket object
void MySocket::SetType(SocketType type)
{
	if (!bTCPConnect) {
		mySocket = type;
	}
}
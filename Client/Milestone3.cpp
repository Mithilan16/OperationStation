#include "../Pkt_Def.h"
#include "../MySocket.h"
#include <string.h >
#include <iostream>
#include <iomanip>

bool ExeComplete = false;

bool checkInput(int userInput) {
	if (userInput >=1 && userInput <=5)
		return true;
	else {
		std::cout << "Invalid command. Please try again" << std::endl;
		return false;
	}
}

void commandThread(std::string ip, int port) {

	MySocket commandSocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, 100);
	commandSocket.ConnectTCP();

	PktDef commandPacket;
	MotorBody mBody;

	while (ExeComplete == false) {
		int userCommand = 0, userDirection = 0, userDuration = 0;

		//Get and Set values for command and motorbody
		bool validInput = false;
		do {
			std::cout << "DRIVE: 1\nSLEEP: 3\nARM: 4\nCLAW: 5" << std::endl;
			std::cout << "Enter Command: ";
			std::cin >> userCommand;
			validInput = checkInput(userCommand);
		}while(!validInput);
		
		std::cin.ignore();
		commandPacket.SetCmd((CmdType)userCommand);

		CmdType cmd = commandPacket.GetCmd();
		if (cmd != SLEEP) {
			if (cmd == DRIVE) {
				std::cout << "FORWARD: 1\nBACKWARD: 2\nRIGHT: 3\nLEFT: 4" << std::endl;
			}

			else if (cmd == ARM) {
				std::cout << "UP: 5\nDOWN : 6" << std::endl;
			}

			else if (cmd == CLAW) {
				std::cout << "OPEN: 7\nCLOSE 8" << std::endl;
			}

			std::cout << "Enter Direction:";
			std::cin >> userDirection;
			std::cin.ignore();

			if (cmd == DRIVE) {
				std::cout << "Enter Duration: ";
				std::cin >> userDuration;
				std::cin.ignore();
			}

			//set MotorBody attributes
			mBody.Direction = (char)userDirection;
			mBody.Duration = (char)userDuration;
		}
		//setBody in commandPacket,increment packet count,calcCRC
		commandPacket.SetBodyData((char*)&mBody, sizeof(mBody));
		commandPacket.SetPktCount(commandPacket.GetPktCount() + 1);
		commandPacket.CalcCRC();

		//send packet
		char* txbuffer = new char[(int)commandPacket.GetLength() + 2];
		txbuffer = commandPacket.GenPacket();

		//receive acknowldgement packet rawdata
		commandSocket.SendData(txbuffer, commandPacket.GetLength());

		//recive ack raw data create ackPacket
		char rxbuffer[128];
		commandSocket.GetData(rxbuffer);
		PktDef ackPacket(rxbuffer);

		if ((commandPacket.GetCmd() == SLEEP) && (ackPacket.GetCmd() == SLEEP)) {
			std::cout << "Robot Entering Sleep Mode...Disconnecting" << std::endl;
			ExeComplete = true;
			commandSocket.DisconnectTCP();
		}

		else if ((commandPacket.GetCmd() == ackPacket.GetCmd()) && ackPacket.GetAck()) {
			std::cout << "Command Id: " <<
				ackPacket.GetCmd() << " Received Sucessfully!" << std::endl;
		}

		else if ((ackPacket.GetCmd() == NOTSET) && ackPacket.GetAck() == false) {
			std::cout << "Command Rejected. Try Again!" << std::endl;
		}
	}
}

void TelemtryThread(std::string ip, int port) {
	MySocket telemetrySocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, 100);
	telemetrySocket.ConnectTCP();

	char rxbuffer[100];
	while (ExeComplete == false) {

		memset(rxbuffer, 0, sizeof(rxbuffer));
		int size = telemetrySocket.GetData(rxbuffer);
		PktDef telemetryPacket(rxbuffer);

		if (telemetryPacket.CheckCRC(rxbuffer, size) == true) {//CRC Check
			if (telemetryPacket.GetCmd() == STATUS) {//Check if StatusBit is set

				unsigned char* rawDataPtr = (unsigned char*)rxbuffer;
				std::cout << "Receiving Telemetry Data" << std::endl
					<< "=================================" << std::endl;
				std::cout <<
					std::hex <<           // output in hex
					std::setw(2) <<       // each byte prints as two characters
					std::setfill('0');  // fill with 0 if not enough characters
										//Print out Raw Data
				for (int i = 0; i < size; i++) {
					std::cout << static_cast<unsigned int>(*rawDataPtr++) << ", ";
				}
				std::cout << std::endl << std::dec;

				short* sensorPtr = (short*)telemetryPacket.GetBodyData();
				std::cout << "Sonar Data: " << (short)*sensorPtr++ << std::endl;
				std::cout << "Arm Data :" << (short)*sensorPtr++ << std::endl;
				char * bitPtr = (char*)sensorPtr;
				std::cout << "Drive Bit: " << (bool)((*bitPtr) & 0x01) << std::endl; //display drive bit

				//Check Arm Status
				if ((*bitPtr >> 1) & 0x01) {
					std::cout << "Arm is Up" << std::endl;
				}
				else if ((*bitPtr >> 2 & 0x01)) {
					std::cout << "Arm is Down" << std::endl;
				}

				//Check Claw Status
				if ((*bitPtr >> 3) & 0x01) {
					std::cout << "Claw is Open" << std::endl;
				}
				else if ((*bitPtr >> 4) & 0x01) {
					std::cout << "Claw is Closed" << std::endl << std::endl;
				}
			}
			else {
				std::cout << "Status Bit Not Set. Invalid Telemetry Packet" << std::endl;
			}	
		}
		else {
			std::cout << "CRC Error" << std::endl;
		}
	}
}

int main(int argc, char *argv[]) {

	std::string ip_addr = "127.0.0.1";
	int cmdPort = 27000;
	int telePort = 27501;

	std::cout << "Enter IP Address: ";
	std::getline(std::cin, ip_addr);

	std::cout << "Enter Command Port: ";
	std::cin >> cmdPort;
	std::cin.ignore();

	std::cout << "Enter Telemetry Port: ";
	std::cin >> telePort;
	std::cin.ignore();

	std::thread(commandThread,  ip_addr,  cmdPort) .detach();
	std::thread(TelemtryThread, ip_addr, telePort).detach();
	while (!ExeComplete) { };

}

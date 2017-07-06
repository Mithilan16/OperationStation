#include "Pkt_Def.h"
#include <stdio.h>

// A default constructor that places the PktDef object in a safe state
PktDef::PktDef() {
	//header
	mypkt.header.PktCount = 0; // Packet count
							   //Bit fields
	mypkt.header.Drive = 0;
	mypkt.header.Status = 0;
	mypkt.header.Sleep = 0;
	mypkt.header.Arm = 0;
	mypkt.header.Claw = 0;
	mypkt.header.Ack = 0;
	mypkt.header.Padding = 0;
	mypkt.header.Length = 0;
	//Body - motorbody
	mypkt.Data = nullptr;
	// tail
	mypkt.CRC = 0;

	//Rawbuffer
	RawBuffer = nullptr;
}

// An overloaded constructor that takes a RAW data buffer
// parses the data and populates the Header, Body, and CRC contents of the PktDef object
PktDef::PktDef(char *rawDataBuffer) {
	/**
	*  First extract, then check CheckCRC, if false use default constructor
	*/
	char *ptr = rawDataBuffer;

	int dataLength = sizeof(int);
	memcpy(&mypkt.header.PktCount, ptr, dataLength);

	ptr += dataLength;//move pointer to start of bitfield

	mypkt.header.Drive = (*ptr) & 0x01;
	mypkt.header.Status = (*ptr >> 1) & 0x01;
	mypkt.header.Sleep = (*ptr >> 2) & 0x01;
	mypkt.header.Arm = (*ptr >> 3) & 0x01;
	mypkt.header.Claw = (*ptr >> 4) & 0x01;
	mypkt.header.Ack = (*ptr >> 5) & 0x01;

	ptr += sizeof(char); // move pointer to over size of BitField (1 Byte) to start of length(char)

	//set length
	memcpy(&mypkt.header.Length, ptr, sizeof(char));

	ptr += sizeof(char);// move pointer over size of length(1 Byte) to start of Data(char*)

	int MbodySize = mypkt.header.Length - HEADERSIZE - sizeof(char);//calculate size of MotorBody

	//set Body
	SetBodyData(ptr, MbodySize);
	ptr += MbodySize; // move Pointer over size of Data(2Bytes)

	memcpy(&mypkt.CRC, ptr, sizeof(char));//set CRC

	/*if (CheckCRC(rawDataBuffer, mypkt.header.Length - sizeof(char)) == false) {
		std::cout << "CRC Error" << std::endl;
	}*/
}

// a set function that takes a pointer to a RAW data buffer and the size of the buffer in bytes
void PktDef::SetBodyData(char *rawDataPointer, int sizeInBytes) {
	mypkt.Data = new char[sizeInBytes]; // Allocate memory to Motor Body 
	if(sizeInBytes > 0) {
		for (int i = 0; i < sizeInBytes; i++) {
			mypkt.Data[i] = *(rawDataPointer++); // Assign value and increment pointer	
		}
	}
	else {
		mypkt.Data = nullptr; 
	}
	mypkt.header.Length = (char)(sizeInBytes + HEADERSIZE + sizeof(char));
}

// a set function that sets the objects PktCount header variable
void PktDef::SetPktCount(int count) {
	mypkt.header.PktCount = count;
}

// a query function that returns the PktCount value
int PktDef::GetPktCount() {
	return mypkt.header.PktCount;
}

// A set function that sets the packets command flag based on the CmdType
// 
void PktDef::SetCmd(CmdType cmd) {
	//cmdType = cmd;
	switch (cmd)
	{
	case NOTSET:
		mypkt.header.Drive = 0;
		mypkt.header.Status = 0;
		mypkt.header.Sleep = 0;
		mypkt.header.Arm = 0;
		mypkt.header.Claw = 0;
		mypkt.header.Ack = 0;
		break;
	case DRIVE:
		mypkt.header.Drive = 1;
		mypkt.header.Status = 0;
		mypkt.header.Sleep = 0;
		mypkt.header.Arm = 0;
		mypkt.header.Claw = 0;
		mypkt.header.Ack = 0;
		break;
	case STATUS:
		mypkt.header.Drive = 0;
		mypkt.header.Status = 1;
		mypkt.header.Sleep = 0;
		mypkt.header.Arm = 0;
		mypkt.header.Claw = 0;
		mypkt.header.Ack = 0;
		break;
	case SLEEP:
		mypkt.header.Drive = 0;
		mypkt.header.Status = 0;
		mypkt.header.Sleep = 1;
		mypkt.header.Arm = 0;
		mypkt.header.Claw = 0;
		mypkt.header.Ack = 0;
		break;
	case ARM:
		mypkt.header.Drive = 0;
		mypkt.header.Status = 0;
		mypkt.header.Sleep = 0;
		mypkt.header.Arm = 1;
		mypkt.header.Claw = 0;
		mypkt.header.Ack = 0;
		break;
	case CLAW:
		mypkt.header.Drive = 0;
		mypkt.header.Status = 0;
		mypkt.header.Sleep = 0;
		mypkt.header.Arm = 0;
		mypkt.header.Claw = 1;
		mypkt.header.Ack = 0;
		break;
	case ACK:
		mypkt.header.Ack = 1;
		break;
	default:
		std::cout << "Invalid Command Type, Please Try Again!" << std::endl;
		break;
	}
}

// a query function that returns the CmdType based on the set command flag bit
CmdType PktDef::GetCmd() {
	char * ptr = (char *)&this->mypkt; //pointer to point to the cammand packet
	ptr += sizeof(int); // move pointer beyond the pktcount

	// check the command bit set nand return the CmdType
	if ((*ptr) & 0x01)
	{
		return DRIVE;
	}
	else if ((*ptr >> 1) & 0x01)
	{
		return STATUS;
	}
	else if ((*ptr >> 2) & 0x01)
	{
		return SLEEP;
	}
	else if ((*ptr >> 3) & 0x01)
	{
		return ARM;
	}
	else if ((*ptr >> 4) & 0x01)
	{
		return CLAW;
	}
	else {
		return NOTSET;
	}
}

// a query function that returns True / False based on the Ack flag in the header
bool PktDef::GetAck() {
	bool retVal = false;

	if (mypkt.header.Ack & 0x01)
	{
		retVal = true;
	}
	return retVal;
}

// a query function that returns the packet Length in bytes
int PktDef::GetLength() {
	return (int)mypkt.header.Length;
}

// a query function that returns a pointer to the objects Body field
char *PktDef::GetBodyData() {
	char * ptr = (char*)mypkt.Data;
	return ptr;
}

// a function that takes a pointer to a RAW data buffer, the size of the buffer in bytes, and calculates the CRC.
bool PktDef::CheckCRC(char *dataToTest, int bufferInBytes) {
	//CalcCRC();//CRC is set in one argument constructor
	int counter = 0;
	bool match = false;
	for (int i = 0; i < bufferInBytes - 1; i++)//iterate through each byte of Raw Data
	{
		for (int j = 0; j < 8; j++) { //iterate through each bit in a single byte
			counter += (*dataToTest >> j) & 0x01; //increment counter by 1 if bit is 1
		}
		dataToTest++;
	}

	if (counter == (int)mypkt.CRC)
	{
		match = true;
	}
	return match;
}

// a function that calculates the CRC and sets the objects packet CRC parameter.
void PktDef::CalcCRC() {
	char *ptr = GenPacket(); // pointer to Raw Data Buffer
	int count = 0;

	for (int i = 0; i < (int)mypkt.header.Length - 1; i++)//Iterate through each byte of Raw
	{
		for (int j = 0; j < 8; j++)//iterate through each bit in a single byte
		{
			count += (*ptr >> j) & 0x01;//increment count by 1 if bit is 1
		}
		ptr++;
	}
	mypkt.CRC = (char)count;
}

// a function that allocates the private RawBuffer
// transfers the contents from the objects member variables into a RAW data packet(RawBuffer) for transmission.
// The address of the allocated RawBuffer is returned.
char* PktDef::GenPacket()
{
	// allocate memory for the private RawBuffer
	RawBuffer = new char[mypkt.header.Length];
	char* ptr = RawBuffer;

	// copy over header into address of raw buffer
	memcpy(ptr, &mypkt.header, HEADERSIZE);
	ptr += HEADERSIZE;

	// copy over data into address of raw buffer
	char* auxPtr = mypkt.Data;
	int dataSize = mypkt.header.Length - HEADERSIZE - sizeof(char);//calculte size of the Body

	for (int i = 0; i < dataSize; i++) {
		*ptr++ = *(auxPtr++);//copy Body Element to RawBuffer
	}
	memcpy(ptr, &mypkt.CRC, sizeof(char));
	
	return RawBuffer;
}
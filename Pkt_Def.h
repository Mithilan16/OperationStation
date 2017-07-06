#ifndef PKT_DEF_H
#define PKT_DEF_H

#include <iostream>

#define FORWARD 1;
#define BACKWARD 2;
#define RIGHT 3;
#define LEFT 4;
#define UP 5;
#define DOWN 6;
#define OPEN 7;
#define CLOSE 8;
const int HEADERSIZE = 6;

struct Header {
    unsigned int PktCount;
    unsigned char Drive : 1;
    unsigned char Status : 1;
    unsigned char Sleep : 1;
    unsigned char Arm : 1;
    unsigned char Claw : 1;
    unsigned char Ack : 1;
    unsigned char Padding : 2;           //padding
    unsigned char Length;
};

struct MotorBody {
    unsigned char Direction;
    unsigned char Duration;
};

enum CmdType {
   NOTSET,DRIVE, STATUS, SLEEP, ARM, CLAW, ACK
};

class PktDef {
private:
    struct CmdPacket {
        Header header;
        char *Data;
        char CRC;
    } mypkt;

    char *RawBuffer;

public:
    PktDef();                        // A default constructor that places the PktDef object in a safe state
    PktDef(char *);                    // Constructor that takes a RAW data buffer, parses the data and populates the Header, Body, and CRC contents of the PktDef object.
    void SetCmd(CmdType);            // A set function that sets the packets command flag based on the CmdType
    void SetBodyData(char *, int);    // a set function that takes a pointer to a RAW data buffer and the size of the buffer in bytes.
    void SetPktCount(int);            // a set function that sets the objects PktCount header variable
    CmdType GetCmd();                // a query function that returns the CmdType based on the set command flag bit
    bool GetAck();                    // a query function that returns True / False based on the Ack flag in the header
    int GetLength();                // a query function that returns the packet Length in bytes
    char *GetBodyData();            // a query function that returns a pointer to the objects Body field
    int GetPktCount();                // a query function that returns the PktCount value
    bool CheckCRC(char *, int);        // a function that takes a pointer to a RAW data buffer, the size of the buffer in bytes, and calculates the CRC.
    void CalcCRC();            // a function that calculates the CRC and sets the objects packet CRC parameter.
    char *GenPacket();                // allocates the private RawBuffer and transfers the contents from the objects member variables into (RawBuffer)
};

#endif // !PKT_DEF_H
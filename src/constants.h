#ifndef __CONSTANTS_H
#define __CONSTANTS_H

//all given in terms of octets

//for header
#define HEADER_SIZE 4 //header packet
#define OPCODE_SIZE 2 //opcode in header packet
#define MESSAGE_SIZE 2 //contains message length in header packet

//for entries
#define ENTRY_SIZE 36 //length of each entry in contact list
#define ENTRY_NICKNAME_SIZE 15
#define ENTRY_ADDRESS_SIZE 4
#define ENTRY_PORT_SIZE 2
#define ENTRY_KEY_SIZE 15

//general information
//max number of bytes of info that can be received or sent, not including '\0'
#define CHAT_MESSAGE_MAX 254
//Maximum number of sockets pending on listen()
#define MAXPENDINGTCP 30
//Maximum number of sockets you can talk to simultaneously
#define MAXCLIENTS 55

//1 for very verbose messages in debug window
#define VERBOSE 0

#endif

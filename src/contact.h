//sample looping
//struct contact* c;
//for(contact_get_first(&c); c != NULL; contact_get_next(&c));

#ifndef __CONTACT_H
#define __CONTACT_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "constants.h"
#include "chat_packet_message.h"
#include "contact.h"
#include "chat_event.h"

struct contact {
	//only directly modify these four fields below
	char *c_nickname;
	char *c_address;
	char *c_key;
	unsigned short c_port;
	unsigned short c_listening_port;

	//only access these but do not modify these
	unsigned short c_states[NR_CHAT_EVENTS];

	//do not publicly use variables below this line; only for private use
	
	struct contact* contact_next;
	struct contact* contact_prev;

	int c_tcp_sock;
	struct sockaddr_in c_sockaddr;

	struct chat_packet_message cpm;
	struct chat_state* c_state;
};

//adds a contact to local contact folder
//returns 1 on success
int contact_add_basic(struct contact** c);

// *c points to first contact after call
// *c is NULL otherwise
void contact_get_first(struct contact** c);

// *c points to last contact after call
// *c is NULL otherwise
void contact_get_last(struct contact** c);

//returns nothing on success or failure
void contact_delete(struct contact* c);

// *c points to NULL if no next exists
void contact_get_next(struct contact** c);

// *c points to NULL if no previous contact exists
void contact_get_prev(struct contact** c);

//reads local contacts folder to get list of friends
//returns 1 on success; 0 on failure
//you must pass the location of folder( "." is pwd)
int contact_read(char* loc);

//writes local contacts folder so can be loaded later
//you must pass the location of folder( "." is pwd)
void contact_write(char* loc);

//returns size of contact list
int contact_size();

//returns 1 if c is first element
int contact_is_first(struct contact * c);

//returns 1 if c is last element
int contact_is_last(struct contact * c);

//call this when a contact socket has been changed, whether 
//disconnected or new message received or otherwise
//returns 1 if node deleted
int contact_update_read(struct contact* c);
int contact_update_write(struct contact* c);

//inputs tcp server socket
//this is assuming the TCP server has something waiting
//0 on failure
int contact_new_from_server(int serv_sock);
int contact_new_from_client(struct contact* c);

//returns a contact by key
void contact_get_by_key(char* key, struct contact **c);

#endif

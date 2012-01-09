#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include "constants.h"
#include "chat.h"
#include "views.h"

//Timeout interval between calls to select(). Will use 0 for no intervals
//and user-called.
static struct timeval timeout;

//contains largest value of a socket, used in select()
static int max_dscrp = -1;

//acting as server receiving from TCP
//socket for tcp server
static int servsock_tcp;
static unsigned int servport_tcp;
static struct sockaddr_in servsockaddr_tcp;
static char serv_nick[ENTRY_NICKNAME_SIZE+1];
static char serv_key[ENTRY_KEY_SIZE+1];
static char serv_port[5+1];
static char serv_address[15+1];

static void log_add_entry(char* file,char* msg);

void chat_set_server_address(char* s) 
{
	if(s == NULL) {
		return;
	}

	memset(serv_address,'\0',15+1);
	sprintf(serv_address,"%s",s);

#if VERBOSE == 1
	view_debug_print("SERVER : new server address[%s]",chat_server_address());
#endif
}

void chat_set_server_port(char* s) 
{
	if(s == NULL) {
		return;
	}

	memset(serv_port,'\0',5+1);
	sprintf(serv_port,"%s",s);

#if VERBOSE == 1
	view_debug_print("SERVER : new server port[%s]",chat_server_port());
#endif
}

void chat_set_server_nick(char* s) 
{
	if(s == NULL) {
		return;
	}

	memset(serv_nick,'\0',ENTRY_NICKNAME_SIZE+1);
	sprintf(serv_nick,"%s",s);

#if VERBOSE == 1
	view_debug_print("SERVER : new server nick[%s]",chat_server_nick());
#endif
}

void chat_set_server_key(char* s)
{
	if(s == NULL) {
		return;
	}

	memset(serv_key,'\0',ENTRY_KEY_SIZE+1);
	sprintf(serv_key,"%s",s);

#if VERBOSE == 1
	view_debug_print("SERVER : new server key[%s]",chat_server_key());
#endif
}

char* chat_server_address()
{
	return serv_address;
}

char* chat_server_port()
{
	return serv_port;
}

char* chat_server_nick()
{
	return serv_nick;
}

char* chat_server_key()
{
	return serv_key;
}

int chat_server_init(unsigned int portNo) 
{
	//save port number for later reference
	servport_tcp = portNo;

	//fill sockaddr with 0s
	memset (&servsockaddr_tcp, 0, sizeof(struct sockaddr_in));
	servsockaddr_tcp.sin_family = AF_INET;
	servsockaddr_tcp.sin_addr.s_addr = htonl(INADDR_ANY);
	servsockaddr_tcp.sin_port = htons(servport_tcp);

	if((servsock_tcp = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return 1;

	int one = 1;
	setsockopt(servsock_tcp,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));

	if(bind(servsock_tcp, 
				(struct sockaddr *) &servsockaddr_tcp,
				sizeof(servsockaddr_tcp)) < 0)
		return 1;

	if(listen(servsock_tcp, MAXPENDINGTCP) < 0)
		return 1;

	//assuming interval between select() is user-called
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

//	view_debug_print("SERVER : listening on port %d",portNo);

	return 0;
}

void chat_free_node_self()
{
	close(servsock_tcp);
}

void chat_input_poll()
{
	//contains all the socket
	fd_set sock_set_read,sock_set_write;
	//counter used for sock clients
	struct contact* c;

	//update max_dscrp, ONLY for TCP server socket
	//we need to keep track of highest number of socket in order
	//to not search the max value a socket can hold
	max_dscrp = servsock_tcp;

	//socksets must be reset everytime select() is called
	FD_ZERO(&sock_set_read);
	FD_ZERO(&sock_set_write);
	//add server so it can be scanned to see if any new clients are waiting
	FD_SET(servsock_tcp, &sock_set_read);

	for(contact_get_first(&c); c != NULL; contact_get_next(&c)) {

		if(c->c_states[CONNECTED] == 1 || c->c_states[RETRY_CONNECTION] > 0) {
			FD_SET(c->c_tcp_sock, &sock_set_read);
			FD_SET(c->c_tcp_sock, &sock_set_write);

			//we need to keep track of highest number of socket in order
			//to not search the max value a socket can hold
			if (c->c_tcp_sock > max_dscrp)
				max_dscrp = c->c_tcp_sock;
		}
	}

	//check if no new clients
	int val;
	if((val=select(max_dscrp+1,&sock_set_read,&sock_set_write,NULL,&timeout)) == 0) {
		//		view_debug_print("%d connected clients", contact_size());
	}
	else {

		//check if new connections to server, and pass save them to global lst
		if(FD_ISSET(servsock_tcp, &sock_set_read)) { 
			contact_new_from_server(servsock_tcp);
		}


		//check if any client sockets are updated
		for(contact_get_first(&c); c != NULL; contact_get_next(&c)) {
			if(c->c_states[CONNECTED] == 1 || c->c_states[RETRY_CONNECTION] > 0) {

				if(FD_ISSET(c->c_tcp_sock, &sock_set_read) && c->c_states[CONNECTED] == 1) {
					if(contact_update_read(c))
						contact_get_first(&c);
				}


				int rcv_sock;
				unsigned int rcv_size = sizeof(rcv_sock);
				if(getsockopt(c->c_tcp_sock,SOL_SOCKET,SO_ERROR,&rcv_sock,&rcv_size) < 0) {
//					view_debug_print("getsockopt() failed");
				} else {
//					view_debug_print("getsockopt() %d %d", rcv_sock, ENOTCONN);
				}

				//connecting for first time or retrying connection
//				if(FD_ISSET(c->c_tcp_sock, &sock_set_write) && c->c_states[RETRY_CONNECTION] > 0 && c->c_states[CONNECTED] == 0) {
				if(FD_ISSET(c->c_tcp_sock, &sock_set_write) && c->c_states[RETRY_CONNECTION] > 0 && rcv_sock == 0 && c->c_states[CONNECTED] == 0) {
					contact_update_write(c);
				} 
			}
		}

	}

	FD_ZERO(&sock_set_read);
	FD_ZERO(&sock_set_write);

}

//1 for success; 0 otherwise; connects to a peer
int chat_connect(struct contact* c) 
{
	if(c == NULL){
		return 0;
	}

	if(!contact_new_from_client(c)) {
		//error
		return 0;
	}

	return 1;
}

void chat_send(struct contact* c, struct chat_event* evt)
{
	if(c == NULL || (&evt) == NULL){
		return;
	}

	if(c->c_states[CONNECTED] != 1){
#if VERBOSE == 1
		view_debug_print("SERVER : cannot send any messages to offline key[%s]",c->c_key);
#endif
		return;
	}

	//lets prepare packet
	unsigned int header = ((evt->ce_type)<<16)|((unsigned short)evt->ce_size);
	//declare for generic use
	char* msg = malloc(sizeof(char)*(HEADER_SIZE+evt->ce_size+1));
	sprintf(msg,"0000%s",evt->ce_message);
	unsigned short len = strlen(msg); //size of message, will be filled out later

	//generic setup for most messages
	msg[0] = (header & 0xFF000000)>>24;
	msg[1] = (header & 0x00FF0000)>>16;
	msg[2] = (header & 0x0000FF00)>>8;
	msg[3] = (header & 0x000000FF);

	if(evt->ce_type == CHAT_MESSAGE) {
		//nothing special needs to be done for CHAT_MESSAGE
	} else if(evt->ce_type == BROADCAST) {
		//nothing special needs to be done for NICKNAME
	} else if(evt->ce_type == BASIC_INFO) {
		struct contact* p;
		int i,peers;
		i = peers = 0;
		unsigned int address;
		unsigned short host_port;
		char* temp;
		free(msg);

		//get number of peers
		for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {
				if(p->c_states[BASIC_INFO_RECEIVED] == 1 && p != c)
					peers++;
		}

		len = 4+ENTRY_SIZE+peers*ENTRY_SIZE;
		header = ((evt->ce_type&0xFFFF)<<16)|((len-HEADER_SIZE)&0xFFFF);
		msg = malloc(sizeof(char)*(HEADER_SIZE+ENTRY_SIZE+peers*ENTRY_SIZE+1));
		memset(msg,0,len+1);

		msg[0] = (header & 0xFF000000)>>24;
		msg[1] = (header & 0x00FF0000)>>16;
		msg[2] = (header & 0x0000FF00)>>8;
		msg[3] = (header & 0x000000FF);

		i = HEADER_SIZE;

		//this is only for first entry, yourself
		temp = chat_server_nick();
//		run = 1;
//		for(j=0; j < ENTRY_NICKNAME_SIZE;j++) {
//			if(temp[j] == '\0')
//				run = 0;
//			if(run == 1)
//				msg[i] = temp[j];
//			i++;
//		}
		strncpy(&msg[i],temp,ENTRY_NICKNAME_SIZE);
		i += ENTRY_NICKNAME_SIZE;

		//address in 4 octets; little endian
		address = inet_addr(c->c_address);
		msg[i++] = 0;
		msg[i++] = 0;
		msg[i++] = 0;
		msg[i++] = 0;

		//port
		host_port = atoi(chat_server_port());
		msg[i++] = (0xFF00&host_port)>>8;
		msg[i++] = 0xFF&host_port;

		temp = chat_server_key();
		strncpy(&msg[i],temp,ENTRY_KEY_SIZE);
		i += ENTRY_KEY_SIZE;
//		run = 1;
//		for(j=0; j < ENTRY_KEY_SIZE;j++) {
//			if(temp[j] == '\0')
//				run = 0;
//			if(run == 1)
//				msg[i] = temp[j];
//			i++;
//		}

//		i=HEADER_SIZE+ENTRY_NICKNAME_SIZE+ENTRY_ADDRESS_SIZE+ENTRY_PORT_SIZE+ENTRY_KEY_SIZE;
		
		for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {
				if(p->c_states[BASIC_INFO_RECEIVED] != 1 || p == c) {
					continue;
				}
//			if(p == c) {
////				view_debug_print("RAW p == c %s", p->c_nickname);
//				continue;
//			}

//			if(p->c_states[CONNECTED] != 1) {
//				continue; 
//			}

//			run = 1;
////			view_debug_print("RAW sending nick %s to %s",p->c_nickname, c->c_nickname);
//			for(j=0; j < ENTRY_NICKNAME_SIZE;j++) {
//				if(p->c_nickname[j] == '\0')
//					run = 0;
//				if(run == 1)
//					msg[i] = p->c_nickname[j];
//				i++;
//			}
			strncpy(&msg[i],p->c_nickname,ENTRY_NICKNAME_SIZE);
			i += ENTRY_NICKNAME_SIZE;

			//address in 4 octets; little endian
			address = inet_addr(p->c_address);
			msg[i++] = (address&0x000000FF);
			msg[i++] = (address&0x0000FF00)>>8;
			msg[i++] = (address&0x00FF0000)>>16;
			msg[i++] = (address&0xFF000000)>>24;

			//port
//			if(p->c_listening_port == 0) {
//				view_debug_print("sending empty port!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
//			}

			msg[i++] = (0xFF00&p->c_listening_port)>>8;
			msg[i++] = 0xFF&p->c_listening_port;

			strncpy(&msg[i],p->c_key,ENTRY_KEY_SIZE);
			i += ENTRY_KEY_SIZE;
//			run = 1;
//			for(j=0;j<ENTRY_KEY_SIZE;j++) {
//				if(p->c_key[j] == '\0')
//					run = 0;
//				if(run == 1)
//					msg[i] = p->c_key[j];
//				i++;
//			}
		}

//		char test[len+1];
//		memset(test,'-',len+1);
//		test[len] = '\0';
//		for(j=0;j<len;j++) {
//			if(msg[j] != '\0')
//				test[j] = msg[j];
//		}
//		view_debug_print("DEBUG : SENT : %s",test);

	} else {
#if VERBOSE == 1
		view_debug_print("SERVER : could not identify event type in chat_send()");
#endif
	}
	
	if(send(c->c_tcp_sock,msg,len,0) == len) {
#if VERBOSE == 1
		view_debug_print("SERVER : (length %d) sent to nickname[%s] address[%s] port[%d]",len,c->c_nickname,c->c_address,c->c_port);
#endif
		if(evt->ce_type == BASIC_INFO) {
			c->c_states[BASIC_INFO_SENT] = 1;
		}
	} else {

#if VERBOSE == 1
		view_debug_print("~SERVER : %s(length %d) NOT sent to %s",evt->ce_message,len,c->c_nickname);
#endif
		
	}

	//let free() function correctly because we have null terminators
//	memset(msg,0,len+1);

	free(msg);
}

void chat_send_message(char* key,char* s)
{
	struct contact* c;

	contact_get_by_key(key,&c);

	if(c == NULL || s == NULL) {
#if VERBOSE == 1
		view_debug_print("SERVER : chat_send_message() using NULL argument(s)");
#endif
		return;
	}

	if(strcmp(s,"") == 0 || strcmp(s," ") == 0) {
#if VERBOSE == 1
		view_debug_print("SERVER : chat_send_message() received empty message");
#endif
		return;
	}

	struct chat_event* ce;
	ce_init(&ce);

	ce->ce_type = CHAT_MESSAGE;
	sprintf(ce->ce_message,"%s",s);
	ce->ce_size = strlen(ce->ce_message);
	chat_send(c,ce);
	//update message log
	char output[ENTRY_NICKNAME_SIZE+CHAT_MESSAGE_MAX+40+1];
	sprintf(output,"%s : %s",chat_server_nick(),ce->ce_message);
	chat_update_log(output,c->c_key);
	ce_free(ce);

}

void chat_send_broadcast(char* s)
{
	if(s == NULL) {
#if VERBOSE == 1
		view_debug_print("SERVER : chat_send_broadcast() received NULL argument(s)");
#endif
		return;
	}

	if(strcmp(s,"") == 0 || strcmp(s," ") == 0) {
#if VERBOSE == 1
		view_debug_print("SERVER : chat_send_broadcast() received empty message");
#endif
		return;
	}

	struct chat_event* ce;
	struct contact* p;
	ce_init(&ce);

	ce->ce_type = BROADCAST;
//	sprintf(ce->ce_message,"%s : %s",chat_server_nick(),s);
	sprintf(ce->ce_message,"%s",s);
	ce->ce_size = strlen(ce->ce_message);

#if VERBOSE == 1
	view_debug_print("passing along %s", ce->ce_message);
#endif
	for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {
		if(p->c_states[CONNECTED] == 1) {
			chat_send(p,ce);

			//update message log
			char msg[CHAT_MESSAGE_MAX+ENTRY_NICKNAME_SIZE+1];
			sprintf(msg,"BROADCAST %s : %s",chat_server_nick(),ce->ce_message);
			chat_update_log(msg,NULL);
		}
	}
	ce_free(ce);

}

void chat_update_log(char* msg,char* key)
{
	char* nick = NULL;

	if(msg == NULL) {
		return;
	}


	if(key != NULL) {
		if(strcmp(key,chat_server_key()) == 0) {
			nick = chat_server_nick();
		} else {

			struct contact* c;
			contact_get_by_key(key,&c);
			nick = c->c_nickname;

			if(c == NULL) {
				return;
			}
		}
	}

	time_t rawtime;
	struct tm *timeinfo;
	char stamp[ENTRY_NICKNAME_SIZE+CHAT_MESSAGE_MAX+40+1];
	char output[ENTRY_NICKNAME_SIZE+CHAT_MESSAGE_MAX+40+1];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(stamp,ENTRY_NICKNAME_SIZE+CHAT_MESSAGE_MAX+40+1,
			"[%Y:%m:%d:%H:%M:%S]",timeinfo);

//	if(key == NULL) {
	sprintf(output,"%s %s\n",stamp,msg);
	view_debug_print("%s %s",stamp,msg);
//	} else {
//		sprintf(output,"%s %s: %s\n",stamp,nick,msg);
//		view_debug_print("%s %s: %s ",stamp,nick,msg);
//	}

	char location[500];
	if(key == NULL) {
		sprintf(location,"./logs/%s","broadcast");
	} else {
		sprintf(location,"./logs/%s",nick);
	}

	log_add_entry(location,output);
}

void log_add_entry(char* file,char* msg) 
{
	FILE *fp;
	fp=fopen(file, "a");

	fprintf(fp,msg);

	fclose(fp);
}

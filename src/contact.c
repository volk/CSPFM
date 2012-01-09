//doubly linked list, not circular!
//sample looping
//struct contact* c;
//for(contact_get_first(&c); c != NULL; contact_get_next(&c));

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "constants.h"
#include "chat_packet_message.h"
#include "chat.h"
#include "contact.h"
#include "views.h"
#include "chat_event.h"

static struct contact* contact_head, *contact_tail;
static int contact_length;
static char* contact_folder;
static void contact_read_file(char* folder,char* filename);

static void contact_read_header(struct contact* d, char* buffer);
static void contact_read_message(struct contact* d, char* buffer);
static void contact_process_message(struct contact* c);

//adds a contact to local contact folder
//returns 1 on success
int contact_add_basic(struct contact** c) 
{
	if(c == NULL)
		return 0;

	int i;
	(*c) = malloc(sizeof(struct contact));
	(*c)->c_nickname = malloc((ENTRY_NICKNAME_SIZE+1)*sizeof(char));
	(*c)->c_address = malloc(17*sizeof(char));
	(*c)->c_key = malloc((ENTRY_KEY_SIZE+1)*sizeof(char));
	(*c)->c_listening_port = 0;
	(*c)->c_port = 0;

	cpm_init(&((*c)->cpm));

	cs_init(&((*c)->c_state));

	(*c)->c_tcp_sock = -1;
	for(i=0;i<NR_CHAT_EVENTS;i++)
		(*c)->c_states[i]=0;
	
	//if list is empty
	if(contact_size() == 0) {
		contact_head = contact_tail = *c;

		(*c)->contact_next = NULL;
		(*c)->contact_prev = NULL;
	} else {
		contact_head->contact_prev = (*c);
		(*c)->contact_next = contact_head;
		(*c)->contact_prev = NULL;

		contact_head = (*c);
	}

	contact_length++;

	return 1;
}

// *c points to last contact after call
// *c is NULL otherwise
void contact_get_last(struct contact** c) 
{
	if(c == NULL) {
		return;
	}

	*c = contact_tail;
}

// *c points to first contact after call
// *c is NULL otherwise
void contact_get_first(struct contact** c) 
{
	if(c == NULL) {
		return;
	}

	*c = contact_head;
}

//returns nothing on success or failure
void contact_delete(struct contact* c)
{
	if(c == NULL)
		return;

	struct contact* n = c->contact_next;
	struct contact* p = c->contact_prev;

	if(contact_size() == 1) {

		contact_tail = NULL;
		contact_head = NULL;

	} else if(contact_head == c) {

		contact_head = contact_head->contact_next;
		contact_head->contact_prev = NULL;

	} else if(contact_tail == c) {

		contact_tail = contact_tail->contact_prev;
		contact_tail->contact_next = NULL;

	} else {

		if(p != NULL)
			p->contact_next = n;

		if(n != NULL)
			n->contact_prev = p;

	}

	contact_length--;

	if(c->c_nickname != NULL)
		free(c->c_nickname);

	if(c->c_key != NULL)
		free(c->c_key);

	if(c->c_address != NULL)
		free(c->c_address);

	close(c->c_tcp_sock);

	free(c);
}

// *c points to NULL if no next exists
void contact_get_next(struct contact** c) {
	if(c == NULL) {
		return;
	}

	*c = (*c)->contact_next;
}

// *c points to NULL if no previous contact exists
void contact_get_prev(struct contact** c)
{
	if(c == NULL || *c == NULL) {
		return;
	}

	*c = (*c)->contact_prev;
}

int contact_size()
{
	return contact_length;
}

int contact_read(char* loc)
{
	if(loc == NULL)
		return 0;

	DIR *dir;
	struct dirent* dir_entry;
	struct stat stat_buf;
	int file_type;

	if(contact_folder != NULL)
		free(contact_folder);

	//save copy of directory for later reference
	contact_folder = malloc(256*sizeof(char));
	strcpy(contact_folder,loc);

	chat_set_server_nick("unspc_nick");
	chat_set_server_key("unspc_key");
	chat_set_server_address("1.1.1.1");
	chat_set_server_port("33333");

	//attempt to open directory
	if((dir=opendir(contact_folder)) == NULL)
		return 0;
	
	//read all files of directory
	while((dir_entry=readdir(dir)) != NULL)
	{
		char full_path[256];
		sprintf(full_path,"%s/%s",contact_folder,dir_entry->d_name);

		//used to get file type(normal, directory, pipe, etc.)
		file_type = stat(full_path, &stat_buf);

		//check if filetype holds no errors and that its a reg. file
		if(file_type != 0 || !S_ISREG(stat_buf.st_mode)) {
			continue;
		}


		//files found, lets try to extract from them
		contact_read_file(contact_folder,dir_entry->d_name);
	}
	closedir(dir);

	return 1;
}

//returns a pointer to a contact if file contents match
//specs of contact information file
//returns NULL on error
static void contact_read_file(char* path, char* filename)
{
	FILE *cur_file;
	char full_path[256];
	char nickname[ENTRY_NICKNAME_SIZE+2];
	char key[ENTRY_KEY_SIZE+2];
	char address[17];
	char port[7];
	int i;
	struct contact* c;
	
	//full_path name
	sprintf(full_path,"%s/%s",path,filename);

	if((cur_file = fopen(full_path,"r")) == NULL)
		return;

	if(fgets(nickname,ENTRY_NICKNAME_SIZE+2,cur_file) == NULL)
		return;

	if(fgets(address,17,cur_file) == NULL)
		return;

	if(fgets(port,7,cur_file) == NULL)
		return;
	
	if(fgets(key,ENTRY_KEY_SIZE+2,cur_file) == NULL)
		return;

	//removing all new lines

	for(i=0; i<16; i++) {
		if(address[i] == '\n') 
			address[i] = '\0';
	}

	for(i=0; i<ENTRY_NICKNAME_SIZE+1; i++) {
		if(nickname[i] == '\n') 
			nickname[i] = '\0';
	}

	for(i=0; i<ENTRY_KEY_SIZE+1; i++) {
		if(key[i] == '\n') 
			key[i] = '\0';
	}

	for(i=0; i<6; i++) {
		if(port[i] == '\n') 
			port[i] = '\0';
	}

	if(strcmp(filename,"config") == 0) {

		chat_set_server_address(address);
		chat_set_server_port(port);
		chat_set_server_nick(nickname);
		chat_set_server_key(key);

		fclose(cur_file);

		return;
	}

	if(!contact_add_basic(&c))
		return;
	
	c->c_listening_port = atoi(port);
	c->c_port = 0;
	sprintf(c->c_key,"%s",key);
	sprintf(c->c_nickname,"%s",nickname);
	sprintf(c->c_address,"%s",address);

	c->c_states[RETRY_CONNECTION] = 0;
	c->c_states[BASIC_INFO_RECEIVED] = 1;

#if VERBOSE == 1
	view_debug_print("SERVER : added to contact list nickname[%s] address[%s] port[%d] key[%s]", c->c_nickname,c->c_address,c->c_listening_port,c->c_key);
#endif

	fclose(cur_file);
}

//returns 1 if c is first element
int contact_is_first(struct contact * c)
{
	return c == contact_head;
}

//returns 1 if c is last element
int contact_is_last(struct contact * c)
{
	return c == contact_tail;
}

//writes local contacts folder so can be loaded later
//returns 1 on success; 0 on failure
//you must pass the location of folder( "." is pwd)
void contact_write(char* loc)
{
	FILE *fp;
	struct contact *p;
	char file[200];

	//write nickname, address, port, and key for each contact
	for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {

		sprintf(file,"%s/contacts/%s",loc,p->c_nickname);

		if((fp=fopen(file,"w")) == NULL) {
#if VERBOSE == 1
			view_debug_print("contact_write() : could not open file[%s] for writing",p->c_nickname);
#endif
			continue;
		} else {
#if VERBOSE == 1
			view_debug_print("SERVER : written out to contacts folder nickname[%s] " 
					"address[%s] port[%d] key[%s]", p->c_nickname,
					p->c_address,p->c_listening_port,p->c_key);
#endif
		}

		fprintf(fp, "%s\n", p->c_nickname);
		fprintf(fp, "%s\n", p->c_address);
		fprintf(fp, "%d\n", p->c_listening_port);
		fprintf(fp, "%s\n", p->c_key);

		fclose(fp);
	}

	//write out server information
	sprintf(file,"%s/contacts/config",loc);

	if((fp=fopen(file,"w")) == NULL) {
#if VERBOSE == 1
		view_debug_print("contact_write() : could not open file[%s] for writing",p->c_nickname);
#endif
		return;
	} else {
		view_debug_print("SERVER : written out server file to contacts folder" 
				" nickname[%s] key[%s] port[%s] address[%s]",chat_server_nick(),
				chat_server_key(), chat_server_port(),chat_server_address());
	}


	fprintf(fp, "%s\n", chat_server_nick());
	fprintf(fp, "%s\n", chat_server_address());
	fprintf(fp, "%s\n", chat_server_port());
	fprintf(fp, "%s\n", chat_server_key());

	fclose(fp);
}

//call this when a contact socket has been changed, whether 
//disconnected or new message received or otherwise
//returns 1 if node deleted
int contact_update_read(struct contact* p)
{
	//we cannot just exit out anytime and so must store this value 
	//to return it at the end
	int ret = 0;
	int buffer_size = 2;
	char buffer[buffer_size];
	struct chat_event* ce;
	int recv_msg_size;

	recv_msg_size = recv(p->c_tcp_sock, buffer, buffer_size-1, 0);
	buffer[recv_msg_size] = '\0';
	ce_init(&ce);

	if(recv_msg_size == 0) {
		view_debug_print("SERVER : %s disconnected",p->c_nickname);
		p->c_states[CONNECTED] = 0;
		p->c_states[RETRY_CONNECTION] = 0;
		ce->ce_type = DISCONNECTED;
		sprintf(ce->ce_message," ");
		ce->ce_size = strlen(ce->ce_message);

		//let interface know someone disconnected
		view_chat_handle_event(p,ce);

		ret = 1;
	}
	else if(recv_msg_size < 0) {
		//very unlikely to be here; requires advanced networking
	}
	else {

		int current_type = p->c_state->cs_type;

		//success if we have not been processing anything before
		//we still have something read otherwise
		if(current_type == NO_OP || current_type == READING_HEADER) {
			// reading opcode and size of message
			contact_read_header(p,buffer);

		} else if(current_type == READING_MESSAGE) {
			contact_read_message(p,buffer);
		}
	}

	ce_free(ce);
	return ret;
}

static void contact_read_message(struct contact* p, char* buffer)
{
	char t = buffer[0];
	int i;

	if(p == NULL || buffer == NULL) {
		return;
	}

	//not sure how we got here
	if(p->c_state->cs_type != READING_MESSAGE) {
		return;
	}

	//first time reading message if we are here
	if(p->cpm.cpm_offset == 0) {

		if(p->cpm.cpm_opcode == BASIC_INFO) {
			if(p->cpm.cbe_length != 0) {
#if VERBOSE == 1
				view_debug_print("SERVER : critical error : cbe_list NOT empty");
#endif

				for(i=0;i<p->cpm.cbe_length;i++) {
					cbe_free(p->cpm.cbe_list[i]);
				}

				free(p->cpm.cbe_list);
				p->cpm.cbe_length = 0;
			}

			//message size must be modulo ENTRY SIZE
			if(p->cpm.cpm_length % ENTRY_SIZE != 0) {
#if VERBOSE == 1
				view_debug_print("SERVER : critical error : message size NOT modulo ENTRY_SIZE");
#endif
			}

			p->cpm.cbe_length = p->cpm.cpm_length/ENTRY_SIZE;
			p->cpm.cbe_list = malloc(sizeof(struct chat_bare_entry*)*p->cpm.cbe_length);

			for(i=0;i<p->cpm.cbe_length;i++) {
				struct chat_bare_entry* cbe;
				cbe_init(&cbe);
				p->cpm.cbe_list[i] = cbe;
			}

		}else if(p->cpm.cpm_opcode == CHAT_MESSAGE) {
			memset(p->cpm.cpm_message,0,sizeof(sizeof(char)*(CHAT_MESSAGE_MAX+1)));
		}else if(p->cpm.cpm_opcode == BROADCAST) {
			memset(p->cpm.cpm_message,0,sizeof(sizeof(char)*(CHAT_MESSAGE_MAX+1)));
		}

	}

	if(p->cpm.cpm_opcode == BASIC_INFO) {
		//also check we are less than total message size
		if(p->cpm.cpm_offset < p->cpm.cpm_length) {
			struct chat_bare_entry* cbe = p->cpm.cbe_list[p->cpm.cpm_offset/ENTRY_SIZE];
			if(cbe == NULL) {
#if VERBOSE == 1
				view_debug_print("SERVER : critical : NULL %d",p->cpm.cbe_length);
#endif
			}
			int offset = (p->cpm.cpm_offset)%ENTRY_SIZE;

			if(offset < ENTRY_NICKNAME_SIZE) {
				cbe->cbe_nickname[offset] = t;
			} else if(offset >= ENTRY_NICKNAME_SIZE && offset < (ENTRY_NICKNAME_SIZE+ENTRY_ADDRESS_SIZE)) {
				cbe->cbe_address = cbe->cbe_address<<8;
				cbe->cbe_address = (cbe->cbe_address)|(t&0xFF);
			} else if(offset >= (ENTRY_NICKNAME_SIZE+ENTRY_ADDRESS_SIZE) && offset < (ENTRY_NICKNAME_SIZE+ENTRY_ADDRESS_SIZE+ENTRY_PORT_SIZE)) {
				cbe->cbe_port = cbe->cbe_port<<8;
				cbe->cbe_port = (cbe->cbe_port)|(t&0xFF);
			} else if(offset >= (ENTRY_NICKNAME_SIZE+ENTRY_ADDRESS_SIZE+ENTRY_PORT_SIZE) && offset < ENTRY_SIZE) {
				cbe->cbe_key[offset-ENTRY_NICKNAME_SIZE-ENTRY_ADDRESS_SIZE-ENTRY_PORT_SIZE] = t;
			}

			p->cpm.cpm_offset++;
		}
	}else if(p->cpm.cpm_opcode == CHAT_MESSAGE) {
		if(p->cpm.cpm_offset < CHAT_MESSAGE_MAX && p->cpm.cpm_offset < p->cpm.cpm_length) { 
			p->cpm.cpm_message[p->cpm.cpm_offset] = t;
			p->cpm.cpm_offset++;
		}
	}else if(p->cpm.cpm_opcode == BROADCAST) {
		if(p->cpm.cpm_offset < CHAT_MESSAGE_MAX && p->cpm.cpm_offset < p->cpm.cpm_length) { 
			p->cpm.cpm_message[p->cpm.cpm_offset] = t;
			p->cpm.cpm_offset++;
		}
	}

	//termination conditions
	if(p->cpm.cpm_opcode == BASIC_INFO) {
		if(p->cpm.cpm_offset >= p->cpm.cpm_length) {

#if VERBOSE == 1
			view_debug_print("SERVER(%s) : received message for BASIC_INFO[%d] size[%d]",p->c_nickname,p->cpm.cpm_opcode, p->cpm.cpm_length);
#endif

			//call process here
			contact_process_message(p);

			for(i=0;i<p->cpm.cbe_length;i++) {
				cbe_free(p->cpm.cbe_list[i]);
			}

			free(p->cpm.cbe_list);
			p->cpm.cbe_list = NULL;
			p->cpm.cbe_length = 0;
			p->c_state->cs_type = NO_OP;
			p->cpm.cpm_opcode = NO_OP;
			p->cpm.cpm_length = 0;
			p->cpm.cpm_offset = 0;
			memset(p->cpm.cpm_message,0,sizeof(sizeof(char)*(CHAT_MESSAGE_MAX+1)));
		}
	}else if(p->cpm.cpm_opcode == CHAT_MESSAGE) {
		if(p->cpm.cpm_offset == CHAT_MESSAGE_MAX || p->cpm.cpm_offset >= p->cpm.cpm_length) { 
			p->cpm.cpm_message[p->cpm.cpm_offset] = '\0';
			p->cpm.cpm_offset++;

			//call process here
			contact_process_message(p);
			//set opcode to noop
			p->c_state->cs_type = NO_OP;
			p->cpm.cpm_opcode = NO_OP;
			p->cpm.cpm_length = 0;
			p->cpm.cpm_offset = 0;
			memset(p->cpm.cpm_message,0,sizeof(sizeof(char)*(CHAT_MESSAGE_MAX+1)));
		}
	} else if(p->cpm.cpm_opcode == BROADCAST) { 
		if(p->cpm.cpm_offset == CHAT_MESSAGE_MAX || p->cpm.cpm_offset >= p->cpm.cpm_length) { 
			p->cpm.cpm_message[p->cpm.cpm_offset] = '\0';
			p->cpm.cpm_offset++;

			//call process here
			contact_process_message(p);
			//set opcode to noop
			p->c_state->cs_type = NO_OP;
			p->cpm.cpm_opcode = NO_OP;
			p->cpm.cpm_length = 0;
			p->cpm.cpm_offset = 0;
			memset(p->cpm.cpm_message,0,sizeof(sizeof(char)*(CHAT_MESSAGE_MAX+1)));
		}
	}

}

static void contact_process_message(struct contact* c)
{
	if(c == NULL)
		return;


	if(c->cpm.cpm_opcode == CHAT_MESSAGE) {
		struct chat_event* ce;
		ce_init(&ce);
		ce->ce_type = CHAT_MESSAGE;
		ce_resize_message(ce,sizeof(c->cpm.cpm_message)+15);
		sprintf(ce->ce_message,"%s",c->cpm.cpm_message);
		ce->ce_size = strlen(c->cpm.cpm_message);

#if VERBOSE == 1
		view_debug_print("%s : CHAT_MESSAGE received \"%s\"",c->c_nickname, ce->ce_message);
#endif
		view_chat_handle_event(c,ce);
		//update message log
		char msg[CHAT_MESSAGE_MAX+ENTRY_NICKNAME_SIZE+1];
		sprintf(msg,"%s : %s",c->c_nickname,ce->ce_message);
		chat_update_log(msg,c->c_key);
	} else if(c->cpm.cpm_opcode == BROADCAST) {
		struct chat_event* ce;
		ce_init(&ce);
		ce->ce_type = BROADCAST;
		sprintf(ce->ce_message,"%s",c->cpm.cpm_message);
		ce->ce_size = strlen(c->cpm.cpm_message);

#if VERBOSE == 1
		view_debug_print("SERVER : BROADCAST received \"%s\"", ce->ce_message);
#endif

		view_chat_handle_event(c,ce);
		//update message log
		char msg[CHAT_MESSAGE_MAX+ENTRY_NICKNAME_SIZE+1];
		sprintf(msg,"BROADCAST %s : %s",c->c_nickname,ce->ce_message);
		chat_update_log(msg,NULL);
	} else if(c->cpm.cpm_opcode == BASIC_INFO) {
#if VERBOSE == 1
		view_debug_print("SERVER(%s) : BASIC_INFO received : [%d] peers(not includ. host)",c->c_nickname,c->cpm.cbe_length-1);
#endif
		int i,changed=0,local_changed=0,new,send_out=1;
		struct chat_bare_entry* cbe = c->cpm.cbe_list[0];
		struct chat_event* ce;
		struct contact* p;
		char temp_address[15+1]; //1 for null term. and 15 for address in string format
		ce_init(&ce);
		sprintf(ce->ce_message,"contact changed");
//		view_debug_print("new LIST");

		//check for any duplicates by key and make sure the deleted peer is offline
		for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {
//			if(strcmp(p->c_key,cbe->cbe_key) == 0 && p->c_states[CONNECTED] == 0) {
//			view_debug_print("comparing %s with %s  = %d p != c == %d",p->c_key,cbe->cbe_key,strcmp(p->c_key,cbe->cbe_key),p!=c);
			if(strcmp(p->c_key,cbe->cbe_key) == 0 && p != c) {
#if VERBOSE == 1
				view_debug_print("SERVER : removing offline dupe by key[%s]",p->c_key);
#endif
//				view_debug_print("DEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE  [%s]",p->c_key);
				if(p->c_states[CONNECTED] == 1) {
					ce->ce_type = DISCONNECTED;
					view_chat_handle_event(c,ce);
				}
				contact_delete(p);
				break;
			}
		}

		for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {
		}


		if(strcmp(c->c_nickname,cbe->cbe_nickname) != 0) {
#if VERBOSE == 1
			view_debug_print("DEBUG(%s) : changed to nick[%s]",c->c_nickname,cbe->cbe_nickname);
#endif
			sprintf(c->c_nickname,"%s",cbe->cbe_nickname);
			changed = 1;
			local_changed = 1;
		}

		if(strcmp(c->c_key,cbe->cbe_key) != 0) {
			sprintf(c->c_key,"%s",cbe->cbe_key);
			changed = 1;
			local_changed = 1;
		}

		if(c->c_listening_port != cbe->cbe_port) {
			c->c_listening_port = cbe->cbe_port;
			changed = 1;
			local_changed = 1;
		}

		if(c->c_states[CONNECTED_SENT] == 0) {
			ce->ce_type = CONNECTED;
			view_chat_handle_event(c,ce);
#if VERBOSE == 1
			view_debug_print("CONNECTED sent, auto, nick[%s] key[%s]",c->c_nickname,c->c_key);
#endif
			c->c_states[CONNECTED_SENT] = 1;
		}

		c->c_states[BASIC_INFO_RECEIVED] = 1;
		

		if(local_changed == 0) {
#if VERBOSE == 1
			view_debug_print("SERVER : unchaged lead contact nick[%s] address[%s] port[%d] key[%s]", c->c_nickname, c->c_address,c->c_listening_port,c->c_key);
#endif
		} else {
			view_debug_print("SERVER : updated lead contact nick[%s] address[%s] port[%d] key[%s]", c->c_nickname, c->c_address,c->c_listening_port,c->c_key);
		}

		for(i=1;i<c->cpm.cbe_length;i++) {
			new = 0;
			local_changed = 0;
			cbe = c->cpm.cbe_list[i];

			//if this is you or from the contact c, ignore yourself
			if(strcmp(chat_server_key(),cbe->cbe_key) == 0) {
				continue;
			}

			//check if this is an existing peer; find him
			for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {
				if(strcmp(p->c_key,cbe->cbe_key) == 0) {
					break;
				}
			}
			//p should now contain pointer to existing peer or NULL

			if(p == NULL) {
				contact_add_basic(&p);
				new = 1;
				changed = 1;
				local_changed = 1;
			}
			if(new == 1) {
				sprintf(p->c_nickname,"%s",cbe->cbe_nickname);
				sprintf(p->c_key,"%s",cbe->cbe_key);
				sprintf(p->c_address,"%d.%d.%d.%d",
						(cbe->cbe_address&0xFF000000)>>24,
						(cbe->cbe_address&0x00FF0000)>>16,
						(cbe->cbe_address&0x0000FF00)>>8,
						(cbe->cbe_address&0x000000FF)
						);
				p->c_listening_port= cbe->cbe_port;
			} else {

				if(strcmp(p->c_nickname,cbe->cbe_nickname) != 0) {
					sprintf(p->c_nickname,"%s",cbe->cbe_nickname);
					ce->ce_type = CONTACT_CHANGED;
					view_chat_handle_event(p,ce);
					changed = 1;
					local_changed = 1;
				}

				if(strcmp(p->c_key,cbe->cbe_key) != 0) {
					sprintf(p->c_key,"%s",cbe->cbe_key);
					changed = 1;
					local_changed = 1;
				}


				sprintf(temp_address,"%d.%d.%d.%d",
						(cbe->cbe_address&0xFF000000)>>24,
						(cbe->cbe_address&0x00FF0000)>>16,
						(cbe->cbe_address&0x0000FF00)>>8,
						(cbe->cbe_address&0x000000FF)
						);
				if(strcmp(p->c_address,temp_address) != 0) {
					sprintf(p->c_address,"%s",temp_address);
					changed = 1;
					local_changed = 1;
				}

				if(p->c_listening_port != cbe->cbe_port) {
					p->c_listening_port = cbe->cbe_port;
					changed = 1;
					local_changed = 1;
				}

			}

			if(new == 1) {
#if VERBOSE == 1
				view_debug_print("SERVER : new contact nick[%s] address[%s] port[%d] key[%s]", p->c_nickname, p->c_address,p->c_listening_port,p->c_key);
#endif
			} else {

				if(local_changed == 1) {
#if VERBOSE == 1
					view_debug_print("SERVER : updated contact nick[%s] address[%s] port[%d] key[%s]", p->c_nickname, p->c_address,p->c_listening_port,p->c_key);
#endif
				} else {
#if VERBOSE == 1
					view_debug_print("SERVER : unchanged contact nick[%s] address[%s] port[%d] key[%s]", p->c_nickname, p->c_address,p->c_listening_port,p->c_key);
#endif
				}

			}


		}

		//if list has changed
		if(changed == 1) {
#if VERBOSE == 1
			view_debug_print("SERVER : changed RECV_INFO list, request to send out new list and (re)connect to all");
#endif
			ce->ce_type = BASIC_INFO;

			//resend new list to everyone
			for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {
				if(p->c_states[CONNECTED] == 1) {
#if VERBOSE == 1
					view_debug_print("sending new list to key[%s]",p->c_key);
#endif
					chat_send(p,ce);
				}
			}

			for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {
				if(p->c_states[BASIC_INFO_RECEIVED] == 0 && p->c_states[CONNECTED] == 1) {
					send_out = 0;
					break;
				}
			}

			//then try to connect to all offline peers
			if(send_out == 1) {
				for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {
					if(p->c_states[CONNECTED] == 0) {
#if VERBOSE == 1
						view_debug_print("trying to connect to key[%s]",p->c_key);
#endif

						chat_connect(p);
					} else {
//						view_debug_print("NOT trying to coneect to key[%s]",p->c_key);
					}
				}
			}

		} else {
#if VERBOSE == 1
			view_debug_print("SERVER : unchanged RECV_INFO list");
#endif
		}
	}

	//ce_free(ce);
}



static void contact_read_header(struct contact* p, char* buffer)
{
	if(p == NULL || buffer == NULL)
		return;

	char t = buffer[0];

	//invalid type with to which we got here, exit now
	if(p->c_state->cs_type != READING_HEADER && p->c_state->cs_type != NO_OP) {
		return; 
	}

	//we should set to reading if we came here for the first time
	if(p->c_state->cs_type == NO_OP) {
		p->c_state->cs_type = READING_HEADER;
	} 

	//check if we finished reading header
	if(p->cpm.cpm_offset < HEADER_SIZE) {
		if(p->cpm.cpm_offset == 0) {
#if VERBOSE == 1
			view_debug_print("SERVER(%s) : started reading header",p->c_nickname);
#endif
		}

		//only first four bytes for clarity
		if(p->cpm.cpm_offset < OPCODE_SIZE) {
			p->cpm.cpm_opcode = p->cpm.cpm_opcode<<8;
			p->cpm.cpm_opcode = p->cpm.cpm_opcode|(0xFF&t);
		} else if(p->cpm.cpm_offset >= OPCODE_SIZE && p->cpm.cpm_offset < HEADER_SIZE) {
			p->cpm.cpm_length = p->cpm.cpm_length<<8;
			p->cpm.cpm_length = p->cpm.cpm_length|(0xFF&t);
		}

		p->cpm.cpm_offset++;
	} 
	
	if(p->cpm.cpm_offset >= HEADER_SIZE){
		p->c_state->cs_type = READING_MESSAGE;
		p->cpm.cpm_offset = 0;

		if(p->cpm.cpm_opcode >= NR_CHAT_EVENTS) {
#if VERBOSE == 1
			view_debug_print("SERVER(%s) : finished reading ILLEGAL header[%d] length[%d]",p->c_nickname,p->cpm.cpm_opcode,p->cpm.cpm_length);
#endif
		} else {
#if VERBOSE == 1
			view_debug_print("SERVER(%s) : finished reading legal header[%d] length[%d]",p->c_nickname,p->cpm.cpm_opcode,p->cpm.cpm_length);
#endif
		}
	}

}

int contact_update_write(struct contact* p)
{
	int ret = 0;
	if(p->c_states[CONNECTED] == 0) {
		struct chat_event* ce;
		ce_init(&ce);
		if(connect(p->c_tcp_sock,(struct sockaddr*)&p->c_sockaddr, sizeof(p->c_sockaddr)) < 0){
			p->c_states[RETRY_CONNECTION]--;
			
			if(p->c_states[RETRY_CONNECTION] == 0) {
				ce->ce_type = CHAT_MESSAGE;

				sprintf(ce->ce_message,"Unable to connect to %s on %d",p->c_nickname,p->c_listening_port);
				ce->ce_size = strlen(ce->ce_message);

//				view_chat_handle_event(p,ce);
				
#if VERBOSE == 1
				view_debug_print("SERVER : could not connect nickname[%s] retries remaining[%d]",p->c_nickname,p->c_states[RETRY_CONNECTION]);
#endif
			}

		}
		else {
			p->c_states[CONNECTED] = 1;
			p->c_states[RETRY_CONNECTION] = 0;
			p->c_states[CONNECTED_SENT] = 1;
			ce->ce_type = CONNECTED;
			view_debug_print("CONNECTED sent, manual, nick[%s] key[%s]",p->c_nickname,p->c_key);
			view_chat_handle_event(p,ce);
			ce->ce_type = CONTACT_CHANGED;
//			view_chat_handle_event(p,ce);

#if VERBOSE == 1
			view_debug_print("SERVER : connected to nickname[%s] at address[%s]:port[%d]", p->c_nickname, p->c_address, p->c_port);
#endif
			

			ce->ce_type = BASIC_INFO;
			chat_send(p,ce);
		}


		ce_free(ce);
	}

	return ret;
}

//inputs tcp server socket
//this is assuming the TCP server has something waiting
int contact_new_from_server(int serv_sock)
{
	unsigned int clnt_len;
	struct contact* p;

	if(!contact_add_basic(&p))
		return 0;

	clnt_len = sizeof(p->c_sockaddr);

	if((p->c_tcp_sock = accept(serv_sock,(struct sockaddr*)&p->c_sockaddr,&clnt_len)) < 0)
		return 0;

	p->c_states[CONNECTED] = 1;
	p->c_states[RETRY_CONNECTION] = 0;

	p->c_port = ntohs(p->c_sockaddr.sin_port);
	sprintf(p->c_address,"%s",inet_ntoa(p->c_sockaddr.sin_addr));
	sprintf(p->c_key," ");
	sprintf(p->c_nickname,"%s:%d",p->c_address,p->c_port);

	view_debug_print("SERVER : %s connected",p->c_nickname);

	struct chat_event* ce;
	ce_init(&ce);
//	ce->ce_type = CONNECTED;
	sprintf(ce->ce_message," ");
	ce->ce_size = strlen(ce->ce_message);

	//let interface know someone connected
//	view_chat_handle_event(p,ce);
	
	ce->ce_type = BASIC_INFO;
	chat_send(p,ce);

	ce_free(ce);

	return 1;
}

int contact_new_from_client(struct contact* c)
{
	if(c == NULL)
		return 0;

	if((c->c_tcp_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return 0;

	struct chat_event* ce;
	ce_init(&ce);

	memset(&(c->c_sockaddr), 0, sizeof(c->c_sockaddr));
	c->c_sockaddr.sin_family = AF_INET;
	c->c_sockaddr.sin_addr.s_addr = inet_addr(c->c_address);
	c->c_sockaddr.sin_port = htons(c->c_listening_port);

	c->c_port = c->c_listening_port;

	int x;
	x=fcntl(c->c_tcp_sock,F_GETFL,0);
	fcntl(c->c_tcp_sock,F_SETFL,x | O_NONBLOCK);

	if(connect(c->c_tcp_sock,(struct sockaddr*)&(c->c_sockaddr), sizeof(c->c_sockaddr)) < 0){
		c->c_states[CONNECTED] = 0;
		c->c_states[RETRY_CONNECTION] = 5;
#if VERBOSE == 1
		view_debug_print("SERVER : set max %d retries for nickname[%s] address[%s] port[%d] key[%s]", c->c_states[RETRY_CONNECTION],c->c_nickname,c->c_address,c->c_port,c->c_key);
#endif

		if(errno == EINPROGRESS) {
			//connection is always in progress whether connected or not
		} else {

#if VERBOSE == 1
			view_debug_print("SERVER : could not connect to nickname[%s] address[%s] port[%d] key[%s]", c->c_nickname,c->c_address,c->c_port,c->c_key);
#endif

			ce->ce_type = CHAT_MESSAGE;

			sprintf(ce->ce_message,"SERVER : unable to connect to %s",c->c_nickname);
			ce->ce_size = strlen(ce->ce_message);

			view_chat_handle_event(c,ce);
		}

	}
	else {
		c->c_states[CONNECTED] = 1;
		view_debug_print("%s successfully connected",c->c_nickname);
	}

	ce_free(ce);

	return 1;
}

//returns a contact by key
void contact_get_by_key(char* key, struct contact **c) 
{
	if(key == NULL || c == NULL || strcmp(key," ") == 0) {
		*c = NULL;
		view_debug_print("invalid arguments to contact_get_by_key()");
	}
	
	struct contact* p;

	for(contact_get_first(&p); p != NULL; contact_get_next(&p)) {
		if(strcmp(p->c_key,key) == 0) {
			*c = p;
			return;
		}
	}

	*c = NULL;
	view_debug_print("contact with key[%s] does not exist contact_get_by_key()",key);
}

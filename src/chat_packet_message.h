#ifndef __CHAT_PACKAGE_MESSAGE_H
#define __CHAT_PACKAGE_MESSAGE_H

struct chat_bare_entry {
	//the order of these elements should not change!
	char cbe_nickname[ENTRY_NICKNAME_SIZE+1]; 
	char cbe_key[ENTRY_KEY_SIZE+1];
	unsigned int cbe_address; 
	unsigned short cbe_port; 
};

struct chat_packet_message { 
	//the first two elements must be in the first two positions
	unsigned short cpm_opcode; 
	unsigned short cpm_length; 
	//the order of these elements does not matter
	unsigned int cpm_offset;
	struct chat_bare_entry** cbe_list;
	char cpm_message[CHAT_MESSAGE_MAX+1]; 
	unsigned short cbe_length;
};

void cpm_init(struct chat_packet_message* c);
void cbe_init(struct chat_bare_entry** c);
void cbe_free(struct chat_bare_entry* c);

#endif

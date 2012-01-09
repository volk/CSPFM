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

void cbe_free(struct chat_bare_entry* c) 
{
	if(c == NULL) {
		return;
	}

	free(c);
}

void cpm_init(struct chat_packet_message* cpm) 
{
	if(cpm == NULL) {
		return; 
	}

	memset(cpm,0,sizeof(struct chat_packet_message));
}

void cbe_init(struct chat_bare_entry** cbe) 
{
	if(cbe == NULL) {
		return;
	}

	(*cbe) = malloc(sizeof(struct chat_bare_entry));

	memset(*cbe,0,sizeof(struct chat_bare_entry));
}

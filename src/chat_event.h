 /*
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CHAT_EVENT_H
#define __CHAT_EVENT_H
#include "constants.h"

//typically, the sequence goes as such
//USER_CONNECTING -> USER_CONNECTED
//to : USER_NICKNAME,sizeof(nickname), attach nickname
//from : USER_NICKNAME, size, fill in
//to : USER_MESSAGE_TO_SEND, size, msg
//USER is NOT you but the other person
//variables without USER prefix resolve
//to your machine and your states

enum ce_state {
	//new flags
	NO_OP, //empty state
	VISIBLE,
	CONTACT_LIST,
	BROADCAST,
	CHAT_MESSAGE,
	BASIC_INFO_RECEIVED,
	BASIC_INFO_SENT,
	BASIC_INFO,
	READING_HEADER,
	READING_MESSAGE,
	ENTRY,
	CONNECTED_SENT,
	CONNECTED,//if connect() or accept() succeeded
	
	//only used for interface, don't modify these in an array
	DISCONNECTED,//if user disconnected
	RETRY_CONNECTION,
	FIRST_BASIC_INFO,
	CONTACT_CHANGED,

	NR_CHAT_EVENTS
};


struct chat_event {
	unsigned int ce_type;
	char * ce_message;
	int ce_size;
};

struct chat_state {
	unsigned int cs_type;
	unsigned int type;
	char * cs_message;
	unsigned int cs_size;
	int buf[4];
};

//all for chat_state, NOT struct chat_event
void cs_init(struct chat_state** c);
void cs_resize_message(struct chat_state* c, unsigned short bytes);
void cs_free(struct chat_state* c);

//all for chat_event, not chat_state
void ce_init(struct chat_event** c);
void ce_resize_message(struct chat_event* c, unsigned short bytes);
void ce_free(struct chat_event* c);

//all for struct chat_event
#endif

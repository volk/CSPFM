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

#ifndef __CHAT_H
#define __CHAT_H

#include "constants.h"
#include "contact.h"

struct contact_list;
struct chat_node_self;

//0 for success; 1 otherwise
int chat_server_init(unsigned int portNo);
void chat_free_node_self();
void chat_input_poll();
//0 for success; 1 otherwise; connects to a peer
int chat_connect(struct contact* c);
void chat_send_broadcast(char* s);
void chat_send_message(char* key,char* s);
void chat_send(struct contact* c, struct chat_event* evt);

void chat_set_server_nick(char* s);
void chat_set_server_key(char* s);
void chat_set_server_address(char* s);
void chat_set_server_port(char* s);

char* chat_server_nick();
char* chat_server_key();
char* chat_server_address();
char* chat_server_port();

void chat_update_log(char* msg,char* key);

#endif

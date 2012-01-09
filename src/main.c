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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <locale.h>
#include "constants.h"
#include "contact.h"
#include "chat.h"
#include "windows.h"
#include "views.h"
#include "tabs.h"
#include "controllers.h"
#include "translation.h"

#define MAX_TRIES 9 //number of ports to search to bind() on
#define TESTING 0

int main(int argc, char *argv[]) {

	setlocale(LC_ALL, "");
	bindtextdomain("cs490", "./");
	textdomain("cs490");

	if(!contact_read("./contacts"))
		view_debug_print("failed to open contacts folder");

	view_types_init();
	windows_init();
	tabs_init();
	controllers_init();

	//find a port to bind on
	int i,start_port;
	char port[5+1];
	start_port = atoi(chat_server_port());
	for(i=start_port; i<start_port+MAX_TRIES;i++){

		if(chat_server_init(i)) {
			view_debug_print("chat_init_node_self() failed to initialize");
		} else {
			sprintf(port,"%d",i);
			chat_set_server_port(port);
			break;
		}

	}

	struct contact* c;

	view_debug_print("SERVER : self-identification : nickname[%s] key[%s] " 
			"port[%s] address[%s]",chat_server_nick(),chat_server_key(),
			chat_server_port(),chat_server_address());

	for(contact_get_first(&c); c != NULL; contact_get_next(&c)) {
		view_debug_print("SERVER : added to contact list nickname[%s] " 
				"address[%s] port[%d] key[%s]", c->c_nickname,
				c->c_address,c->c_listening_port,c->c_key);
	}

	while(1) {

		//poll only reads a byte at a time, so multiple checks
		//are beneficial in speeding up the communication
		for(i=0;i<7;i++) {
			chat_input_poll();
//			view_debug_print("------ size of contact list %d",contact_size());
		}

		controllers_input_poll(50);
	}

	return 0;
}

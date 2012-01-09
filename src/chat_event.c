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
#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "chat_event.h"

void cs_init(struct chat_state** c)
{
	if(c == NULL)
		return;

	(*c) = malloc(sizeof(struct chat_state));
	(*c)->cs_type = NO_OP;
	(*c)->type = NO_OP;
	(*c)->cs_message = malloc(sizeof(char)*2048);
	(*c)->cs_message[0] = '\0';
	(*c)->cs_size = 0;
}

void cs_free(struct chat_state* c)
{
	if(c == NULL)
		return;

	free(c->cs_message);
	free(c);
}

void cs_resize_message(struct chat_state* c, unsigned short bytes) 
{
	if(c == NULL || bytes < 0)
		return;

	char* str = malloc(bytes*sizeof(char));
	snprintf(str,bytes,"%s",c->cs_message);
	free(c->cs_message);
	c->cs_message = str;
	c->cs_message[0] = '\0';
}

void ce_init(struct chat_event** c)
{
	if(c == NULL)
		return;

	(*c) = malloc(sizeof(struct chat_event));
	(*c)->ce_type = NO_OP;
	(*c)->ce_message = malloc(sizeof(char)*(CHAT_MESSAGE_MAX+1));
	memset((*c)->ce_message,0,sizeof(char)*(CHAT_MESSAGE_MAX+1));
	(*c)->ce_size = 0;
}

void ce_free(struct chat_event* c)
{
	if(c == NULL)
		return;

	free(c->ce_message);
	free(c);
}

void ce_resize_message(struct chat_event* c, unsigned short bytes) 
{
	if(c == NULL || bytes < 0)
		return;

	char* str = malloc(bytes*sizeof(char));
	snprintf(str,bytes,"%s",c->ce_message);
	free(c->ce_message);
	c->ce_message = str;
	c->ce_message[0] = '\0';
}

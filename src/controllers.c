/*
 * src/controllers.c
 * Controllers system.
 *
 * Copyright (c) 2011 Patrick "P. J." McDermott
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
#include <curses.h>
#include "controllers.h"
#include "tabs.h"

struct controller_key {
	int ck_key;
	int (*ck_handler)(int);
	struct controller_key *ck_next;
};

struct controller {
	struct controller_key *c_keys_head, *c_keys_tail;
};

static struct controller *controller_global, *controller_view;

static int
controller_tab_prev(int k)
{
	tab_show_prev();

	return CONTROLLERS_STOP;
}

static int
controller_tab_next(int k)
{
	tab_show_next();

	return CONTROLLERS_STOP;
}

void
controllers_init()
{
	controller_global = controller_init();
	controller_add_key(controller_global, KEY_F(5), &controller_tab_prev);
	controller_add_key(controller_global, KEY_F(6), &controller_tab_next);
}

struct controller *
controller_init()
{
	struct controller *con;

	/* Allocate memory for and populate a new controller.  If we can't allocate
	 * memory on the heap, return an error. */
	con = malloc(sizeof(struct controller));
	if (con == NULL) {
		return NULL;
	}
	con->c_keys_head = NULL;
	con->c_keys_tail = NULL;

	return con;
}

void
controller_free(struct controller *con)
{
	struct controller_key *k, *kk;

	/* Make sure we've received a real controller. */
	if (con == NULL) {
		return;
	}

	/* Free ALL the keys! */
	for (k = con->c_keys_head; k != NULL;) {
		kk = k;
		k = k->ck_next;
		free(kk);
	}

	/* Free the controller. */
	free(con);
}

int
controller_add_key(struct controller *con, int key, int (*handler)(int))
{
	struct controller_key *new_key;

	/* Allocate memory for and populate a new key.  If we can't allocate memory
	 * on the heap, return an error. */
	new_key = malloc(sizeof(*new_key));
	if (new_key == NULL) {
		/* TODO: I guess this should use a value from an errors enum. */
		return 1;
	}
	new_key->ck_key = key;
	new_key->ck_handler = handler;
	new_key->ck_next = NULL;

	/* Add the new key to the controller. */
	/* TODO: What if this key already exists in the controller?  Right now,
	 * duplicate key entries are simply ignored.  Should we loop through the
	 * whole list to find a key and replace it, or should we just push a new
	 * entry to the head of the list? */
	if (con->c_keys_tail == NULL) {
		con->c_keys_head = new_key;
		con->c_keys_tail = new_key;
	} else {
		con->c_keys_tail->ck_next = new_key;
		con->c_keys_tail = new_key;
	}

	return 0;
}

void
controllers_set_view_controller(struct controller *con)
{
	controller_view = con;
}

void
controllers_input_poll(int timeout)
{
	int c;
	struct controller_key *k;

	/* Check for input. */
	timeout(timeout);
	c = getch();
	if (c == ERR) {
		return;
	}

	/* Find the key in the global controller. */
	for (k = controller_global->c_keys_head; k != NULL; k = k->ck_next) {
		if (k->ck_key == CONTROLLER_KEY_ANY || k->ck_key == c){
			if (k->ck_handler(c) == CONTROLLERS_STOP) {
				return;
			}
		}
	}

	/* Find the key in the view controller. */
	for (k = controller_view->c_keys_head; k != NULL; k = k->ck_next) {
		if (k->ck_key == CONTROLLER_KEY_ANY || k->ck_key == c){
			if (k->ck_handler(c) == CONTROLLERS_STOP) {
				return;
			}
		}
	}
}

/*
 * src/views.c
 * Views system.
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
#include <string.h>
#include "views.h"

struct view_type {
	char *vt_name;
	struct controller *vt_controller;
	struct view *(*vt_init)(void);
	void (*vt_free)(struct view *);
	void (*vt_draw)(struct view *);
	struct view_type *vt_next;
};

static struct view_type *view_types_head = NULL, *view_types_tail = NULL;

static struct view *view_current = NULL;

static struct view_type *
view_type_get(char *type)
{
	struct view_type *vt;

	for (vt = view_types_head; vt != NULL; vt = vt->vt_next) {
		if (strcmp(vt->vt_name, type) == 0) {
			return vt;
		}
	}

	return NULL;
}

void
view_draw_current()
{
	if (view_current == NULL) {
		return;
	}

	view_current->v_type->vt_draw(view_current);
}

void
view_set_current(struct view *v)
{
	if (v == NULL) {
		return;
	}

	view_current = v;
	controllers_set_view_controller(v->v_type->vt_controller);
}

int
view_is_current(struct view *v)
{
	return v == view_current;
}

int
view_type_register(char *name, struct controller *con,
		struct view *(*init_func)(void), void (*free_func)(struct view *),
		void (*draw_func)(struct view *))
{
	struct view_type *vt;

	/* TODO: Currently we just trust the caller to not use an existing name. */

	/* Allocate memory for and populate a new view type.  If we can't allocate
	 * memory on the heap, return an error. */
	vt = malloc(sizeof(*vt));
	if (vt == NULL) {
		return 1;
	}
	vt->vt_name = name;
	vt->vt_controller = con;
	vt->vt_init = init_func;
	vt->vt_free = free_func;
	vt->vt_draw = draw_func;
	vt->vt_next = NULL;

	/* Add the new view type to the list. */
	if (view_types_head == NULL) {
		view_types_head = vt;
		view_types_tail = vt;
	} else {
		view_types_tail->vt_next = vt;
		view_types_tail = vt;
	}

	return 0;
}

void
view_types_init()
{
	view_type_menu_init();
	view_type_contacts_init();
	view_type_edit_contact_init();
	view_type_chat_init();
	view_type_log_init();
}

struct view *
view_init(char *type)
{
	struct view *v;
	struct view_type *vt;

	vt = view_type_get(type);
	if (vt == NULL) {
		return NULL;
	}

	v = vt->vt_init();
	if (v == NULL) {
		return NULL;
	}
	v->v_type = vt;

	return v;
}

void
view_free(struct view *v)
{
	v->v_type->vt_free(v);
}

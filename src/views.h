/*
 * src/views.h
 * Views system interface.
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

#ifndef __VIEWS_H
#define __VIEWS_H

#include <stdarg.h>
#include "controllers.h"
#include "contact.h"
#include "chat_event.h"

struct view {
	struct view_type *v_type;
	struct view *v_down;
	struct tab *v_tab;
};

enum view_log_type {
	VLT_DEBUG,
	VLT_INFO
};

struct view_type;

struct view_menu;
struct view_contacts;
struct view_edit_contact;
struct view_chat;

void view_type_menu_init();
void view_type_contacts_init();
void view_type_edit_contact_init();
void view_type_chat_init();
void view_type_log_init();

void view_contacts_handle_event(struct contact *c, struct chat_event *ce);
void view_edit_contact_set_contact(struct view *v, struct contact *c);
void view_chat_set_contact(struct view *v, char *c);
struct view *view_chat_find_by_contact(char *c);
void view_chat_handle_event(struct contact *c, struct chat_event *ce);
void view_log_print(enum view_log_type type, char *fmt, ...);
void view_debug_print(char *fmt, ...);

/*
 * Draws the currently active view.
 * Parameters:
 *   None.
 * Return value:
 *   None.
 */
void view_draw_current();

/*
 * Changes the active view.
 * Parameters:
 *   v - The new active view.
 * Return value:
 *   None.
 */
void view_set_current(struct view *v);

/*
 * Checks if a certain view is the active view.
 * Parameters:
 *   v - The view to check.
 * Return value:
 *   0 if v is not the active view.
 *   A non-zero value if v is the active view.
 */
int view_is_current(struct view *v);

/*
 * Registers a new view type.
 * Parameters:
 *   name - The name of the view type.
 *   con - The local controller used by views of this type.
 *   init_func - Function to initialize view structs.
 *   free_func - Function to clean up view structs.
 *   draw_func - Function to draw views.
 * Return value:
 *   None.
 */
int view_type_register(char *name, struct controller *con,
		struct view *(*init_func)(void), void (*free_func)(struct view *),
		void (*draw_func)(struct view *));

/*
 * Initializes all view types.
 * Parameters:
 *   None.
 * Return value:
 *   None.
 */
void view_types_init();

/*
 * Allocates and initializes a view.
 * Parameters:
 *   type - The type of view to initialize.
 * Return value:
 *   The new view.
 */
struct view *view_init(char *type);

/*
 * Frees a view.
 * Parameters:
 *   v - The view to free.
 * Return value:
 *   None.
 */
void view_free(struct view *v);

#endif /* #ifndef __VIEWS_H */

/*
 * src/views/edit-contact.c
 * Edit contact view.
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
#include <curses.h>
#include "../views.h"
#include "../controllers.h"
#include "../windows.h"
#include "../translation.h"

struct view_edit_contact {
	struct view_type *vec_type;
	struct view *vec_down;
	struct tab *vec_tab;
};

static struct view *view_edit_contact_init();
static void view_edit_contact_free(struct view *v);
static void view_edit_contact_draw(struct view *v);

void
view_type_edit_contact_init()
{
	struct controller *con;

	con = controller_init();

	view_type_register("edit_contact", con,
			&view_edit_contact_init, &view_edit_contact_free, &view_edit_contact_draw);
}

static struct view *
view_edit_contact_init()
{
	struct view_edit_contact *v;

	v = malloc(sizeof(*v));

	return (struct view *) v;
}

static void
view_edit_contact_free(struct view *v)
{
	free((struct view_edit_contact *) v);
}

static void
view_edit_contact_draw(struct view *v)
{
	static int init_done = 0;
	static char *title;
	static int title_halflen;

	if (!init_done) {
		title = _("Editing contact");
		title_halflen = strlen(title) / 2;
		init_done = 1;
	}

	wclear(window_main);

	mvprintw(1, COLS / 2 - title_halflen, title);

	/* TODO: Add stuff. */

	wrefresh(window_main);
}

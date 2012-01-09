/*
 * src/views/contacts.c
 * Contacts view.
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
#include "../tabs.h"
#include "../contact.h"
#include "../chat.h"
#include "../translation.h"
#include "../constants.h"

struct view_contacts {
	struct view_type *vc_type;
	struct view *vc_down;
	struct tab *vc_tab;
};

static struct view_contacts *view_contacts_view;
static struct contact *view_contacts_contact_current;

static struct view *view_contacts_init();
static void view_contacts_free(struct view *v);
static void view_contacts_draw(struct view *v);

static int view_contacts_key_enter(int k);
static int view_contacts_key_tab(int k);
static int view_contacts_key_del(int k);
static int view_contacts_key_up(int k);
static int view_contacts_key_down(int k);
static int view_contacts_key_home(int k);
static int view_contacts_key_end(int k);

void
view_type_contacts_init()
{
	struct controller *con;

	con = controller_init();
	controller_add_key(con, '\n', &view_contacts_key_enter);
	controller_add_key(con, '\t', &view_contacts_key_tab);
	controller_add_key(con, KEY_DC, &view_contacts_key_del);
	controller_add_key(con, KEY_UP, &view_contacts_key_up);
	controller_add_key(con, 'k', &view_contacts_key_up);
	controller_add_key(con, KEY_DOWN, &view_contacts_key_down);
	controller_add_key(con, 'j', &view_contacts_key_down);
	controller_add_key(con, KEY_HOME, &view_contacts_key_home);
	controller_add_key(con, KEY_END, &view_contacts_key_end);

	view_type_register("contacts", con,
			&view_contacts_init, &view_contacts_free, &view_contacts_draw);
}

void
view_contacts_handle_event(struct contact *c, struct chat_event *ce)
{
	if (ce->ce_type == CONNECTED) {
		if (view_contacts_contact_current == NULL) {
			contact_get_first(&view_contacts_contact_current);
		}
	}

	if (view_is_current((struct view *) view_contacts_view)) {
		view_draw_current();
	}
}

static struct view *
view_contacts_init()
{
	struct view_contacts *v;

	v = malloc(sizeof(*v));
	view_contacts_view = v;

	contact_get_first(&view_contacts_contact_current);

	return (struct view *) v;
}

static void
view_contacts_free(struct view *v)
{
	free((struct view_contacts *) v);
}

static void
view_contacts_draw(struct view *v)
{
	static int init_done = 0;
	static char *title;
	static char *label_keys1;
	static char *label_keys2;
	static int title_halflen;
	static int label_keys1_len;
	static int label_keys2_len;
	int i, j, win_maxy, win_maxx;
	struct contact *con;

	if (!init_done) {
		title = _("Contacts");
		label_keys1 = _("Up/Down/j/k: move up or down  "
				"Home/End: jump to the start or the end");
		label_keys2 = _("Enter: chat with contact  Tab: edit contact  "
				"Delete: delete contact  Insert: add new contact");
		title_halflen = strlen(title) / 2;
		label_keys1_len = strlen(label_keys1);
		label_keys2_len = strlen(label_keys2);
		init_done = 1;
	}

	wclear(window_main);

	curs_set(0);
	getmaxyx(window_main, win_maxy, win_maxx);

	mvwprintw(window_main, 0, win_maxx / 2 - title_halflen, title);

	for (contact_get_first(&con), i = 3;
			con != NULL;
			contact_get_next(&con), ++i) {
		if (con == view_contacts_contact_current) {
			wattron(window_main, A_REVERSE);
			for (j = 0; j < ENTRY_NICKNAME_SIZE; ++j) {
				mvwprintw(window_main, i, j + 2, " ");
			}
			mvwprintw(window_main, i, 0,
					"%c %s", con->c_states[CONNECTED] ? '+' : '-', con->c_nickname);
			wattroff(window_main, A_REVERSE);
		} else {
			mvwprintw(window_main, i, 0,
					"%c %s", con->c_states[CONNECTED] ? '+' : '-', con->c_nickname);
		}
	}

	mvwprintw(window_main, win_maxy - 2, win_maxx - label_keys1_len,
			label_keys1);
	mvwprintw(window_main, win_maxy - 1, win_maxx - label_keys2_len,
			label_keys2);

	wrefresh(window_main);
}

static int
view_contacts_key_enter(int k)
{
	struct view *v;
	struct tab *t;

	v = view_chat_find_by_contact(view_contacts_contact_current->c_key);
	if (v == NULL) {
		t = tab_new(view_contacts_contact_current->c_nickname);
		v = view_init("chat");
		view_chat_set_contact(v, view_contacts_contact_current->c_key);
		tab_push_view(t, v);
		tab_show(t);
	} else {
		tab_show(v->v_tab);
	}
	chat_connect(view_contacts_contact_current);

	return CONTROLLERS_STOP;
}

static int
view_contacts_key_tab(int k)
{
	struct view_edit_contact *v;

	v = (struct view_edit_contact *) view_init("edit_contact");
	/* TODO: Fix this. */
//	view_edit_contact_set_contact((struct view *) v, view_contacts_contact_current);
	tab_push_view(tab_current, (struct view *) v);

	return CONTROLLERS_STOP;
}

static int
view_contacts_key_del(int k)
{
	contact_delete(view_contacts_contact_current);
	view_draw_current();

	return CONTROLLERS_STOP;
}

static int
view_contacts_key_up(int k)
{
	if (!contact_is_first(view_contacts_contact_current)) {
		contact_get_prev(&view_contacts_contact_current);
		view_draw_current();
	}

	return CONTROLLERS_STOP;
}

static int
view_contacts_key_down(int k)
{
	if (!contact_is_last(view_contacts_contact_current)) {
		contact_get_next(&view_contacts_contact_current);
		view_draw_current();
	}

	return CONTROLLERS_STOP;
}

static int
view_contacts_key_home(int k)
{
	contact_get_first(&view_contacts_contact_current);
	view_draw_current();

	return CONTROLLERS_STOP;
}

static int
view_contacts_key_end(int k)
{
	contact_get_last(&view_contacts_contact_current);
	view_draw_current();

	return CONTROLLERS_STOP;
}

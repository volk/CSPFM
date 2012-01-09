/*
 * src/views/chat.c
 * Chat view.
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
#include "../text-input.h"
#include "../contact.h"
#include "../chat.h"
#include "../translation.h"
#include "../constants.h"

enum view_chat_event_type {
	VIEW_CHAT_EVENT_CHAT,
	VIEW_CHAT_EVENT_CONN,
	VIEW_CHAT_EVENT_DISC
};

struct view_chat_message {
	char *vcm_party;
	enum view_chat_event_type vcm_type;
	char *vcm_message;
	int vcm_msg_len;
	int vcm_msg_lines;
	/* TODO: Record time. */
	struct view_chat_message *vcm_prev;
	struct view_chat_message *vcm_next;
};

struct view_chat {
	struct view_type *vc_type;
	struct view *vc_down;
	struct tab *vc_tab;
	struct text_input *vc_msg_input;
	char *vc_contact;
	struct view_chat_message *vc_msg_tail;
	struct view_chat_message *vc_msg_scroll_tail;
	struct view_chat *vc_prev;
	struct view_chat *vc_next;
};

static struct view_chat *view_chat_head, *view_chat_tail;

static struct view *view_chat_init();
static void view_chat_free(struct view *v);
static void view_chat_draw(struct view *v);
static int view_chat_message_wrap(char *msg, int len, int width);
static int view_chat_key_any(int k);
static int view_chat_key_enter(int k);
static int view_chat_key_up(int k);
static int view_chat_key_down(int k);

void
view_type_chat_init()
{
	struct controller *con;

	con = controller_init();
	controller_add_key(con, CONTROLLER_KEY_ANY, &view_chat_key_any);
	controller_add_key(con, '\n', &view_chat_key_enter);
	controller_add_key(con, KEY_UP, &view_chat_key_up);
	controller_add_key(con, KEY_DOWN, &view_chat_key_down);

	view_type_register("chat", con,
			&view_chat_init, &view_chat_free, &view_chat_draw);

	view_chat_head = NULL;
	view_chat_tail = NULL;
}

void
view_chat_set_contact(struct view *v, char *c)
{
	if (v == NULL) {
		return;
	}

	((struct view_chat *) v)->vc_contact = (c == NULL ? c : strdup(c));
}

void
view_chat_handle_event(struct contact *c, struct chat_event* ce)
{
	struct view *v;
	struct view_chat *vc;
	struct tab *t;
	int width, win_maxy, win_maxx;
	struct view_chat_message *vcm;

	if (c == NULL || ce == NULL) {
		return;
	}

	getmaxyx(window_main, win_maxy, win_maxx);

	view_debug_print("CHAT VIEW: Message of type %d from contact '%s' ('%s'): "
			"'%s'", ce->ce_type, c->c_nickname, c->c_key, ce->ce_message);

	/* TODO: The chat subsystem should do this. */
	view_contacts_handle_event(c, ce);

	v = NULL;

	if (ce->ce_type == CONTACT_CHANGED) {
		v = view_chat_find_by_contact(c->c_key);
		if (v == NULL) {
			return;
		}

		tab_set_label(v->v_tab, c->c_nickname);
		window_tabbar_draw();

		goto draw;
	}

	/* Find the appropriate chat view. */
	if (ce->ce_type == BROADCAST) {
		v = view_chat_find_by_contact(NULL);
		if (v == NULL) {
			return;
		}
	} else {
		v = view_chat_find_by_contact(c->c_key);
		if (v == NULL) {
			t = tab_new(c->c_nickname);
			v = view_init("chat");
			view_chat_set_contact(v, c->c_key);
			tab_push_view(t, v);
		}
	}

	/* Access the view as a chat view. */
	vc = (struct view_chat *) v;

	/* Allocate and initialize a message. */
	vcm = malloc(sizeof(*vcm));
	if (vcm == NULL) {
		return;
	}
	vcm->vcm_party = strdup(c->c_nickname);
	vcm->vcm_prev = vc->vc_msg_tail;
	if (vc->vc_msg_tail != NULL) {
		vc->vc_msg_tail->vcm_next = vcm;
	}
	if (vc->vc_msg_scroll_tail == vc->vc_msg_tail) {
		vc->vc_msg_scroll_tail = vcm;
	}
	vc->vc_msg_tail = vcm;

	/* Fill in message details */
	if (ce->ce_type == CONNECTED) {
		vcm->vcm_type = VIEW_CHAT_EVENT_CONN;
		vcm->vcm_msg_lines = 1;
	} else if (ce->ce_type == DISCONNECTED) {
		vcm->vcm_type = VIEW_CHAT_EVENT_DISC;
		vcm->vcm_msg_lines = 1;
	} else {
		vcm->vcm_type = VIEW_CHAT_EVENT_CHAT;
		vcm->vcm_message = strdup(ce->ce_message);
		vcm->vcm_msg_len = strlen(vcm->vcm_message);
		width = win_maxx - 3 - strlen(vcm->vcm_party);
		vcm->vcm_msg_lines = view_chat_message_wrap(vcm->vcm_message,
				vcm->vcm_msg_len, width);
	}

 draw:
	if (view_is_current(v)) {
		view_draw_current();
	}
}

struct view *
view_chat_find_by_contact(char *c)
{
	struct view_chat *vc;

	for (vc = view_chat_head; vc != NULL; vc = vc->vc_next) {
		if (vc->vc_contact == NULL || c == NULL) {
			if (vc->vc_contact == c) {
				break;
			}
		} else if (strcmp(vc->vc_contact, c) == 0) {
			break;
		}
	}

	return (struct view *) vc;
}

static struct view *
view_chat_init()
{
	struct view_chat *v;

	v = malloc(sizeof(*v));
	if (v == NULL) {
		return NULL;
	}

	v->vc_msg_input = text_input_new(256);
	if (v->vc_msg_input == NULL) {
		free(v);
		return NULL;
	}
	v->vc_contact = NULL;
	v->vc_msg_tail = NULL;
	v->vc_msg_scroll_tail = NULL;

	v->vc_next = NULL;
	v->vc_prev = view_chat_tail;
	if (view_chat_head != NULL) {
		view_chat_tail->vc_next = v;
		view_chat_tail = v;
	} else {
		view_chat_head = v;
		view_chat_tail = v;
	}

	return (struct view *) v;
}

static void
view_chat_free(struct view *v)
{
	/* TODO: Delete the chat view from the list of chat views. */
	free((struct view_chat *) v);
}

static void
view_chat_draw(struct view *v)
{
	static int init_done = 0;
	static char *label_keys;
	static int label_keys_len;
	int win_maxy, win_maxx, i;
	struct view_chat *vc;
	struct view_chat_message *msg;

	if (!init_done) {
		label_keys = _("Up/Down: scroll up or down");
		label_keys_len = strlen(label_keys);
		init_done = 1;
	}

	vc = (struct view_chat *) v;

	wclear(window_main);

	curs_set(1);

	getmaxyx(window_main, win_maxy, win_maxx);

	for (i = win_maxy - 4, msg = vc->vc_msg_scroll_tail;
			i >= 0 && msg != NULL;
			i -= msg->vcm_msg_lines, msg = msg->vcm_prev) {
		if (msg->vcm_type == VIEW_CHAT_EVENT_CHAT) {
			mvwprintw(window_main, i - msg->vcm_msg_lines, 0, "<%s> %s",
					msg->vcm_party, msg->vcm_message);
		} else if (msg->vcm_type == VIEW_CHAT_EVENT_CONN) {
			mvwprintw(window_main, i - msg->vcm_msg_lines, 0,
					_("%s has connected."), msg->vcm_party);
		} else if (msg->vcm_type == VIEW_CHAT_EVENT_DISC) {
			mvwprintw(window_main, i - msg->vcm_msg_lines, 0,
					_("%s has disconnected."), msg->vcm_party);
		}
	}

	mvwhline(window_main, win_maxy - 2, 0, ACS_HLINE, win_maxx);
	mvwprintw(window_main, win_maxy - 1, win_maxx - label_keys_len,
			label_keys);

	mvwhline(window_main, win_maxy - 4, 0, ACS_HLINE, win_maxx);
	mvwprintw(window_main, win_maxy - 3, 0, "%s",
			text_input_get_buffer(vc->vc_msg_input));
	wmove(window_main,
			win_maxy - 3,
			0 + text_input_get_cursor_position(vc->vc_msg_input));

	wrefresh(window_main);
}

static int
view_chat_message_wrap(char *msg, int len, int width)
{
	int i, lines;

	lines = 1;

	/* While the line is longer than width. */
	while (len > width) {
		/* Start at the end and go left. */
		for (i = width; i >= 0; --i) {
			if (msg[i] == ' ' || msg[i] == '\t' || msg[i] == '\n') {
				msg += i + 1;
				len -= i + 1;
				++lines;
				break;
			}
		}
	}

	return lines;
}

static int
view_chat_key_any(int k)
{
	struct view_chat *this;
	int win_maxy, win_maxx, i;

	this = (struct view_chat *) tab_peek_view(tab_current);
	getmaxyx(window_main, win_maxy, win_maxx);

	if (text_input_edit(this->vc_msg_input, k) == CONTROLLERS_STOP) {
		for (i = 0; i < win_maxx; ++i) {
			mvwprintw(window_main, win_maxy - 3, i, " ");
		}
		mvwprintw(window_main, win_maxy - 3, 0, "%s",
				text_input_get_buffer(this->vc_msg_input));
		wmove(window_main,
				win_maxy - 3,
				text_input_get_cursor_position(this->vc_msg_input));
		wrefresh(window_main);
		return CONTROLLERS_STOP;
	} else {
		return CONTROLLERS_CONTINUE;
	}
}

static int
view_chat_key_enter(int k)
{
	struct view_chat *this;
	int win_maxy, win_maxx, width;
	struct view_chat_message *vcm;

	this = (struct view_chat *) tab_peek_view(tab_current);
	getmaxyx(window_main, win_maxy, win_maxx);

	vcm = malloc(sizeof(*vcm));
	if (vcm == NULL) {
		return 0xDEADBEEF;
	}
	vcm->vcm_party = chat_server_nick();
	vcm->vcm_type = VIEW_CHAT_EVENT_CHAT;
	vcm->vcm_message = strdup(text_input_get_buffer(this->vc_msg_input));
	if (vcm->vcm_message[0] == '\0') {
		free(vcm);
		goto out;
	}
	vcm->vcm_msg_len = strlen(vcm->vcm_message);
	/* TODO: Account for length of other party's nickname and of a timestamp. */
	width = COLS - 5 - strlen(chat_server_nick());
	vcm->vcm_msg_lines = view_chat_message_wrap(vcm->vcm_message,
			vcm->vcm_msg_len, width);
	vcm->vcm_prev = this->vc_msg_tail;
	if (this->vc_msg_tail != NULL) {
		this->vc_msg_tail->vcm_next = vcm;
	}
	if (this->vc_msg_scroll_tail == this->vc_msg_tail) {
		this->vc_msg_scroll_tail = vcm;
	}
	this->vc_msg_tail = vcm;

	if (this->vc_contact == NULL) {
		chat_send_broadcast(text_input_get_buffer(this->vc_msg_input));
	} else {
		chat_send_message(this->vc_contact, 
				text_input_get_buffer(this->vc_msg_input));
	}

	text_input_clear(this->vc_msg_input);

	view_draw_current();

 out:
	return CONTROLLERS_STOP;
}

static int
view_chat_key_up(int k)
{
	struct view_chat *this;

	this = (struct view_chat *) tab_peek_view(tab_current);

	if (this->vc_msg_scroll_tail->vcm_prev != NULL) {
		this->vc_msg_scroll_tail = this->vc_msg_scroll_tail->vcm_prev;
	}

	view_draw_current();

	return CONTROLLERS_STOP;
}

static int
view_chat_key_down(int k)
{
	struct view_chat *this;

	this = (struct view_chat *) tab_peek_view(tab_current);

	if (this->vc_msg_scroll_tail->vcm_next != NULL) {
		this->vc_msg_scroll_tail = this->vc_msg_scroll_tail->vcm_next;
	}

	view_draw_current();

	return CONTROLLERS_STOP;
}

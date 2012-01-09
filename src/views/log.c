/*
 * src/views/log.c
 * Log view.
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
#include <stdarg.h>
#include <curses.h>
#include "../views.h"
#include "../windows.h"

struct view_log {
	struct view_type *vm_type;
	struct view *vm_down;
	struct tab *vm_tab;
};

struct view_log_msg {
	enum view_log_type vlm_type;
	char *vlm_msg;
	struct view_log_msg *vlm_prev;
};

static struct view_log *view_log_view;
static struct view_log_msg *view_log_msg_tail;

static struct view *view_log_init();
static void view_log_free(struct view *v);
static void view_log_draw(struct view *v);

void
view_type_log_init()
{
	struct controller *con;

	con = controller_init();

	view_type_register("log", con,
			&view_log_init, &view_log_free, &view_log_draw);
}

static struct view *
view_log_init()
{
	struct view_log *v;

	v = malloc(sizeof(*v));
	view_log_view = v;

	view_log_msg_tail = NULL;

	return (struct view *) v;
}

static void
view_log_free(struct view *v)
{
	free((struct view_log *) v);
}

static void
view_log_draw(struct view *v)
{
	int win_maxy, win_maxx, i;
	struct view_log_msg *msg;

	wclear(window_main);

	curs_set(0);
	getmaxyx(window_main, win_maxy, win_maxx);

	for (i = win_maxy - 1, msg = view_log_msg_tail;
			i >= 0 && msg != NULL;
			msg = msg->vlm_prev) {
#ifndef DEBUG
		if (msg->vlm_type != VLT_DEBUG) {
#endif
			wmove(window_main, i--, 0);
			switch (msg->vlm_type) {
				case VLT_DEBUG:
					wprintw(window_main, "DEBUG: %s", msg->vlm_msg);
					break;
				case VLT_INFO:
					wprintw(window_main, "%s", msg->vlm_msg);
					break;
			}
#ifndef DEBUG
		}
#endif
	}

	wrefresh(window_main);
}

void
view_log_print(enum view_log_type type, char *fmt, ...)
{
	va_list args;
	struct view_log_msg *vlm;

	vlm = malloc(sizeof(*vlm));
	vlm->vlm_type = type;
	vlm->vlm_msg = malloc(256);

	va_start(args, fmt);
	vsprintf(vlm->vlm_msg, fmt, args);
	va_end(args);

	if (view_log_msg_tail != NULL) {
		vlm->vlm_prev = view_log_msg_tail;
	} else {
		vlm->vlm_prev = NULL;
	}
	view_log_msg_tail = vlm;

	if (view_is_current((struct view *) view_log_view)) {
		view_draw_current();
	}
}

void
view_debug_print(char *fmt, ...)
{
	va_list args;
	struct view_log_msg *vlm;

	vlm = malloc(sizeof(*vlm));
	vlm->vlm_type = VLT_DEBUG;
	vlm->vlm_msg = malloc(256);

	va_start(args, fmt);
	vsprintf(vlm->vlm_msg, fmt, args);
	va_end(args);

	if (view_log_msg_tail != NULL) {
		vlm->vlm_prev = view_log_msg_tail;
	} else {
		vlm->vlm_prev = NULL;
	}
	view_log_msg_tail = vlm;

	if (view_is_current((struct view *) view_log_view)) {
		view_draw_current();
	}
}

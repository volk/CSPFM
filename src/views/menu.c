/*
 * src/views/menu.c
 * Menu view.
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

struct view_menu {
	struct view_type *vm_type;
	struct view *vm_down;
	struct tab *vm_tab;
};

static struct view *view_menu_init();
static void view_menu_free(struct view *v);
static void view_menu_draw(struct view *v);

static int view_menu_key_enter(int k);
static int view_menu_key_up(int k);
static int view_menu_key_down(int k);

void
view_type_menu_init()
{
	struct controller *con;

	con = controller_init();
	controller_add_key(con, '\n', &view_menu_key_enter);
	controller_add_key(con, KEY_UP, &view_menu_key_up);
	controller_add_key(con, 'k', &view_menu_key_up);
	controller_add_key(con, KEY_DOWN, &view_menu_key_down);
	controller_add_key(con, 'j', &view_menu_key_down);

	view_type_register("menu", con,
			&view_menu_init, &view_menu_free, &view_menu_draw);
}

static struct view *
view_menu_init()
{
	struct view_menu *v;

	v = malloc(sizeof(*v));

	return (struct view *) v;
}

static void
view_menu_free(struct view *v)
{
	free((struct view_menu *) v);
}

static void
view_menu_draw(struct view *v)
{
	static int init_done = 0;
	static const char *title;
	static char *label_quit;
	static char *label_keys;
	static int title_halflen;
	static int label_quit_halflen;
	static int label_keys_len;
	int win_maxy, win_maxx;

	if (!init_done) {
		title = _("Menu");
		label_quit = _("Quit");
		label_keys = _("Up/Down/j/k: move up or down  Enter: select");
		title_halflen = strlen(title) / 2;
		label_quit_halflen = strlen(label_quit) / 2;
		label_keys_len = strlen(label_keys);
		init_done = 1;
	}

	wclear(window_main);

	curs_set(0);
	getmaxyx(window_main, win_maxy, win_maxx);

	mvwprintw(window_main, 0, win_maxx / 2 - title_halflen, title);

	wattron(window_main, A_REVERSE);
	mvwprintw(window_main, 3, win_maxx / 2 - 20,
			"                                        ");
	mvwprintw(window_main, 3, win_maxx / 2 - label_quit_halflen, label_quit);
	wattroff(window_main, A_REVERSE);

	mvwprintw(window_main, win_maxy - 1, win_maxx - label_keys_len, label_keys);

	wrefresh(window_main);
}

static int
view_menu_key_enter(int k)
{
	/* TODO: Maybe there should be a general exit function that might do clean
	 * some up. */
	delwin(window_tabbar);
	delwin(window_main);
	endwin();
	exit(0);
}

static int
view_menu_key_up(int k)
{
	return CONTROLLERS_STOP;
}

static int
view_menu_key_down(int k)
{
	return CONTROLLERS_STOP;
}

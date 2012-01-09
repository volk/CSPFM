/*
 * src/windows.c
 * Windows system.
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

#include <string.h>
#include <curses.h>
#include "windows.h"
#include "tabs.h"
#include "translation.h"

void
windows_init()
{
	initscr();
	cbreak();
	noecho();
	refresh();
	keypad(stdscr, TRUE);

	window_tabbar = newwin(3, COLS, LINES - 3, 0);
	window_main = newwin(LINES - 4, COLS - 2, 1, 1);
}

void
window_tabbar_draw()
{
	static int init_done = 0;
	static char *label_keys;
	static int label_keys_len;
	struct tab *t;
	int y, x;

	if (!init_done) {
		label_keys = _("F5/F6: switch tabs");
		label_keys_len = strlen(label_keys);
		init_done = 1;
	}

	getyx(window_main, y, x);

	wclear(window_tabbar);

	wborder(window_tabbar, ' ', ' ', ACS_HLINE, ' ', ACS_HLINE, ACS_HLINE, ' ', ' ');

	wmove(window_tabbar, 1, 0);

	/* TODO: What if the user opens so many tabs that the tab bar overflows the
	 * screen? */
	for (tab_get_first(&t); t != NULL; tab_get_next_linear(&t)) {
		if (tab_is_current(t)) {
			wprintw(window_tabbar, " ");
			wattron(window_tabbar, A_REVERSE);
			wprintw(window_tabbar, "%s", tab_get_label(t));
			wattroff(window_tabbar, A_REVERSE);
			wprintw(window_tabbar, " ");
		} else {
			wprintw(window_tabbar, " %s ", tab_get_label(t));
		}
	}
	mvwprintw(window_tabbar, 1, COLS - label_keys_len - 1, label_keys);

	wrefresh(window_tabbar);

	wmove(window_main, y, x);
	wrefresh(window_main);
}

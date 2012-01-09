/*
 * src/tabs.h
 * Tabs system interface.
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

#ifndef __TABS_H
#define __TABS_H

#include "views.h"

struct tab;

struct tab *tab_current;

void tabs_init();

struct tab *tab_new(const char *label);

void tab_close(struct tab *t);

char *tab_get_label(struct tab *t);

void tab_set_label(struct tab *t, const char *label);

void tab_push_view(struct tab *t, struct view *v);

struct view *tab_pop_view(struct tab *t);

struct view *tab_peek_view(struct tab *t);

void tab_show_prev();

void tab_show_next();

void tab_show(struct tab *t);

void tab_get_first(struct tab **t);

void tab_get_last(struct tab **t);

int tab_is_first(struct tab *t);

int tab_is_last(struct tab *t);

int tab_is_current(struct tab *t);

void tab_get_next(struct tab **t);

void tab_get_next_linear(struct tab **t);

void tab_get_prev(struct tab **t);

void tab_get_prev_linear(struct tab **t);

#endif /* #ifndef __TABS_H */

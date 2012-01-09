/*
 * src/tabs.c
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

#include <stdlib.h>
#include <string.h>
#include "tabs.h"
#include "views.h"
#include "windows.h"
#include "translation.h"

struct tab {
	char *t_label;
	struct view *t_views_top;
	struct tab *t_prev, *t_next;
};

static struct tab *tabs_head, *tabs_tail;

void
tabs_init()
{
	char *tab_label_menu = _("Menu");
	char *tab_label_contacts = _("Contacts");
	char *tab_label_broadcast = _("Global");
	struct tab *t;
	struct view *v;

	t = tab_new("Log");
	v = view_init("log");
	tab_push_view(t, v);
	view_log_print(VLT_INFO, "Begin logging...");

	t = tab_new(tab_label_menu);
	v = view_init("menu");
	tab_push_view(t, v);
	tab_show(t);

	t = tab_new(tab_label_contacts);
	v = view_init("contacts");
	tab_push_view(t, v);

	t = tab_new(tab_label_broadcast);
	v = view_init("chat");
	view_chat_set_contact(v, NULL);
	tab_push_view(t, v);
}

struct tab *
tab_new(const char *label)
{
	struct tab *t;

	t = malloc(sizeof(*t));
	if (t == NULL) {
		return NULL;
	}
	t->t_label = strdup(label);
	t->t_views_top = NULL;

	if (tabs_head == NULL) {
		tabs_head = t;
		tabs_tail = t;
		t->t_prev = t;
		t->t_next = t;
		tab_current = t;
	} else {
		tabs_tail->t_next = t;
		tabs_head->t_prev = t;
		t->t_prev = tabs_tail;
		t->t_next = tabs_head;
		tabs_tail = t;
	}

	window_tabbar_draw();

	return t;
}

void
tab_close(struct tab *t)
{
	if (t == NULL) {
		return;
	}

	t->t_prev->t_next = t->t_next;
	t->t_next->t_prev = t->t_prev;

	if (tabs_tail == t) {
		tabs_tail = t->t_prev;
	}
	if (tabs_head == t) {
		tabs_head = t->t_next;
	}

	if (tabs_head == tabs_tail) {
		tab_current = tabs_tail = tabs_head = NULL;
	}

	while (tab_pop_view(t) != NULL);

	free(t->t_label);

	if (tab_current == t) {
		tab_show_prev();
	}

	free(t);

	window_tabbar_draw();
}

char *
tab_get_label(struct tab *t)
{
	if (t == NULL) {
		return NULL;
	}

	return t->t_label;
}

void
tab_set_label(struct tab *t, const char *label)
{
	if (t == NULL) {
		return;
	}

	free(t->t_label);

	t->t_label = strdup(label);

	window_tabbar_draw();
}

void
tab_push_view(struct tab *t, struct view *v)
{
	if (t == NULL) {
		return;
	}

	if (v == NULL) {
		return;
	}

	v->v_down = t->t_views_top;
	t->t_views_top = v;

	v->v_tab = t;

	if (t == tab_current) {
		view_set_current(tab_current->t_views_top);
		view_draw_current();
	}
}

struct view *
tab_pop_view(struct tab *t)
{
	struct view *v;

	if (t == NULL) {
		return NULL;
	}

	v = t->t_views_top;
	if (v == NULL) {
		return NULL;
	}

	t->t_views_top = t->t_views_top->v_down;

	v->v_tab = NULL;

	if (t == tab_current) {
		view_set_current(tab_current->t_views_top);
		view_draw_current();
	}

	return v;
}

struct view *
tab_peek_view(struct tab *t)
{
	if (t == NULL) {
		return NULL;
	}

	return t->t_views_top;
}

void tab_show_prev()
{
	if (tab_current == NULL) {
		return;
	}

	tab_current = tab_current->t_prev;

	view_set_current(tab_current->t_views_top);
	view_draw_current();

	window_tabbar_draw();
}

void tab_show_next()
{
	if (tab_current == NULL) {
		return;
	}

	tab_current = tab_current->t_next;

	view_set_current(tab_current->t_views_top);
	view_draw_current();

	window_tabbar_draw();
}

void tab_show(struct tab *t)
{
	if (t == NULL) {
		return;
	}

	tab_current = t;

	view_set_current(tab_current->t_views_top);
	view_draw_current();

	window_tabbar_draw();
}

void
tab_get_first(struct tab **t)
{
	if (t == NULL) {
		return;
	}

	*t = tabs_head;
}

void
tab_get_last(struct tab **t)
{
	if (t == NULL) {
		return;
	}

	*t = tabs_tail;
}

int
tab_is_first(struct tab *t)
{
	return t == tabs_head;
}

int
tab_is_last(struct tab *t)
{
	return t == tabs_tail;
}

int
tab_is_current(struct tab *t)
{
	return t == tab_current;
}

void
tab_get_next(struct tab **t)
{
	if (t == NULL) {
		return;
	}

	*t = (*t)->t_next;
}

void
tab_get_next_linear(struct tab **t)
{
	if (t == NULL || *t == NULL) {
		return;
	}

	if (*t == tabs_tail) {
		*t = NULL;
	} else {
		*t = (*t)->t_next;
	}
}

void
tab_get_prev(struct tab **t)
{
	if (t == NULL) {
		return;
	}

	*t = (*t)->t_prev;
}

void
tab_get_prev_linear(struct tab **t)
{
	if (t == NULL || *t == NULL) {
		return;
	}

	if (*t == tabs_tail) {
		*t = NULL;
	} else {
		*t = (*t)->t_prev;
	}
}

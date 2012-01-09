/*
 * src/text-input.c
 * Line editing system.
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
#include <ctype.h>
#include <curses.h>
#include "text-input.h"
#include "controllers.h"

struct text_input {
    char *ti_buffer;
    int ti_max_len;
    int ti_cursor_pos;
};

struct text_input *
text_input_new(int len)
{
	struct text_input *ti;

	if (len <= 0) {
		return NULL;
	}

	ti = malloc(sizeof(*ti));
	if (ti == NULL) {
		return NULL;
	}
	ti->ti_buffer = malloc(sizeof(*(ti->ti_buffer)) * len);
	if (ti->ti_buffer == NULL) {
		free(ti);
		return NULL;
	}
	ti->ti_buffer[0] = '\0';
	ti->ti_max_len = len;
	ti->ti_cursor_pos = 0;

	return ti;
}

void
text_input_free(struct text_input *ti)
{
	if (ti == NULL) {
		return;
	}

	free(ti->ti_buffer);
	free(ti);
}

int
text_input_edit(struct text_input *ti, int key)
{
	int i;
	char buf, input;

	if (key == '\n' || key == '\r') {
		return CONTROLLERS_CONTINUE;
	} else if (isprint(key)) {
		for (i = ti->ti_cursor_pos, input = key; i < ti->ti_max_len; ++i) {
			buf = ti->ti_buffer[i];
			ti->ti_buffer[i] = input;
			if (input == '\0') {
				break;
			} else {
				input = buf;
			}
		}
		ti->ti_buffer[ti->ti_max_len - 1] = '\0';
		++ti->ti_cursor_pos;
	} else if (key == KEY_BACKSPACE) {
		if (ti->ti_cursor_pos > 0) {
			for (i = ti->ti_cursor_pos - 1; ; ++i) {
				ti->ti_buffer[i] = ti->ti_buffer[i + 1];
				if (ti->ti_buffer[i] == '\0') {
					break;
				}
			}
			--ti->ti_cursor_pos;
		}
	} else if (key == KEY_DC) {
		if (ti->ti_buffer[ti->ti_cursor_pos] != '\0') {
			for (i = ti->ti_cursor_pos; ; ++i) {
				ti->ti_buffer[i] = ti->ti_buffer[i + 1];
				if (ti->ti_buffer[i] == '\0') {
					break;
				}
			}
		}
	} else if (key == KEY_LEFT) {
		if (ti->ti_cursor_pos > 0) {
			--ti->ti_cursor_pos;
		}
	} else if (key == KEY_RIGHT) {
		if (ti->ti_cursor_pos + 1 < ti->ti_max_len &&
				ti->ti_buffer[ti->ti_cursor_pos] != '\0') {
			++ti->ti_cursor_pos;
		}
	} else if (key == KEY_HOME) {
		ti->ti_cursor_pos = 0;
	} else if (key == KEY_END) {
		while (ti->ti_buffer[ti->ti_cursor_pos] != '\0') {
			++ti->ti_cursor_pos;
		}
	} else {
		return CONTROLLERS_CONTINUE;
	}

	return CONTROLLERS_STOP;
}

char *
text_input_get_buffer(struct text_input *ti)
{
	if (ti == NULL) {
		return NULL;
	}

	return ti->ti_buffer;
}

int
text_input_get_cursor_position(struct text_input *ti)
{
	if (ti == NULL) {
		return -1;
	}

	return ti->ti_cursor_pos;
}

void
text_input_clear(struct text_input *ti)
{
	if (ti == NULL) {
		return;
	}

	ti->ti_buffer[0] = '\0';
	ti->ti_cursor_pos = 0;
}

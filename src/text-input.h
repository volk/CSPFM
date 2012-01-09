/*
 * src/text-input.h
 * Line editing system interface.
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

#ifndef __TEXT_INPUT_H
#define __TEXT_INPUT_H

struct text_input;

struct text_input *text_input_new(int len);

void text_input_free(struct text_input *ti);

int text_input_edit(struct text_input *ti, int key);

char *text_input_get_buffer(struct text_input *ti);

int text_input_get_cursor_position(struct text_input *ti);

void text_input_clear(struct text_input *ti);

#endif /* #ifndef __TEXT_INPUT_H */

/*
 * src/controllers.h
 * Controllers system interface.
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

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

/*
 * A wildcard key value to match any key.
 */
#define CONTROLLER_KEY_ANY 0

/*
 * Constants to be returned by key handlers to indicate whether handling should
 * continue.
 */
#define CONTROLLERS_STOP 0
#define CONTROLLERS_CONTINUE 1

/*
 * The controller structure used by controllers system functions.
 */
struct controller;

/*
 * Initializes the controllers subsystem.
 * Parameters:
 *   None.
 * Return value:
 *   None.
 */
void controllers_init();

/*
 * Allocates and initializes a new controller.
 * Parameters:
 *   None.
 * Return value:
 *   A new controller on success.
 *   NULL on error.
 */
struct controller *controller_init();

/*
 * Frees memory used by a controller.
 * Parameters:
 *   con - The controller to free.
 * Return value:
 *   None.
 */
void controller_free(struct controller *con);

/*
 * Adds a key mapping to a controller.
 * Parameters:
 *   con - The controller to which the key should be added.
 *   key - The key to be added.  CONTROLLER_KEY_ANY will match any key.
 *   handler - The function to be called when the key is pressed.
 * Return value:
 *   0 on success.
 *   1 on memory allocation failure.
 */
int controller_add_key(struct controller *con, int key, int (*handler)(int));

/*
 * Sets the current local/view controller to be used.
 * Parameters:
 *   con - The controller to be used.
 * Return value:
 *   None.
 */
void controllers_set_view_controller(struct controller *con);

/*
 * Poll for and respond to keyboard input.
 * Parameters:
 *   timeout - Maximum time to block for input.  Passed to timeout() of curses.
 * Return value:
 *   None.
 */
void controllers_input_poll(int timeout);

#endif /* #ifndef __CONTROLLER_H */

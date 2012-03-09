/*
 * This file is part of jackeventcmd - headphones jack event handler
 * Copyright (C) 2012 Maxim A. Mikityanskiy - <maxtram95@gmail.com>
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

#include <dbus/dbus.h>
#include <stdio.h>
#include <string.h>

#include "run.h"

static DBusError error;
static DBusConnection *connection;

static void process_event(const char *codename, int state);

int je_dbus_init()
{
	dbus_error_init(&error);

	// Connect
	connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	if (dbus_error_is_set(&error)) {
		fprintf(stderr, "DBus: dbus_bus_get: %s\n", error.message);
		dbus_error_free(&error);
		return 0;
	}
	if (connection == NULL) {
		fprintf(stderr, "DBus: connection to system bus failed\n");
		dbus_error_free(&error);
		return 0;
	}

	// Connect to jacklistener
	dbus_bus_add_match(connection, "type='signal',interface='org.ude.jacklistener'", &error);
	dbus_connection_flush(connection);
	if (dbus_error_is_set(&error)) {
		fprintf(stderr, "DBus: dbus_bus_add_match: %s\n", error.message);
		dbus_connection_close(connection);
		dbus_error_free(&error);
		return 0;
	}

	return 1;
}

void je_dbus_fini()
{
	dbus_connection_close(connection);
	dbus_error_free(&error);
}

void je_dbus_listen_events()
{
	for (;;) {
		// Event loop
		dbus_connection_read_write(connection, -1);
		DBusMessage *msg;
		while ((msg = dbus_connection_pop_message(connection)) != NULL) {
			// Filter signals
			if (dbus_message_is_signal(msg, "org.ude.jacklistener", "statechanged")) {
				DBusMessageIter args;
				if (!dbus_message_iter_init(msg, &args)) {
					goto final;
				}

				// Get first argument
				if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) {
					goto final;
				}
				const char *str;
				dbus_message_iter_get_basic(&args, &str);
				
				// Get second argument
				if (!dbus_message_iter_next(&args)) {
					goto final;
				}
				if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_UINT32) {
					goto final;
				}
				int value;
				dbus_message_iter_get_basic(&args, &value);

				// Process them
				process_event(str, value);
			}
final:
			dbus_message_unref(msg);
		}
	}
}

static void process_event(const char *codename, int state)
{
	int num;
	state = !!state;
	if (!strcmp(codename, "headphone")) {
		num = state;
	} else
	if (!strcmp(codename, "microphone")) {
		num = state+2;
	} else
	if (!strcmp(codename, "lineout")) {
		num = state+4;
	} else
	if (!strcmp(codename, "videoout")) {
		num = state+6;
	} else
	if (!strcmp(codename, "linein")) {
		num = state+8;
	} else {
		num = state+10;
	}
	run_cmds(num);
}

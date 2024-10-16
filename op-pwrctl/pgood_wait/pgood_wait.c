/**
 * Copyright © 2016 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>

/* Copied from phosphor-objmgr to make the code compile. */
static const int mapper_busy_retries = 5;
static const uint64_t mapper_busy_delay_interval_usec = 1000000;

static void quit(int r, void *loop)
{
	sd_event_exit((sd_event *)loop, r);
}

static int callback(sd_bus_message *m, void *user, sd_bus_error *error)
{
	(void) error;

	sd_event *loop = user;
	int r;
	char *property = NULL;

	r = sd_bus_message_skip(m, "s");
	if (r < 0)
	{
		fprintf(stderr, "Error skipping message fields: %s\n",
				strerror(-r));
		quit(r, loop);
		return r;
	}

	r = sd_bus_message_enter_container(m, SD_BUS_TYPE_ARRAY, "{sv}");
	if (r < 0)
	{
		fprintf(stderr, "Error entering container: %s\n",
				strerror(-r));
		quit(r, loop);
		return r;
	}

	while ((r = sd_bus_message_enter_container(
				m,
				SD_BUS_TYPE_DICT_ENTRY,
				"sv")) > 0)
	{
		r = sd_bus_message_read(m, "s", &property);
		if (r < 0)
		{
			fprintf(stderr, "Error reading message: %s\n",
					strerror(-r));
			quit(r, loop);
			return r;
		}

		if (strcmp(property, "pgood"))
			continue;

		quit(0, loop);
		break;
	}

	return 0;
}

static int get_object(sd_bus *conn, const char *obj,
					  sd_bus_message **reply)
{
	sd_bus_message *request = NULL;
	int r, retry = 0;

	r = sd_bus_message_new_method_call(
		conn, &request, "xyz.openbmc_project.ObjectMapper",
		"/xyz/openbmc_project/object_mapper",
		"xyz.openbmc_project.ObjectMapper", "GetObject");
	if (r < 0)
		goto exit;

	r = sd_bus_message_append(request, "s", obj);
	if (r < 0)
		goto exit;
	r = sd_bus_message_append(request, "as", 0, NULL);
	if (r < 0)
		goto exit;

	while (true)
	{
		r = sd_bus_call(conn, request, 0, NULL, reply);
		if (r == -EBUSY || r == -ENOBUFS)
		{
			if (retry >= mapper_busy_retries)
				break;

			usleep(mapper_busy_delay_interval_usec * (1 << retry));
			++retry;
			continue;
		}
		break;
	}

	if (r < 0)
		goto exit;

exit:
	sd_bus_message_unref(request);

	return r;
}

static int get_service(sd_bus *conn, const char *obj, char **service)
{
	sd_bus_message *reply = NULL;
	const char *tmp;
	int r;

	r = get_object(conn, obj, &reply);
	if (r < 0)
		goto exit;

	r = sd_bus_message_enter_container(reply, 0, NULL);
	if (r < 0)
		goto exit;

	r = sd_bus_message_enter_container(reply, 0, NULL);
	if (r < 0)
		goto exit;

	r = sd_bus_message_read(reply, "s", &tmp);
	if (r < 0)
		goto exit;

	*service = strdup(tmp);

exit:
	sd_bus_message_unref(reply);

	return r;
}

int main(int argc, char *argv[])
{
	static const char *matchfmt =
		"type='signal',"
		"interface='org.freedesktop.DBus.Properties',"
		"member='PropertiesChanged',"
		"arg0='org.openbmc.control.Power',"
		"path='%s',"
		"sender='%s'";
	static const char *usage =
		"Usage: %s OBJECTPATH on|off\n";
	static const size_t LEN = 256;

	sd_bus *conn = NULL;
	sd_event *loop = NULL;
	sd_bus_slot *slot = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	char *service = NULL;
	int r, dest = -1, state;
	char match[LEN];

	if (argc < 3)
	{
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	if (!strcmp(argv[2], "on"))
		dest = 1;
	if (!strcmp(argv[2], "off"))
		dest = 0;

	if (dest != 0 && dest != 1)
	{
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	r = sd_bus_default_system(&conn);
	if (r < 0)
	{
		fprintf(stderr, "Error connecting to system bus: %s\n",
				strerror(-r));
		goto finish;
	}

	r = get_service(conn, argv[1], &service);
	if (r < 0)
	{
		fprintf(stderr, "Error obtaining host service: %s\n",
				strerror(-r));
		goto finish;
	}

	r = sd_event_default(&loop);
	if (r < 0)
	{
		fprintf(stderr, "Error obtaining event loop: %s\n",
				strerror(-r));
		goto finish;
	}

	r = sd_bus_attach_event(conn, loop, SD_EVENT_PRIORITY_NORMAL);
	if (r < 0)
	{
		fprintf(stderr, "Failed to attach system "
						"bus to event loop: %s\n",
				strerror(-r));
		goto finish;
	}

	if (strlen(matchfmt) + strnlen(argv[1], LEN) > LEN)
	{
		r = -E2BIG;
		fprintf(stderr, "Error adding match rule: %s\n",
				strerror(-r));
		goto finish;
	}

	sprintf(match, matchfmt, argv[1], service);

	r = sd_bus_add_match(conn,
						 &slot,
						 match,
						 callback,
						 loop);
	if (r < 0)
	{
		fprintf(stderr, "Error adding match rule: %s\n",
				strerror(-r));
		goto finish;
	}

	r = sd_bus_get_property_trivial(conn,
									service,
									argv[1],
									"org.openbmc.control.Power",
									"pgood",
									&error,
									'i',
									&state);
	if (r < 0)
	{
		fprintf(stderr, "Error getting property: %s\n",
				strerror(-r));
		goto finish;
	}

	if (dest == state)
		goto finish;

	r = sd_event_loop(loop);
	if (r < 0)
	{
		fprintf(stderr, "Error starting event loop: %s\n",
				strerror(-r));
		goto finish;
	}

finish:
	exit(r < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

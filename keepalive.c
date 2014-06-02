/*
 * keepalive - A tool for keeping device out of suspend for a period of time
 *
 * Copyright (C) 2014 Jolla Ltd.
 * Contact: Kalle Jokiniemi <kalle.jokiniemi@jolla.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <mce/dbus-names.h>

#include <gio/gio.h>



static int keepalive_verbose = 0;

#define keepalive_print(...) \
        do { fprintf(stderr, "keepalive: " \
				__VA_ARGS__); } while (0)

#define keepalive_debug(fmt, ...) \
        do { if (keepalive_verbose) fprintf(stderr, "%s:%d:%s(): " fmt, \
                       __FILE__, __LINE__, __func__, __VA_ARGS__); } while (0)

#define KEEPALIVE_DBUS_TIMEOUT	-1
#define KEEPALIVE_NAME	"keepalive-tool"

struct keepalive_data {
	GDBusConnection *bus;
};

static struct keepalive_data keepalive;

static struct option const options[] = {
	{"verbose", no_argument, NULL, 'v'},
	{"help", no_argument, NULL, 'h'},
	{"time", required_argument, NULL, 't'},
	{NULL, 0, NULL, 0}
};

void usage (void)
{
	printf("\
Usage: keepalive [OPTIONS] [SECONDS]\n\
  SECONDS                      How many seconds to keep cpu alive. If not\n\
                               given, or 0, run forever.\n\
  OPTIONS:\n\
  -v, --verbose                Print debug prints when running\n\
  -h, --help                   Print this help\n\n\
");
}

int keepalive_init(struct keepalive_data *data)
{
	GError *error = NULL;

	data->bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM,
					NULL,
					&error);
	if (data->bus == NULL) {
		keepalive_print("failed to get system bus: %s", error->message);
		g_error_free(error);
		return errno;
	}

	keepalive_debug("Got dbus %u\n", (int)data->bus);

	return 0;
}

void keepalive_free(struct keepalive_data *data)
{
	g_object_unref(data->bus);
	data->bus = NULL;
}

int keepalive_mce(struct keepalive_data *data, bool enable)
{
	GError *error = NULL;
	GDBusMessage *msg;
	GDBusMessage *reply;
	GVariant *val;
	volatile guint32 *serial = 0;
	gboolean mce_success = FALSE;
	int ret = 0;

	if (enable)
		msg = g_dbus_message_new_method_call(MCE_SERVICE,
					MCE_REQUEST_PATH,
					MCE_REQUEST_IF,
					MCE_CPU_KEEPALIVE_START_REQ);
	else
		msg = g_dbus_message_new_method_call(MCE_SERVICE,
					MCE_REQUEST_PATH,
					MCE_REQUEST_IF,
					MCE_CPU_KEEPALIVE_STOP_REQ);

	keepalive_debug("%s\n", g_dbus_message_print(msg, 4));

	reply = g_dbus_connection_send_message_with_reply_sync(
					data->bus,
					msg,
					G_DBUS_CALL_FLAGS_NONE,
					KEEPALIVE_DBUS_TIMEOUT,
					serial,
					NULL,
					&error);
	if (reply == NULL) {
		keepalive_print("Failed keepalive enable %d:%s\n", enable,
								error->message);
		ret = -ENOMSG;
		g_error_free(error);
		goto out;
	}

	keepalive_debug("%s\n", g_dbus_message_print(reply, 4));

	val = g_dbus_message_get_body(reply);
	g_variant_get(val, "(b)", &mce_success);
	keepalive_debug("mce result %d\n", mce_success);
	if (!mce_success) {
		keepalive_print("Could not start keepalive\n");
		ret = -ECOMM;
		goto out;
	}

out:
	g_object_unref(msg);
	g_object_unref(reply);
	return ret;
}

gint32 keepalive_mce_get_period(struct keepalive_data *data)
{
	GError *error = NULL;
	GDBusMessage *msg;
	GDBusMessage *reply;
	GVariant *val;
	volatile guint32 *serial = 0;
	gint32 ret = 0;

	msg = g_dbus_message_new_method_call(MCE_SERVICE,
					MCE_REQUEST_PATH,
					MCE_REQUEST_IF,
					MCE_CPU_KEEPALIVE_PERIOD_REQ);

	g_dbus_message_set_body(msg,
			g_variant_new("(s)", KEEPALIVE_NAME));

	keepalive_debug("%s\n", g_dbus_message_print(msg, 4));

	reply = g_dbus_connection_send_message_with_reply_sync(
					data->bus,
					msg,
					G_DBUS_CALL_FLAGS_NONE,
					KEEPALIVE_DBUS_TIMEOUT,
					serial,
					NULL,
					&error);
	if (reply == NULL) {
		keepalive_print("Could not get period, %s\n", error->message);
		ret = -ENOMSG;
		g_error_free(error);
		goto out;
	}

	keepalive_debug("%s\n", g_dbus_message_print(reply, 4));

	val = g_dbus_message_get_body(reply);
	g_variant_get(val, "(i)", &ret);
	keepalive_debug("Got period %d from mce\n", ret);

out:
	g_object_unref(msg);
	g_object_unref(reply);
	return ret;
}

int keepalive_start(struct keepalive_data *data, int seconds)
{
	int timeout_period;
	int time_left = seconds;
	int ret = 0;

	timeout_period = keepalive_mce_get_period(data);

	if (timeout_period < 0) {
		keepalive_print("Could not get timeout period\n");
		ret = timeout_period;
		goto out;
	}

	/* if no keepalive time given, stay up forever */
	if (seconds == 0) {
		while (1) {
			ret = keepalive_mce(data, true);
			if (ret)
				goto out_cleanup;
			sleep(timeout_period);
		}
	} else {
		while (time_left > 0) {
			ret = keepalive_mce(data, true);
			if (ret)
				goto out_cleanup;

			if (time_left > timeout_period) {
				sleep(timeout_period);
				time_left -= timeout_period;
			} else {
				sleep(time_left);
				time_left -= time_left;
			}
		}
	}
out_cleanup:
	keepalive_mce(data, false);
out:
	return ret;
}

int main (int argc, char *argv[])
{
	int c, option_index = 0;
	int seconds = 0;
	int ret = 0;
	char *tail;

	while (1) {
		c = getopt_long (argc, argv, "hv", options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage();
			goto done;
			break;
		case 'v':
			keepalive_verbose = 1;
			break;
		default:
			printf("Bad option \"%d\" given!\n", c);
			goto fail_help;
			break;
		}
	}

	if (optind < argc) {
		seconds = strtol(argv[optind++], &tail, 10);
	}

	keepalive_debug("t = %d\n", seconds);
	keepalive_init(&keepalive);
	ret = keepalive_start(&keepalive, seconds);
	keepalive_free(&keepalive);
	if (ret < 0)
		goto fail;

done:
	return EXIT_SUCCESS;
fail_help:
	usage();
fail:
	return EXIT_FAILURE;
}


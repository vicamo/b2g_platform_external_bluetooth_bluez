/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#ifdef ANDROID
#  define LOG_TAG "bluetoothd"
#  include <cutils/log.h>
#else
#  define USE_SYSLOG 1
#  include <syslog.h>
#endif

#include <glib.h>

#include "log.h"

void info(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);

#ifdef ANDROID
	LOG_PRI_VA(ANDROID_LOG_INFO, LOG_TAG, format, ap);
#elif defined(USE_SYSLOG)
	vsyslog(LOG_INFO, format, ap);
#endif

	va_end(ap);
}

void error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);

#ifdef ANDROID
	LOG_PRI_VA(ANDROID_LOG_ERROR, LOG_TAG, format, ap);
#elif defined(USE_SYSLOG)
	vsyslog(LOG_ERR, format, ap);
#endif

	va_end(ap);
}

void btd_debug(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);

#ifdef ANDROID
	LOG_PRI_VA(ANDROID_LOG_DEBUG, LOG_TAG, format, ap);
#elif defined(USE_SYSLOG)
	vsyslog(LOG_DEBUG, format, ap);
#endif

	va_end(ap);
}

extern struct btd_debug_desc __start___debug[];
extern struct btd_debug_desc __stop___debug[];

static gchar **enabled = NULL;

static gboolean is_enabled(struct btd_debug_desc *desc)
{
	int i;

	if (enabled == NULL)
		return 0;

	for (i = 0; enabled[i] != NULL; i++)
		if (desc->file != NULL && g_pattern_match_simple(enabled[i],
							desc->file) == TRUE)
			return 1;

	return 0;
}

void __btd_toggle_debug(void)
{
	struct btd_debug_desc *desc;

	for (desc = __start___debug; desc < __stop___debug; desc++)
		desc->flags |= BTD_DEBUG_FLAG_PRINT;
}

void __btd_log_init(const char *debug, int detach)
{
#ifdef USE_SYSLOG
	int option = LOG_NDELAY | LOG_PID;
#endif
	struct btd_debug_desc *desc;

	SLOGD("__btd_log_init: debug=%s", debug);
	if (debug != NULL)
		enabled = g_strsplit_set(debug, ":, ", 0);

	for (desc = __start___debug; desc < __stop___debug; desc++)
		if (is_enabled(desc))
			desc->flags |= BTD_DEBUG_FLAG_PRINT;

#ifdef USE_SYSLOG
	if (!detach)
		option |= LOG_PERROR;

	openlog("bluetoothd", option, LOG_DAEMON);

	syslog(LOG_INFO, "Bluetooth deamon %s", VERSION);
#endif
}

void __btd_log_cleanup(void)
{
#ifdef USE_SYSLOG
	closelog();
#endif

	g_strfreev(enabled);
}

/*
 * Copyright 2017-2019 Baidu Inc.
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

#include "openrasp_hook.h"
#include "openrasp_v8.h"
#include "openrasp_mongo_utils.h"

static int port_start_to_int(char *port_start)
{
	int tmp_port = 27017;
	if (port_start)
	{
		tmp_port = atoi(port_start);
	}
	return tmp_port;
}

int mongo_parse_connection_string(sql_connection_entry *mongo_connection_p, char *spec)
{
	char *pos;
	char *host_start, *host_end, *port_start, *db_start, *db_end, *last_slash;
	int i;
	std::string tmp_user;
	std::string tmp_pass;

	/* Initialisation */
	pos = spec;
	mongo_connection_p->set_connection_string(std::string(spec));

	if (strstr(spec, "mongodb://") == spec)
	{
		char *at, *colon;
		/* mongodb://user:pass@host:port,host:port
		 *           ^                             */
		pos += 10;
		/* mongodb://user:pass@host:port,host:port
		 *                    ^                    */
		at = strchr(pos, '@');
		/* mongodb://user:pass@host:port,host:port
		 *               ^                         */
		colon = strchr(pos, ':');
		/* check for username:password */
		if (at && colon && at - colon > 0)
		{
			tmp_user = std::string(pos, colon - pos);
			tmp_pass = std::string(colon + 1, at - (colon + 1));
			mongo_connection_p->set_username(tmp_user);
			mongo_connection_p->set_password(tmp_pass);
			/* move current
			 * mongodb://user:pass@host:port,host:port
			 *                     ^                   */
			pos = at + 1;
		}
	}
	host_start = pos;
	host_end = NULL;
	port_start = NULL;
	last_slash = NULL;

	/* parse the host part - two cases:
	 * 1: mongodb://user:pass@host:port,host:port/database?opt=1 -- TCP/IP
	 *                        ^
	 * 2: mongodb://user:pass@/tmp/mongo.sock/database?opt=1 -- Unix Domain sockets
	 *                        ^                                                     */
	if (*pos != '/')
	{
		/* TCP/IP:
		 * mongodb://user:pass@host:port,host:port/database?opt=1 -- TCP/IP
		 *                     ^                                            */
		do
		{
			if (*pos == ':')
			{
				host_end = pos;
				port_start = pos + 1;
			}
			if (*pos == ',')
			{
				if (!host_end)
				{
					host_end = pos;
				}
				mongo_connection_p->append_host_port(std::string(host_start, host_end), port_start_to_int(port_start));

				host_start = pos + 1;
				host_end = port_start = NULL;
			}
			if (*pos == '/')
			{
				if (!host_end)
				{
					host_end = pos;
				}
				break;
			}
			pos++;
		} while (*pos != '\0');
		mongo_connection_p->append_host_port(std::string(host_start, host_end ? host_end : pos), port_start_to_int(port_start));
	}
	else if (*pos == '/')
	{
		host_start = pos;
		port_start = "0";

		/* Unix Domain Socket
		 * mongodb://user:pass@/tmp/mongo.sock
		 * mongodb://user:pass@/tmp/mongo.sock/?opt=1
		 * mongodb://user:pass@/tmp/mongo.sock/database?opt=1
		 */
		last_slash = strrchr(pos, '/');

		/* The last component of the path *could* be a database name.  The rule
		 * is; if the last component has a dot, we use the full string since
		 * "host_start" as host */
		if (strchr(last_slash, '.'))
		{
			host_end = host_start + strlen(host_start);
		}
		else
		{
			host_end = last_slash;
		}
		pos = host_end;
		mongo_connection_p->set_socket(std::string(host_start, host_end - host_start));
	}
	return 0;
}
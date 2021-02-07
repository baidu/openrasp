/*
 * Copyright 2017-2021 Baidu Inc.
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

#include "url.h"
#include <curl/curl.h>
#include "openrasp_utils.h"

namespace openrasp
{

Url::Url()
{
  parse_error = true;
}

Url::Url(const std::string &url_string)
{
  parse(url_string);
}

void Url::parse(const std::string &url_string)
{
#if (LIBCURL_VERSION_MAJOR >= 7) && (LIBCURL_VERSION_MINOR >= 62)
  CURLUcode rc;
  CURLU *url = curl_url();
  rc = curl_url_set(url, CURLUPART_URL, url_string.c_str(), CURLU_DEFAULT_SCHEME);
  if (!rc)
  {
    char *libcurl_scheme;
    rc = curl_url_get(url, CURLUPART_SCHEME, &libcurl_scheme, CURLU_DEFAULT_SCHEME);
    if (!rc)
    {
      scheme = std::string(libcurl_scheme);
      curl_free(libcurl_scheme);
    }

    char *libcurl_host;
    rc = curl_url_get(url, CURLUPART_HOST, &libcurl_host, 0);
    if (!rc)
    {
      host = std::string(libcurl_host);
      curl_free(libcurl_host);
    }

    char *libcurl_port;
    rc = curl_url_get(url, CURLUPART_PORT, &libcurl_port, CURLU_DEFAULT_PORT);
    if (!rc)
    {
      port = std::string(libcurl_port);
      curl_free(libcurl_port);
    }

    char *libcurl_path;
    rc = curl_url_get(url, CURLUPART_PATH, &libcurl_path, 0);
    if (!rc)
    {
      path = std::string(libcurl_path);
      curl_free(libcurl_path);
    }

    char *libcurl_query;
    rc = curl_url_get(url, CURLUPART_QUERY, &libcurl_query, 0);
    if (!rc)
    {
      path = std::string(libcurl_query);
      curl_free(libcurl_query);
    }

    curl_url_cleanup(url);
    parse_error = false;
  }
  else
  {
    parse_error = true;
  }
#else
  if (!openrasp_parse_url(url_string, *this) || scheme.empty())
  {
    if (!openrasp_parse_url("http://" + url_string, *this))
    {
      parse_error = true;
      return;
    }
  }
  parse_error = false;
#endif
}

bool Url::has_error() const
{
  return parse_error;
}

std::string Url::get_scheme() const
{
  return scheme;
}
std::string Url::get_host() const
{
  return host;
}

std::string Url::get_port() const
{
  return port;
}

std::string Url::get_path() const
{
  if (path.empty())
  {
    return "/";
  }
  return path;
}
std::string Url::get_query() const
{
  return query;
}
void Url::set_scheme(const std::string &scheme)
{
  this->scheme = scheme;
}
void Url::set_host(const std::string &host)
{
  this->host = host;
}
void Url::set_port(const std::string &port)
{
  this->port = port;
}
void Url::set_path(const std::string &path)
{
  this->path = path;
}
void Url::set_query(const std::string &query)
{
  this->query = query;
}

bool operator==(const openrasp::Url &lhs, const openrasp::Url &rhs)
{
  return lhs.get_host() == rhs.get_host() &&
         lhs.get_port() == rhs.get_port() &&
         lhs.get_path() == rhs.get_path() &&
         lhs.get_query() == rhs.get_query();
}

} // namespace openrasp

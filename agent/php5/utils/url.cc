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

#include "url.h"
#include <curl/curl.h>
#include "openrasp_utils.h"

namespace openrasp
{

Url::Url(std::string url_string)
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

    curl_url_cleanup(url);
  }
  else
  {
    parse_error = true;
  }
#else
  if (!openrasp_parse_url(url_string, scheme, host, port) || scheme.empty())
  {
    if (!openrasp_parse_url("http://" + url_string, scheme, host, port))
    {
      parse_error = true;
    }
  }
#endif
}

bool Url::has_error() const
{
  return parse_error;
}

std::string Url::get_host() const
{
  return host;
}

std::string Url::get_port() const
{
  return port;
}

} // namespace openrasp

/*
 * Copyright 2017-2018 Baidu Inc.
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

#include "curl_helper.h"
#include "openrasp_ini.h"

namespace openrasp
{

static size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append((char *)ptr, size * nmemb);
    return size * nmemb;
}

void perform_curl(CURL *curl, const std::string url_string, const char *postdata, ResponseInfo &res_info)
{
    if (curl)
    {
        struct curl_slist *chunk = nullptr;
        std::string auth_header = "X-OpenRASP-AppID: " + std::string(openrasp_ini.app_id);
        chunk = curl_slist_append(chunk, auth_header.c_str());
        res_info.res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_URL, url_string.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        if (postdata)
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(res_info.response_string));
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(res_info.header_string));
        res_info.res = curl_easy_perform(curl);
        if (CURLE_OK == res_info.res)
        {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(res_info.response_code));
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &(res_info.elapsed));
        }
    }
}

char *fetch_outmost_string_from_ht(HashTable *ht, const char *arKey)
{
    zval *origin_zv;
    if ((origin_zv = zend_hash_str_find(ht, arKey, strlen(arKey))) != nullptr &&
        Z_TYPE_P(origin_zv) == IS_STRING)
    {
        return Z_STRVAL_P(origin_zv);
    }
    return nullptr;
}

HashTable *fetch_outmost_hashtable_from_ht(HashTable *ht, const char *arKey)
{
    zval *origin_zv;
    if ((origin_zv = zend_hash_str_find(ht, arKey, strlen(arKey))) != nullptr &&
        Z_TYPE_P(origin_zv) == IS_ARRAY)
    {
        return Z_ARRVAL_P(origin_zv);
    }
    return nullptr;
}

bool fetch_outmost_long_from_ht(HashTable *ht, const char *arKey, long *result)
{
    zval *origin_zv;
    if ((origin_zv = zend_hash_str_find(ht, arKey, strlen(arKey))) != nullptr &&
        Z_TYPE_P(origin_zv) == IS_LONG)
    {
        *result = Z_LVAL_P(origin_zv);
        return true;
    }
    return false;
}
} // namespace openrasp
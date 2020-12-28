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

#include "openrasp_sql.h"
#include "openrasp_hook.h"

void pg_conninfo_parse(char *connstring, std::function<void(const char *pname, const char *pval)> info_store_func)
{
    char *buf = NULL;
    char *cp = NULL;
    char *cp2 = NULL;
    char *pname = NULL;
    char *pval = NULL;
    if (connstring)
    {
        buf = estrdup(connstring);
        cp = buf;
        while (*cp)
        {
            if (isspace((unsigned char)*cp))
            {
                cp++;
                continue;
            }

            pname = cp;
            while (*cp)
            {
                if (*cp == '=')
                {
                    break;
                }
                if (isspace((unsigned char)*cp))
                {
                    *cp++ = '\0';
                    while (*cp)
                    {
                        if (!isspace((unsigned char)*cp))
                        {
                            break;
                        }
                        cp++;
                    }
                    break;
                }
                cp++;
            }

            if (*cp != '=')
            {
                efree(buf);
                return;
            }
            *cp++ = '\0';

            while (*cp)
            {
                if (!isspace((unsigned char)*cp))
                {
                    break;
                }
                cp++;
            }

            pval = cp;
            if (*cp != '\'')
            {
                cp2 = pval;
                while (*cp)
                {
                    if (isspace((unsigned char)*cp))
                    {
                        *cp++ = '\0';
                        break;
                    }
                    if (*cp == '\\')
                    {
                        cp++;
                        if (*cp != '\0')
                        {
                            *cp2++ = *cp++;
                        }
                    }
                    else
                    {
                        *cp2++ = *cp++;
                    }
                }
                *cp2 = '\0';
            }
            else
            {
                cp2 = pval;
                cp++;
                for (;;)
                {
                    if (*cp == '\0')
                    {
                        efree(buf);
                        return;
                    }
                    if (*cp == '\\')
                    {
                        cp++;
                        if (*cp != '\0')
                        {
                            *cp2++ = *cp++;
                        }
                        continue;
                    }
                    if (*cp == '\'')
                    {
                        *cp2 = '\0';
                        cp++;
                        break;
                    }
                    *cp2++ = *cp++;
                }
            }

            if (info_store_func)
            {
                info_store_func(pname, pval);
            }
        }
        efree(buf);
    }
}
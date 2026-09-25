#ifndef LIBCRAWLER_STDAFX_H
#define LIBCRAWLER_STDAFX_H

#include <stddef.h>
#include <string.h>

#include <curl/curl.h>

#include <lexbor/css/parser.h>
#include <lexbor/css/selectors/selectors.h>
#include <lexbor/html/parser.h>
#include <lexbor/selectors/selectors.h>

#include <lexbor/dom/interfaces/element.h>
#include <lexbor/dom/interfaces/node.h>

#ifdef __cplusplus

#include <string>
#include <vector>

extern "C" {
#endif

#include "libcrawler.h"

union mem_16
{
    char c[16];
    uint64_t u64[2];
};

extern CURLcode curl_easy_impersonate(CURL *, const char *, int);

#ifdef __cplusplus
}
#endif

#endif /* LIBCRAWLER_STDAFX_H */

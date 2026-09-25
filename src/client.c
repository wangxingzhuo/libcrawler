#include "stdafx.h"

c_string_t * new_cstring(c_string_t *result, const char *str, size_t len)
{
    if (!result || result->data) return NULL;
    if (NULL == str || 0 == len) return result;

    result->length = 0;
    result->data = (char *)malloc(len + 1);
    if (result->data)
    {
        result->data[len] = 0;
        memcpy(result->data, str, len);
        result->length = len;
    }
    return result;
}

void cstring_free(c_string_t *value)
{
    if (!value) return;
    if (value->data) free(value->data);
    value->data = NULL;
    value->length = 0;
}

static void* mem_n_set(union mem_16 *dst, const void *src, size_t len)
{
    dst->u64[0] = 0;
    dst->u64[1] = 0;
    return memcpy(dst, src, len < 15 ? len : 15);
}

static int strnchr(const char *s, const size_t len, int c)
{
    size_t i;
    for (i = 0; i < len && s[i] != c; i++);
    return len == i ? -1 : i;
}

static size_t read_header(char *data, size_t size, size_t count, struct resp_header *header)
{
    const size_t len = size * count;

    if (len >= 12 && 0 == strncmp(data, "HTTP/", 5))
    {
        union mem_16 cache;
        mem_n_set(&cache, data, len);
        sscanf(cache.c, "HTTP/%*s %hd", &header->status);
        return len;
    }

    if (len < 14 || strncmp(data, "Content-Type:", 13)) return len;

    int off = data[13] < 0x21 ? 14 : 13;
    const char *val = data + off;
    ssize_t val_len = strnchr(val, ';', len - off);
    if (0 < val_len)
    {
        cstring_free(&header->content_type);
        new_cstring(&header->content_type, val, (size_t)val_len);
    }
    
    return len;
}

static size_t write_body(char *data, size_t size, size_t count, response_t *context)
{
    const size_t len = size * count;
    c_string_t buf = {len, data};
    context->on_data(buf, context->context);
    return len;
}

int fetch_get(response_t *resp, const char *browser, const char *url)
{
    if (
        NULL == resp || NULL == url || NULL == browser || NULL == resp->on_data ||
        NULL != resp->header.content_type.data || 0 != resp->header.content_type.length
    )
        return -1;

    CURL *curl = curl_easy_init();
    if (NULL == curl) return -2;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_body);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, read_header);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp->header);

    CURLcode code = curl_easy_impersonate(curl, browser, 1);
    if (CURLE_OK == code)
        code = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    return code;
}

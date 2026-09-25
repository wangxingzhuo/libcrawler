#ifndef LIBCRAWLER_H
#define LIBCRAWLER_H

#include <stddef.h>
#include <stdint.h>

#include <lexbor/dom/interfaces/element.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct c_string
{
    size_t length;
    char *data;
} c_string_t;

typedef void (*on_data_fn)(const c_string_t data, void *context);

struct resp_header
{
    int16_t status;
    c_string_t content_type;
};

typedef struct response
{
	struct resp_header header;
	on_data_fn on_data;
	void *context;
} response_t;

int fetch_get(response_t *response, const char *browser, const char *url);
lxb_html_document_t * to_document(c_string_t html);
int select_nodes(lxb_dom_element_t ***dst, lxb_dom_element_t *root, const char *selector);
c_string_t text_content(lxb_dom_element_t *element);
c_string_t attribute_value(lxb_dom_element_t *element, const char *name);

c_string_t * new_cstring(c_string_t *result, const char *str, size_t len);
void cstring_free(c_string_t *value);
void destroy_document(lxb_html_document_t *doc);

#ifdef __cplusplus
}
#endif

#endif /* LIBCRAWLER_H */

#ifndef LIBCRAWLER_H
#define LIBCRAWLER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct crawler_element_t crawler_element_t;

/* Returns NULL on failure. Check crawler_last_error() on the calling thread. */
crawler_element_t *fetch(const char *url);

/* Returns a JSON/JSONP body owned by the caller, or NULL on failure. */
char *fetch_json(const char *url);

/* Returns NULL when there is no match or the selector is invalid. */
crawler_element_t *query_selector(const crawler_element_t *element, const char *selector);
crawler_element_t *query_selector_all(const crawler_element_t *element, const char *selector, size_t *count);

/* Returned strings must be released with crawler_free_string(). */
char *inner_text(const crawler_element_t *element);
char *get_attribute(const crawler_element_t *element, const char *name);

/* Returns the latest error for the calling thread, or NULL when there is none. */
const char *crawler_last_error(void);

void crawler_free_string(char *value);
void crawler_free(void *value);

#ifdef __cplusplus
}
#endif

#endif /* LIBCRAWLER_H */

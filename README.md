# libcrawler

`libcrawler` is a C23/C++23 shared library that fetches pages through the
`libcurl-impersonate` API, parses HTML with Lexbor, and exposes a small C ABI.

## Setup

Lexbor is managed by vcpkg using [vcpkg.json](vcpkg.json). `libcurl-impersonate`
is not an official vcpkg port in the configured registry, so it must be built
or installed separately:

```sh
vcpkg install --triplet arm64-osx
```

Install or build the static `libcurl-impersonate` development files from
[`curl-impersonate`](https://github.com/lwthiker/curl-impersonate). Set the
directory containing `libcurl-impersonate.a` (or a compatible `libcurl.a`)
before configuring:

```sh
LIBCRAWLER_LIBCURL_IMPERSONATE_DIR=/path/to/libcurl-impersonate/lib \
  VCPKG_TARGET_TRIPLET=arm64-osx \
  ./configure.sh
make -j2
```

The Makefile uses Lexbor from the vcpkg installation and its official CSS
selector API; no selector implementation is maintained in this project. If
the impersonate archive has additional static dependencies, provide them as a
comma-separated linker list:

```sh
LIBCRAWLER_LIBCURL_IMPERSONATE_LIBS="ssl,crypto,z" ./configure.sh
```

The impersonated browser target defaults to `chrome116`; override it with
`LIBCRAWLER_BROWSER` when another supported target is needed. `configure.sh`
requires both a vcpkg Lexbor installation and the external impersonate
archive.

## C API

The matching C declarations are provided in
[`include/libcrawler.h`](include/libcrawler.h). Add `include/` to the C
compiler include path when integrating the library:

```sh
cc -I/path/to/libcrawler/include ... -L/path/to/libcrawler/target/release -llibcrawler
```

```c
#include <libcrawler.h>

crawler_element_t *document = fetch("https://example.com");
char *json = fetch_json("https://example.com/data.json");
crawler_element_t *title = query_selector(document, "title");
crawler_element_t *link = query_selector_from_element(title, "a");
size_t link_count = 0;
crawler_element_t *links = query_selector_all(document, "a", &link_count);
char *text = inner_text(title);
char *href = get_attribute(title, "data-href");

crawler_free_string(text);
crawler_free_string(href);
crawler_free_string(json);
crawler_free(link);
crawler_free(links);
crawler_free(title);
crawler_free(document);
```

`fetch` and invalid selectors return `NULL` on failure. Call
`crawler_last_error()` on the same thread for the latest error message.
`fetch` checks the response `Content-Type` and parses HTML only for
`text/html` or `application/xhtml+xml`. `fetch_json` accepts
`application/json`, media types ending in `+json`, and common JavaScript media
types used by JSONP. For JSONP it removes the `callback(...)` wrapper and
returns the UTF-8 JSON body as a string owned by the caller; release it with
`crawler_free_string`.
`query_selector_all` returns a contiguous `crawler_element_t` array and writes
its length to `count`. Release the array with
`crawler_free(array)`; the allocation stores its length internally. The same
`crawler_free` function releases root elements, single elements, and element
arrays. C strings still use `crawler_free_string`.
`query_selector_from_element` and `query_selector_all_from_element` perform
the same queries relative to an existing element root.
`get_attribute` also returns `NULL` when the attribute does not exist. Strings
returned by `inner_text` and `get_attribute` must be released with
`crawler_free_string`; root elements, element arrays, and elements use
`crawler_free`.

All crawler handles contain immutable response text and can be queried
concurrently from multiple threads. Do not free a handle while another thread
is using it. `crawler_last_error()` is thread-local: each thread sees only its
own most recent error, and a subsequent API call on that same thread may
replace it.

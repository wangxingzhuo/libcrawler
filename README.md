# libcrawler

`libcrawler` is an experimental C/C++ library for fetching HTTP responses with
`libcurl-impersonate` and parsing HTML with Lexbor. The project currently
exposes low-level request, document, CSS selector, and text/attribute helpers.

The public header has a C-compatible data model. The HTTP implementation is
compiled as C23, while the HTML and selector implementation is compiled as
C++23. The final shared-library link is performed by the C++ compiler because
the library contains C++ objects.

## Current status

The repository is mid-refactor. The current Makefile builds `src/client.c` as
C23 and `src/element.cpp` as C++23, then links both objects into a shared
library with the C++ compiler. A configured `build/config.mk` and a compatible
external curl-impersonate installation are required before running `make`.

## Dependencies

- C++23 compiler
- `libcurl-impersonate`, built separately
- Lexbor, installed through vcpkg
- GNU Make

Lexbor is declared in [vcpkg.json](vcpkg.json). The curl impersonation library
is not installed by this project.

## Intended configuration

From the repository root, configure the platform-specific build files with:

```sh
export LIBCRAWLER_LIBCURL_IMPERSONATE_DIR=/path/to/libcurl-impersonate/lib
export LIBCRAWLER_LIBCURL_IMPERSONATE_NAME=libcurl-impersonate.a
export VCPKG_TARGET_TRIPLET=arm64-osx  # optional
./configure.sh
```

`LIBCRAWLER_LIBCURL_IMPERSONATE_NAME` is currently required in practice. The
automatic library-name detection in `configure.sh` is commented out.

The script expects vcpkg at `./vcpkg_installed/vcpkg` and writes
`build/config.mk`. It selects a triplet from the host platform when
`VCPKG_TARGET_TRIPLET` is not set.

The intended build command is:

```sh
make
```

The current Makefile emits a shared library under `build/`, using `.dylib` on
macOS and `.so` on Linux. There is currently no install target.

## Public API

The declarations are in [include/libcrawler.h](include/libcrawler.h).

### HTTP response types

```c
typedef struct c_string {
    size_t length;
    char *data;
} c_string_t;

typedef struct resp_header {
    int16_t status;
    c_string_t content_type;
} resp_header;

typedef struct response {
    struct resp_header header;
    on_data_fn on_data;
    void *context;
} response_t;
```

The available functions are:

```c
const int fetch_get(response_t *response, const char *browser, const char *url);
lxb_html_document_t *to_document(c_string_t html);
int select_nodes(lxb_dom_element_t ***dst,
                 lxb_dom_element_t *root,
                 const char *selector);
c_string_t text_content(lxb_dom_element_t *element);
c_string_t attribute_value(lxb_dom_element_t *element, const char *name);
c_string_t *new_cstring(c_string_t *result, const char *str, size_t len);
```

`browser` is passed to `curl_easy_impersonate`; use a browser target supported
by the installed curl-impersonate build.

## Intended usage

The document owns its DOM tree. Element pointers returned from the document or
from `select_nodes` are borrowed pointers and remain valid only while the
document remains alive.

```cpp
c_string_t html = {html_length, html_bytes};
lxb_html_document_t *document = to_document(html);
if (document == nullptr) {
    return 1;
}

lxb_dom_element_t *root = lxb_dom_interface_element(document->body);
lxb_dom_element_t **matches = nullptr;
int count = select_nodes(&matches, root, "article a");

for (int i = 0; i < count; ++i) {
    c_string_t text = text_content(matches[i]);
    /* consume text.data */
}

free(matches);                         /* pointer array only */
lxb_html_document_destroy(document);   /* frees the DOM tree */
```

Do not call `lxb_dom_element_destroy` or
`lxb_dom_element_interface_destroy` on `root` or on individual selector
results. They are owned by the document. Destroy the document only after all
borrowed element pointers are no longer used.

`fetch_get` invokes `response_t::on_data` for each received body chunk. The
callback receives a temporary view; copy the bytes if they must survive the
callback. The response object and its `content_type` storage must be
initialized and managed by the caller.

## Ownership rules

- `to_document` returns a document owned by the caller.
- Destroy the document with `lxb_html_document_destroy` after all DOM access.
- `select_nodes` allocates only the pointer array; release it with `free`.
- The elements inside that array are borrowed and must not be freed individually.
- `text_content`, `attribute_value`, and `new_cstring` allocate string data
  with `malloc`; release the returned `data` with `free`.
- The callback's body buffer is temporary and must be copied when retained.

## Known limitations

The current tree still needs API and build cleanup before it can be treated as
a stable library:

- `configure.sh` writes a Lexbor include path that should be checked against the
  compiler's expected include root.
- `fetch_get` does not currently clean up its curl easy handle on every path.
- The public header declares `cstring_free` and `element_free`, but the current
  implementation does not provide usable ownership-safe implementations for
  those functions.
- Error handling and allocation-failure reporting are incomplete.
- The API does not yet expose a public `destroy_document` wrapper; callers need
  the Lexbor destroy function directly.

Treat the API as unstable until these issues are resolved.

#pragma once

#include "stdafx.hpp"

namespace libcrawler::detail {

class Document;

struct CurlDeleter {
    void operator()(CURL *handle) const noexcept;
};

struct DocumentDeleter {
    void operator()(lxb_html_document_t *document) const noexcept;
};

using CurlHandle = std::unique_ptr<CURL, CurlDeleter>;
using LexborDocument = std::unique_ptr<lxb_html_document_t, DocumentDeleter>;

class Document {
public:
    explicit Document(std::string_view html);
    lxb_dom_element_t *root() const noexcept;

private:
    LexborDocument value_;
};

struct Response {
    std::string body;
    std::string content_type;
};

struct AllocationHeader {
    enum class Kind : unsigned char { Element, ElementArray };
    Kind kind;
    size_t count;
};

extern thread_local std::string last_error;

void set_error(std::string message);
Response fetch_response(const char *url);
std::string media_type(std::string_view content_type);
bool is_json(std::string_view type);
bool is_jsonp(std::string_view type);
std::string unwrap_jsonp(std::string_view body);
std::vector<lxb_dom_element_t *> select_nodes(lxb_dom_element_t *root,
                                               std::string_view selector);
std::string text_content(lxb_dom_node_t *node);

crawler_element_t *make_element(const std::shared_ptr<Document> &owner,
                                lxb_dom_element_t *node);
crawler_element_t *make_element_array(const std::shared_ptr<Document> &owner,
                                       const std::vector<lxb_dom_element_t *> &nodes);
void free_handle(void *value);

} // namespace libcrawler::detail

struct crawler_element_t {
    std::shared_ptr<libcrawler::detail::Document> owner;
    lxb_dom_element_t *node;
};

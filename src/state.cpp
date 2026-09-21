#include "stdafx.hpp"
#include "internal.hpp"

namespace libcrawler::detail {

thread_local std::string last_error;

void set_error(std::string message) { last_error = std::move(message); }

void DocumentDeleter::operator()(lxb_html_document_t *document) const noexcept {
    if (document != nullptr) lxb_html_document_destroy(document);
}

} // namespace libcrawler::detail

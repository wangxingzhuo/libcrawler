#include "stdafx.hpp"
#include "internal.hpp"

namespace {
char *copy_string(std::string_view value) {
    auto *result = static_cast<char *>(std::malloc(value.size() + 1));
    if (result == nullptr) throw std::bad_alloc();
    std::memcpy(result, value.data(), value.size());
    result[value.size()] = '\0';
    return result;
}

template <typename Function>
void *guard(Function &&function) {
    try {
        return function();
    } catch (const std::exception &error) {
        libcrawler::detail::set_error(error.what());
        return nullptr;
    }
}
} // namespace

extern "C" crawler_element_t *fetch(const char *url) {
    libcrawler::detail::last_error.clear();
    return static_cast<crawler_element_t *>(guard([&]() {
        const auto response = libcrawler::detail::fetch_response(url);
        const auto type = libcrawler::detail::media_type(response.content_type);
        if (!type.empty() && (libcrawler::detail::is_json(type) || libcrawler::detail::is_jsonp(type))) {
            throw std::runtime_error("response is JSON or JSONP; use fetch_json");
        }
        auto document = std::make_shared<libcrawler::detail::Document>(response.body);
        return static_cast<void *>(libcrawler::detail::make_element(document, document->root()));
    }));
}

extern "C" char *fetch_json(const char *url) {
    libcrawler::detail::last_error.clear();
    return static_cast<char *>(guard([&]() {
        const auto response = libcrawler::detail::fetch_response(url);
        const auto type = libcrawler::detail::media_type(response.content_type);
        const auto body = libcrawler::detail::is_json(type)
            ? response.body
            : (libcrawler::detail::is_jsonp(type) ? libcrawler::detail::unwrap_jsonp(response.body) : "");
        if (body.empty()) throw std::runtime_error("response is not JSON or JSONP");
        return static_cast<void *>(copy_string(body));
    }));
}

extern "C" crawler_element_t *query_selector(const crawler_element_t *root, const char *selector) {
    libcrawler::detail::last_error.clear();
    if (root == nullptr || selector == nullptr) {
        libcrawler::detail::set_error("invalid query arguments");
        return nullptr;
    }
    return static_cast<crawler_element_t *>(guard([&]() {
        const auto nodes = libcrawler::detail::select_nodes(root->node, selector);
        return nodes.empty() ? static_cast<void *>(nullptr) : static_cast<void *>(
            libcrawler::detail::make_element(root->owner, nodes.front()));
    }));
}

extern "C" crawler_element_t *query_selector_all(const crawler_element_t *root,
                                                   const char *selector, size_t *count) {
    libcrawler::detail::last_error.clear();
    if (root == nullptr || selector == nullptr || count == nullptr) {
        libcrawler::detail::set_error("invalid query arguments");
        return nullptr;
    }
    return static_cast<crawler_element_t *>(guard([&]() {
        const auto nodes = libcrawler::detail::select_nodes(root->node, selector);
        *count = nodes.size();
        return nodes.empty() ? static_cast<void *>(nullptr) : static_cast<void *>(
            libcrawler::detail::make_element_array(root->owner, nodes));
    }));
}

extern "C" crawler_element_t *query_selector_from_element(const crawler_element_t *root,
                                                             const char *selector) {
    return query_selector(root, selector);
}

extern "C" crawler_element_t *query_selector_all_from_element(const crawler_element_t *root,
                                                                const char *selector,
                                                                size_t *count) {
    return query_selector_all(root, selector, count);
}

extern "C" char *inner_text(const crawler_element_t *element) {
    if (element == nullptr) {
        libcrawler::detail::set_error("element must not be null");
        return nullptr;
    }
    return static_cast<char *>(guard([&]() {
        return static_cast<void *>(copy_string(
            libcrawler::detail::text_content(lxb_dom_interface_node(element->node))));
    }));
}

extern "C" char *get_attribute(const crawler_element_t *element, const char *name) {
    if (element == nullptr || name == nullptr) {
        libcrawler::detail::set_error("invalid attribute arguments");
        return nullptr;
    }
    return static_cast<char *>(guard([&]() -> void * {
        auto *attribute = lxb_dom_element_attr_by_name(
            element->node, reinterpret_cast<const lxb_char_t *>(name), std::strlen(name));
        if (attribute == nullptr) return nullptr;
        size_t length = 0;
        const auto *value = lxb_dom_attr_value(attribute, &length);
        return value == nullptr ? nullptr : copy_string(std::string_view(
            reinterpret_cast<const char *>(value), length));
    }));
}

extern "C" const char *crawler_last_error() {
    return libcrawler::detail::last_error.c_str();
}

extern "C" void crawler_free_string(char *value) { std::free(value); }
extern "C" void crawler_free(void *value) { libcrawler::detail::free_handle(value); }

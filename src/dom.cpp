#include "stdafx.hpp"
#include "internal.hpp"

namespace libcrawler::detail {

Document::Document(std::string_view html) : value_(lxb_html_document_create()) {
    if (!value_) throw std::runtime_error("unable to create Lexbor document");
    const auto *data = reinterpret_cast<const lxb_char_t *>(html.data());
    if (lxb_html_document_parse(value_.get(), data, html.size()) != LXB_STATUS_OK) {
        throw std::runtime_error("unable to parse HTML");
    }
}

lxb_dom_element_t *Document::root() const noexcept {
    return lxb_dom_interface_element(value_->body);
}

namespace {
struct ParserDeleter {
    void operator()(lxb_css_parser_t *value) const noexcept {
        if (value != nullptr) lxb_css_parser_destroy(value, true);
    }
};

struct SelectorsDeleter {
    void operator()(lxb_selectors_t *value) const noexcept {
        if (value != nullptr) lxb_selectors_destroy(value, true);
    }
};

struct SelectorContext {
    std::vector<lxb_dom_element_t *> nodes;
};

lxb_status_t collect_node(lxb_dom_node_t *node, lxb_css_selector_specificity_t,
                          void *context) {
    static_cast<SelectorContext *>(context)->nodes.push_back(lxb_dom_interface_element(node));
    return LXB_STATUS_OK;
}
} // namespace

std::vector<lxb_dom_element_t *> select_nodes(lxb_dom_element_t *root,
                                               std::string_view selector) {
    std::unique_ptr<lxb_css_parser_t, ParserDeleter> parser(lxb_css_parser_create());
    std::unique_ptr<lxb_selectors_t, SelectorsDeleter> selectors(lxb_selectors_create());
    if (!parser || !selectors || lxb_css_parser_init(parser.get(), nullptr) != LXB_STATUS_OK ||
        lxb_selectors_init(selectors.get()) != LXB_STATUS_OK) {
        throw std::runtime_error("unable to initialize Lexbor selectors");
    }

    const auto *data = reinterpret_cast<const lxb_char_t *>(selector.data());
    auto *list = lxb_css_selectors_parse(parser.get(), data, selector.size());
    if (list == nullptr) throw std::invalid_argument("invalid CSS selector");

    SelectorContext context;
    lxb_selectors_find(selectors.get(), lxb_dom_interface_node(root), list, collect_node, &context);
    lxb_css_selector_list_destroy_memory(list);
    return context.nodes;
}

std::string text_content(lxb_dom_node_t *node) {
    size_t length = 0;
    const auto *value = lxb_dom_node_text_content(node, &length);
    return value == nullptr ? std::string{} : std::string(reinterpret_cast<const char *>(value), length);
}

} // namespace libcrawler::detail

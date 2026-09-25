#include "stdafx.h"

static lxb_css_parser_t * new_css_parser()
{
    lxb_css_parser_t *parser = lxb_css_parser_create();
    if (parser && LXB_STATUS_OK != lxb_css_parser_init(parser, nullptr))
    {
        lxb_css_parser_destroy(parser, true);
        parser = nullptr;
    }

    return parser;
}

static lxb_selectors_t * new_selectors()
{
    lxb_selectors_t *selectors = lxb_selectors_create();
    if (selectors && LXB_STATUS_OK != lxb_selectors_init(selectors))
    {
        lxb_selectors_destroy(selectors, true);
        selectors = nullptr;
    }

    return selectors;
}

static lxb_status_t collect_node(lxb_dom_node_t *node, lxb_css_selector_specificity_t, std::vector<lxb_dom_element_t *> *context)
{
    context->push_back(lxb_dom_interface_element(node));
    return LXB_STATUS_OK;
}


lxb_html_document_t * to_document(c_string_t html)
{
    if (nullptr == html.data || 0 == html.length) return nullptr;

    lxb_html_document_t *doc = lxb_html_document_create();
    if (doc && LXB_STATUS_OK != lxb_html_document_parse(doc, (const lxb_char_t *)html.data, html.length))
    {
        lxb_html_document_destroy(doc);
        doc = nullptr;
        // *root = *lxb_dom_interface_element(doc->body);
    }

    return doc;
}

void destroy_document(lxb_html_document_t *doc)
{
    lxb_html_document_destroy(doc);
}

int select_nodes(lxb_dom_element_t ***dst, lxb_dom_element_t *root, const char *selector)
{
    if (nullptr == dst || nullptr == root || nullptr == selector || nullptr != *dst) return -1;

    lxb_css_parser_t *parser = new_css_parser();
    lxb_selectors_t *selectors = new_selectors();
    lxb_css_selector_list_t *list = nullptr;
    std::vector<lxb_dom_element_t *> nodes;
    bool inited = (parser && selectors);

    if (inited)
    {
        list = lxb_css_selectors_parse(parser, (const lxb_char_t *)selector, std::strlen(selector));
        if (list)
        {
            lxb_selectors_find(selectors, lxb_dom_interface_node(root), list, (lxb_selectors_cb_f)collect_node, &nodes);
        }
    }

    lxb_css_selector_list_destroy_memory(list);
    lxb_selectors_destroy(selectors, true);
    lxb_css_parser_destroy(parser, true);

    if (!inited) return -1;
    if (nodes.empty()) return 0;

    lxb_dom_element_t **arr;
    arr = (lxb_dom_element_t **)malloc(sizeof(lxb_dom_element_t *) * nodes.size());
    if (!arr) return -1;

    for (size_t i = 0; i < nodes.size(); i++)
    {
        arr[i] = nodes[i];
    }
    *dst = arr;
    return nodes.size();
}

c_string_t text_content(lxb_dom_element_t *element)
{
    if (nullptr == element) return c_string_t{0, nullptr};

    size_t length = 0;
    const lxb_char_t *value = lxb_dom_node_text_content(&element->node, &length);

    c_string_t result = {0, nullptr};
    return *new_cstring(&result, (const char *)value, length);
}

c_string_t attribute_value(lxb_dom_element_t *element, const char *name)
{
    if (nullptr == element || nullptr == name) return c_string_t{0, nullptr};

    lxb_dom_attr_t *attribute = lxb_dom_element_attr_by_name(element, (const lxb_char_t *)name, std::strlen(name));
    if (nullptr == attribute) return c_string_t{0, nullptr};

    size_t len;
    c_string_t result = {0, nullptr};

    const lxb_char_t *bytes = lxb_dom_attr_value(attribute, &len);
    new_cstring(&result, (const char *)bytes, len);

    return result;
}

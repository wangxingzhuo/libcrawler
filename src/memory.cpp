#include "stdafx.hpp"
#include "internal.hpp"

namespace libcrawler::detail {

namespace {
size_t allocation_size(AllocationHeader::Kind kind, size_t count) {
    const auto elements = kind == AllocationHeader::Kind::ElementArray ? count : 1;
    return sizeof(AllocationHeader) + sizeof(crawler_element_t) * elements;
}
}

crawler_element_t *make_element(const std::shared_ptr<Document> &owner,
                                lxb_dom_element_t *node) {
    const auto bytes = allocation_size(AllocationHeader::Kind::Element, 1);
    auto *base = static_cast<unsigned char *>(std::malloc(bytes));
    if (base == nullptr) throw std::bad_alloc();

    auto *header = reinterpret_cast<AllocationHeader *>(base);
    *header = {AllocationHeader::Kind::Element, 1};
    auto *element = reinterpret_cast<crawler_element_t *>(base + sizeof(*header));
    new (element) crawler_element_t{owner, node};
    return element;
}

crawler_element_t *make_element_array(const std::shared_ptr<Document> &owner,
                                       const std::vector<lxb_dom_element_t *> &nodes) {
    const auto bytes = allocation_size(AllocationHeader::Kind::ElementArray, nodes.size());
    auto *base = static_cast<unsigned char *>(std::malloc(bytes));
    if (base == nullptr) throw std::bad_alloc();

    auto *header = reinterpret_cast<AllocationHeader *>(base);
    *header = {AllocationHeader::Kind::ElementArray, nodes.size()};
    auto *elements = reinterpret_cast<crawler_element_t *>(base + sizeof(*header));
    for (size_t index = 0; index < nodes.size(); ++index) {
        new (elements + index) crawler_element_t{owner, nodes[index]};
    }
    return elements;
}

void free_handle(void *value) {
    if (value == nullptr) return;

    auto *base = static_cast<unsigned char *>(value) - sizeof(AllocationHeader);
    auto *header = reinterpret_cast<AllocationHeader *>(base);
    auto *elements = static_cast<crawler_element_t *>(value);
    for (size_t index = 0; index < header->count; ++index) {
        elements[index].~crawler_element_t();
    }
    std::free(base);
}

} // namespace libcrawler::detail

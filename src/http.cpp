#include "stdafx.hpp"
#include "internal.hpp"

namespace libcrawler::detail {

void CurlDeleter::operator()(CURL *handle) const noexcept {
    if (handle != nullptr) curl_easy_cleanup(handle);
}

namespace {

size_t write_body(char *data, size_t size, size_t count, void *context) {
    auto &body = *static_cast<std::string *>(context);
    body.append(data, size * count);
    return size * count;
}

size_t read_header(char *data, size_t size, size_t count, void *context) {
    auto &content_type = *static_cast<std::string *>(context);
    const std::string_view line(data, size * count);
    constexpr std::string_view prefix = "Content-Type:";
    if (line.size() < prefix.size() || line.substr(0, prefix.size()) != prefix) return size * count;

    auto value = line.substr(prefix.size());
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) value.remove_prefix(1);
    content_type = media_type(value);
    return size * count;
}

CurlHandle create_curl_handle(const char *url, Response &response) {
    CurlHandle curl(curl_easy_init());
    if (!curl) throw std::runtime_error("unable to initialize libcurl");

    curl_easy_setopt(curl.get(), CURLOPT_URL, url);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_body);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, read_header);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &response.content_type);

#ifdef LIBCRAWLER_HAS_IMPERSONATE
    extern CURLcode curl_easy_impersonate(CURL *, const char *, int);
    const char *browser = std::getenv("LIBCRAWLER_BROWSER");
    const char *target = browser == nullptr ? "chrome116" : browser;
    if (curl_easy_impersonate(curl.get(), target, 1) != CURLE_OK) {
        throw std::runtime_error("curl_easy_impersonate failed");
    }
#else
    throw std::runtime_error("library was not built with libcurl-impersonate");
#endif

    return curl;
}

} // namespace

Response fetch_response(const char *url) {
    if (url == nullptr) throw std::invalid_argument("url must not be null");
    Response response;
    auto curl = create_curl_handle(url, response);
    if (const auto code = curl_easy_perform(curl.get()); code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
    }
    return response;
}

std::string media_type(std::string_view content_type) {
    const auto end = content_type.find(';');
    auto result = std::string(content_type.substr(0, end));
    while (!result.empty() && (result.back() == ' ' || result.back() == '\t')) result.pop_back();
    return result;
}

bool is_json(std::string_view type) {
    return type == "application/json" ||
           (type.size() >= 5 && type.substr(type.size() - 5) == "+json");
}

bool is_jsonp(std::string_view type) {
    return type == "application/javascript" || type == "text/javascript" ||
           type == "application/x-javascript";
}

std::string unwrap_jsonp(std::string_view body) {
    const auto open = body.find('(');
    const auto close = body.rfind(')');
    if (open == std::string_view::npos || close == std::string_view::npos || close <= open) return {};
    return std::string(body.substr(open + 1, close - open - 1));
}

} // namespace libcrawler::detail

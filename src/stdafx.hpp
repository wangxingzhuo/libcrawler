#pragma once

#include "libcrawler.h"

#include <curl/curl.h>
#include <lexbor/css/parser.h>
#include <lexbor/css/selectors/selectors.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/dom/interfaces/node.h>
#include <lexbor/html/parser.h>
#include <lexbor/selectors/selectors.h>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

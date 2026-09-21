BUILD_DIR ?= build
CONFIG := $(BUILD_DIR)/config.mk
comma := ,

ifneq ($(wildcard $(CONFIG)),)
include $(CONFIG)
else
$(error $(CONFIG) is missing; run ./configure.sh first)
endif

CXX ?= c++
AR ?= ar
CPPFLAGS += -I$(PROJECT_ROOT)/include -I$(LEXBOR_INCLUDE_DIR)
CXXFLAGS += -std=c++23 -Wall -Wextra -Wpedantic -pthread -include $(PROJECT_ROOT)/src/stdafx.hpp
LDFLAGS += -L$(IMPERSONATE_DIR)
LDLIBS += $(LEXBOR_LIBRARY) $(IMPERSONATE_LIBRARY) $(PLATFORM_LDLIBS) -pthread

SOURCES := src/libcrawler.cpp src/state.cpp src/http.cpp src/dom.cpp src/memory.cpp
OBJECTS := $(SOURCES:src/%.cpp=$(BUILD_DIR)/%.o)
TARGET := $(BUILD_DIR)/libcrawler.$(SHARED_EXT)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS) $(LEXBOR_LIBRARY) $(IMPERSONATE_LIBRARY)
	@mkdir -p $(dir $@)
	$(CXX) $(SHARED_FLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.cpp $(CONFIG) src/stdafx.hpp src/internal.hpp include/libcrawler.h
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

-include $(OBJECTS:.o=.d)

clean:
	rm -rf $(BUILD_DIR)
BUILD_DIR ?= build
CONFIG := $(BUILD_DIR)/config.mk
comma := ,

ifneq ($(wildcard $(CONFIG)),)
include $(CONFIG)
else
$(error $(CONFIG) is missing; run ./configure.sh first)
endif

CC ?= gcc
CXX ?= g++
AR ?= ar
CPPFLAGS += -I$(PROJECT_ROOT)/include -I$(PROJECT_ROOT)/src -I$(LEXBOR_INCLUDE_DIR)/..

CFLAGS =   -std=c2x   $(CPPFLAGS) -Wall -Wextra -Wpedantic -pthread -fPIC
CXXFLAGS = -std=c++23 $(CPPFLAGS) -Wall -Wextra -Wpedantic -pthread -fPIC
LDFLAGS += -L$(IMPERSONATE_DIR)
LDLIBS += $(LEXBOR_LIBRARY) $(IMPERSONATE_LIBRARY) $(PLATFORM_LDLIBS) -pthread

SOURCES := src/client.c src/element.cpp
OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(SOURCES))) \
	$(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(filter %.cpp,$(SOURCES)))
TARGET := $(BUILD_DIR)/libcrawler.$(SHARED_EXT)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(SHARED_FLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c $(CONFIG) src/stdafx.h include/libcrawler.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.cpp $(CONFIG) src/stdafx.h include/libcrawler.h
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

-include $(OBJECTS:.o=.d)

clean:
	rm -rf $(BUILD_DIR)
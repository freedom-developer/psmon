CC ?= cc

CPPFLAGS += -I$(abspath ../include/uapi) -I$(CURDIR)/common
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic

BUILD_DIR := build
COMMON_OBJ := $(BUILD_DIR)/psmon_client.o
BINARIES := $(BUILD_DIR)/psmonctl $(BUILD_DIR)/psmon-agent

.PHONY: all test clean

all: $(BINARIES)

$(BUILD_DIR):
	mkdir -p $@

$(COMMON_OBJ): common/psmon_client.c common/psmon_client.h ../include/uapi/psmon.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/psmonctl: apps/psmonctl/main.c $(COMMON_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/psmon-agent: apps/psmon-agent/main.c $(COMMON_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

test: all
	$(BUILD_DIR)/psmonctl --version
	$(BUILD_DIR)/psmon-agent --version

clean:
	rm -rf $(BUILD_DIR)

CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -I./include
LDFLAGS := -lcurl -ljson-c

ifdef DEBUG
    CFLAGS += -g -O0 -DDEBUG
else
    CFLAGS += -O2
endif

SRC_DIR := src
BUILD_DIR := build
TEST_DIR := tests

COMMON_SRCS := $(wildcard $(SRC_DIR)/common/*.c)
IPMI_SRCS := $(wildcard $(SRC_DIR)/ipmi/*.c)
REDFISH_SRCS := $(wildcard $(SRC_DIR)/redfish/*.c)
CLI_SRCS := $(wildcard $(SRC_DIR)/cli/*.c)

COMMON_OBJS := $(COMMON_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
IPMI_OBJS := $(IPMI_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
REDFISH_OBJS := $(REDFISH_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
CLI_OBJS := $(CLI_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TARGET := bmctool
TEST_COMMON := test_common
TEST_IPMI_PACKET := test_ipmi_packet

.PHONY: all
all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/common
	mkdir -p $(BUILD_DIR)/ipmi
	mkdir -p $(BUILD_DIR)/redfish
	mkdir -p $(BUILD_DIR)/cli

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(COMMON_OBJS) $(IPMI_OBJS) $(REDFISH_OBJS) $(CLI_OBJS)
	@echo "Linking $@..."
	$(CC) $^ -o $@ $(LDFLAGS)

$(TEST_COMMON): $(TEST_DIR)/test_common.c $(COMMON_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_IPMI_PACKET): $(TEST_DIR)/test_ipmi_packet.c $(COMMON_OBJS) $(IPMI_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: test
test: $(TEST_COMMON) $(TEST_IPMI_PACKET)
	@echo "=== Running Common Tests ==="
	./$(TEST_COMMON)
	@echo ""
	@echo "=== Running IPMI Packet Tests ==="
	./$(TEST_IPMI_PACKET)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(TEST_COMMON) $(TEST_IPMI_PACKET)

.PHONY: help
help:
	@echo "Targets:"
	@echo "  all      - Build bmctool"
	@echo "  test     - Run tests"
	@echo "  clean    - Clean build"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1  - Build with debug symbols"

TARGET ?=
ROOT := $(CURDIR)
BUILD := build

Z88DK_HOME ?= $(abspath ../z88dk)
ifneq ($(wildcard $(Z88DK_HOME)/bin/zcc),)
export PATH := $(Z88DK_HOME)/bin:$(PATH)
export ZCCCFG := $(Z88DK_HOME)/lib/config
endif

ZCC ?= zcc
ZESARUX ?= /Applications/ZEsarUX.app/Contents/MacOS/zesarux
ZESARUX_DIR := $(dir $(ZESARUX))

COMMON_C := src/main.c src/video.c src/globe.c src/fxtab.c
HEADERS := $(wildcard include/*.h)
COMMON_C_ABS := $(addprefix $(ROOT)/,$(COMMON_C))
ZCC_BASE := $(ZCC) +zx -startup=31 -SO3 -clib=sdcc_iy \
	-iquote$(ROOT)/include -pragma-define:CLIB_MALLOC_HEAP_SIZE=0

ifneq ($(strip $(TARGET)),)
.DEFAULT_GOAL := target
else
.DEFAULT_GOAL := all
endif

.PHONY: all target timex zx128 test clean run-tc2048 run-tc2068 run-zx128

all: timex zx128

target:
	@test -n "$(strip $(TARGET))" || { echo "usage: make TARGET=<timex|zx128|all|clean>" >&2; exit 2; }
	@$(MAKE) --no-print-directory $(TARGET)

timex: $(BUILD)/planet-timex.tap
zx128: $(BUILD)/planet-zx128.tap

test:
	./test/run.sh

$(BUILD):
	mkdir -p $@

$(BUILD)/planet-timex.tap: $(COMMON_C) $(HEADERS) | $(BUILD)
	mkdir -p $(BUILD)/obj-timex
	cd $(BUILD)/obj-timex && $(ZCC_BASE) \
		$(COMMON_C_ABS) \
		-o $(ROOT)/$(BUILD)/planet-timex -create-app -m
	ls -l $@

$(BUILD)/planet-zx128.tap: $(COMMON_C) src/zx128_page.asm $(HEADERS) tools/check_zx128_layout.py | $(BUILD)
	mkdir -p $(BUILD)/obj-zx128
	cd $(BUILD)/obj-zx128 && $(ZCC_BASE) \
		-DZX128_PAGE_FLIP -Ca-DZX128_PAGE_FLIP \
		-pragma-define:REGISTER_SP=49152 \
		$(COMMON_C_ABS) $(ROOT)/src/zx128_page.asm \
		-o $(ROOT)/$(BUILD)/planet-zx128 -create-app -m
	tools/check_zx128_layout.py $(BUILD)/planet-zx128.map
	ls -l $@

run-tc2048: timex
	@test -x "$(ZESARUX)" || { echo "ZEsarUX binary not found: $(ZESARUX)" >&2; exit 1; }
	cd "$(ZESARUX_DIR)" && ./zesarux --noconfigfile --machine TC2048 \
		--enabletimexvideo --nosplash --verbose 0 "$(ROOT)/$(BUILD)/planet-timex.tap"

run-tc2068: timex
	@test -x "$(ZESARUX)" || { echo "ZEsarUX binary not found: $(ZESARUX)" >&2; exit 1; }
	cd "$(ZESARUX_DIR)" && ./zesarux --noconfigfile --machine TC2068 \
		--enabletimexvideo --nosplash --verbose 0 "$(ROOT)/$(BUILD)/planet-timex.tap"

run-zx128: zx128
	@test -x "$(ZESARUX)" || { echo "ZEsarUX binary not found: $(ZESARUX)" >&2; exit 1; }
	cd "$(ZESARUX_DIR)" && ./zesarux --noconfigfile --machine 128k \
		--nosplash --verbose 0 "$(ROOT)/$(BUILD)/planet-zx128.tap"

clean:
	rm -rf $(BUILD)

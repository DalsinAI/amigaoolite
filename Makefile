# GFOS native Oolite world-generation portability gate.
# Upstream source is NOT vendored. Clone OoliteProject/oolite and pass UPSTREAM.
UPSTREAM ?= ../oolite
PREFIX ?= /tmp/atlas-batch10-20261117/stove-isolated/prefix
OUT ?= out
CROSS_CC ?= $(PREFIX)/bin/m68k-amigaos-gcc
HOST_CC ?= cc
RNG_SRC := $(UPSTREAM)/src/Core/legacy_random.c
RNG_INC := $(UPSTREAM)/src/Core
RNG_GATE := tests/oolite_rng_gate.c
.PHONY: all host-check clean
all: $(OUT)/oolite-rng-68040
$(OUT)/oolite-rng-68040: $(RNG_SRC) $(RNG_GATE)
	@mkdir -p $(OUT)
	$(CROSS_CC) -std=gnu11 -O2 -m68040 -m68881 -noixemul -I $(RNG_INC) $(RNG_GATE) $(RNG_SRC) -lm -o $@
host-check: $(RNG_SRC) $(RNG_GATE)
	@mkdir -p $(OUT)
	$(HOST_CC) -std=gnu11 -O2 -I $(RNG_INC) $(RNG_GATE) $(RNG_SRC) -lm -o $(OUT)/oolite-rng-host
	@$(OUT)/oolite-rng-host
clean:
	rm -rf $(OUT)

# Requires applying patches/0001-gfos-native-GL-geometry.patch to pinned
# upstream. Deliberately does NOT substitute fake SDL3/GLU headers.
.PHONY: planet-object
planet-object: $(OUT)/OOPlanetData.o
$(OUT)/OOPlanetData.o: $(UPSTREAM)/src/Core/OOPlanetData.c
	@mkdir -p $(OUT)
	$(CROSS_CC) -std=gnu11 -O2 -DOOLITE_GFOS=1 -m68040 -m68881 -noixemul -I $(PREFIX)/m68k-amigaos/include -I $(RNG_INC) -c $< -o $@

# Real upstream Objective-C Oolite Foundation module (no stubbed engine code).
GFOS_BASE_DIR ?= /tmp/gfopenfoundation-build-20261118/gnustep-libs-base-b1bfc5b
GFOS_LIB_DIR ?= /tmp/gfopengnustep-20261118/ci-check/out
GFOS_FFI_DIR ?= /tmp/gfopenfoundation-build-20261118/target-libffi
GFOS_BASE_ARCHIVE ?= $(GFOS_BASE_DIR)/Source/obj/libgnustep-base.a
GFOS_DEPS_LIB ?= $(PREFIX)/m68k-amigaos/port-deps/lib
GFOS_LIBNIX_LIB ?= $(PREFIX)/m68k-amigaos/libnix/lib
$(OUT)/OOIsNumberLiteral.o: $(UPSTREAM)/src/Core/OOIsNumberLiteral.m $(UPSTREAM)/src/Core/OOIsNumberLiteral.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul -DGNUSTEP_BASE_LIBRARY=1 \
	-I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(UPSTREAM)/src/SDL \
	-I $(PREFIX)/m68k-amigaos/ndk-include.vendor -c $< -o $@
$(OUT)/oolite_foundation_gate.o: tests/oolite_foundation_gate.m $(UPSTREAM)/src/Core/OOIsNumberLiteral.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul -DGNUSTEP_BASE_LIBRARY=1 \
	-I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(UPSTREAM)/src/SDL \
	-I $(PREFIX)/m68k-amigaos/ndk-include.vendor -c $< -o $@
$(OUT)/oolite-foundation-68040: $(OUT)/oolite_foundation_gate.o $(OUT)/OOIsNumberLiteral.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $^ -L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) \
	-Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: foundation-number
foundation-number: $(OUT)/oolite-foundation-68040

# Native GFOS Foundation/AmigaDOS file-data gate for Oolite resource loading.
# Stage tests/fixtures/oolite-probe.txt at SYS:ACAgent/oolite-probe.txt in the
# Amiga guest before running. DH1 is NOT assumed to be mounted.
$(OUT)/oolite_resource_probe.o: tests/oolite_resource_probe.m
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul \
	-I $(GFOS_BASE_DIR)/Headers -c $< -o $@
$(OUT)/oolite-resource-68040: $(OUT)/oolite_resource_probe.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $< -L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) \
	-Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: resource-probe
resource-probe: $(OUT)/oolite-resource-68040

# Loads actual, unmodified Oolite Config/*.plist via native AmigaDOS and
# GFOpenGNUstep NSPropertyListSerialization. Fixture assets must be retrieved
# from the pinned upstream Oolite commit and staged in SYS:Oolite/Config/.
$(OUT)/oolite_actual_plist_probe.o: tests/oolite_actual_plist_probe.m
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul \
	-I $(GFOS_BASE_DIR)/Headers -c $< -o $@
$(OUT)/oolite-real-assets-68040: $(OUT)/oolite_actual_plist_probe.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $< -L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) \
	-Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: real-assets
real-assets: $(OUT)/oolite-real-assets-68040

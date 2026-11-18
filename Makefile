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

# Original Oolite property-list parser with a GFOS early-boot patch.
# Apply patches/0002-gfos-native-plain-plist.patch to pinned UPSTREAM once.
$(OUT)/OOPListParsing.o: $(UPSTREAM)/src/Core/OOPListParsing.m $(UPSTREAM)/src/Core/OOPListParsing.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul -DOOLITE_GFOS=1 \
	-I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -c $< -o $@
$(OUT)/oolite_upstream_parser_test.o: tests/oolite_upstream_parser_test.m $(UPSTREAM)/src/Core/OOPListParsing.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul -DOOLITE_GFOS=1 \
	-I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -c $< -o $@
$(OUT)/oolite-upstream-plist-68040: $(OUT)/OOPListParsing.o $(OUT)/oolite_upstream_parser_test.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $^ -L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) \
	-Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: upstream-plist
upstream-plist: $(OUT)/oolite-upstream-plist-68040

# Real upstream RanrotWithSeed seeded chunks: deterministic single CPU0
# baseline for future Team 4 OMC_RunRange speedup tests, not multicore yet.
$(OUT)/oolite-rng-cpu0-68040: tests/oolite_rng_cpu0_baseline.c $(RNG_SRC)
	@mkdir -p $(OUT)
	$(CROSS_CC) -std=gnu11 -O2 -m68040 -m68881 -noixemul -I $(RNG_INC) $^ -lm -o $@
$(OUT)/oolite-rng-cpu0-host: tests/oolite_rng_cpu0_baseline.c $(RNG_SRC)
	@mkdir -p $(OUT)
	$(HOST_CC) -std=gnu11 -O2 -I $(RNG_INC) $^ -lm -o $@
.PHONY: rng-cpu0-baseline rng-cpu0-host-check
rng-cpu0-baseline: $(OUT)/oolite-rng-cpu0-68040
rng-cpu0-host-check: $(OUT)/oolite-rng-cpu0-host
	$(OUT)/oolite-rng-cpu0-host

# Oolite numeric workload via Team 4's canonical gfopenmulticore.library.
# Requires Team 4 header-only include root from gfopenmulticore main, NOT
# vendored or copied into this repository. Real OMC_RunRange / OMC_AddGrant.
OMC_INCLUDE ?= ../gfopenmulticore/include
$(OUT)/oolite-rng-omc-range-68040: tests/oolite_rng_omc_range.c $(RNG_SRC)
	@mkdir -p $(OUT)
	$(CROSS_CC) -std=gnu11 -O2 -m68040 -m68881 -noixemul \
	-I $(OMC_INCLUDE) -I $(PREFIX)/m68k-amigaos/ndk-include.vendor \
	-I $(RNG_INC) $^ -lm -o $@
.PHONY: rng-omc-range
rng-omc-range: $(OUT)/oolite-rng-omc-range-68040

# OXZ preflight using the pinned upstream MiniZip library. Apply patch 0005
# to UPSTREAM/src/Core/MiniZip/ioapi.c before invoking this target. Do not
# enable full OXZ installation on the strength of this qualification alone.
OXZ_MINIZIP := $(UPSTREAM)/src/Core/MiniZip
OXZ_DEPS := $(PREFIX)/m68k-amigaos/port-deps
OXZ_FLAGS := -std=gnu11 -O2 -m68040 -m68881 -noixemul -DOOLITE_GFOS=1 -I src/Core -I $(OXZ_MINIZIP) -I $(OXZ_DEPS)/include
$(OUT)/oolite-oxz-preflight-68040: tests/oolite_oxz_preflight_gate.c src/Core/OOOXZArchivePreflight.c src/Core/OOOXZPathSafety.h src/Core/OOOXZArchivePreflight.h $(OXZ_MINIZIP)/ioapi.c $(OXZ_MINIZIP)/unzip.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(OXZ_FLAGS) tests/oolite_oxz_preflight_gate.c src/Core/OOOXZArchivePreflight.c $(OXZ_MINIZIP)/ioapi.c $(OXZ_MINIZIP)/unzip.c -L $(OXZ_DEPS)/lib -lz -o $@
.PHONY: oxz-preflight
oxz-preflight: $(OUT)/oolite-oxz-preflight-68040

# Native Oolite OXZ extraction and failed-install cleanup gate.
# Requires the pinned upstream MiniZip with patch 0005 applied.
# The GFOS OOOXZManager bridge is patch 0008; this target exercises its
# extraction helper without falsely claiming the whole manager is linked.
$(OUT)/oolite-oxz-extraction-68040: tests/oolite_oxz_rollback_gate.m src/Core/OOOXZGFOSExtraction.m src/Core/OOOXZGFOSExtraction.h src/Core/OOOXZArchivePreflight.c src/Core/OOOXZPathSafety.h $(OXZ_MINIZIP)/ioapi.c $(OXZ_MINIZIP)/unzip.c
	@mkdir -p $(OUT)
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -I $(GFOS_BASE_DIR)/Headers -I src/Core -I $(OXZ_MINIZIP) -I $(OXZ_DEPS)/include -c tests/oolite_oxz_rollback_gate.m -o $(OUT)/oolite_oxz_rollback_gate.o
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -I $(GFOS_BASE_DIR)/Headers -I src/Core -I $(OXZ_MINIZIP) -I $(OXZ_DEPS)/include -c src/Core/OOOXZGFOSExtraction.m -o $(OUT)/OOOXZGFOSExtraction.o
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -I src/Core -I $(OXZ_MINIZIP) -I $(OXZ_DEPS)/include -c src/Core/OOOXZArchivePreflight.c -o $(OUT)/OOOXZArchivePreflight.o
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -I $(OXZ_MINIZIP) -I $(OXZ_DEPS)/include -DOOLITE_GFOS=1 -c $(OXZ_MINIZIP)/ioapi.c -o $(OUT)/oolite_oxz_ioapi.o
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -I $(OXZ_MINIZIP) -I $(OXZ_DEPS)/include -DOOLITE_GFOS=1 -c $(OXZ_MINIZIP)/unzip.c -o $(OUT)/oolite_oxz_unzip.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite_oxz_rollback_gate.o $(OUT)/OOOXZGFOSExtraction.o $(OUT)/OOOXZArchivePreflight.o $(OUT)/oolite_oxz_ioapi.o $(OUT)/oolite_oxz_unzip.o -L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive $(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: oxz-extract
oxz-extract: $(OUT)/oolite-oxz-extraction-68040

# Four-field Oolite OXZ manifest schema qualification with original Oolite
# MiniZip and GNUstep Base; not a full OOOXZManager build or version gate.
$(OUT)/oolite-oxz-schema-68040: tests/oolite_oxz_manifest_schema_gate.m $(OXZ_MINIZIP)/ioapi.c $(OXZ_MINIZIP)/unzip.c $(UPSTREAM)/src/Core/OOManifestProperties.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(OXZ_MINIZIP) -I $(OXZ_DEPS)/include -c tests/oolite_oxz_manifest_schema_gate.m -o $(OUT)/oolite_oxz_schema_gate.o
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -DOOLITE_GFOS=1 -I $(OXZ_MINIZIP) -I $(OXZ_DEPS)/include -c $(OXZ_MINIZIP)/ioapi.c -o $(OUT)/oolite_oxz_schema_ioapi.o
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -DOOLITE_GFOS=1 -I $(OXZ_MINIZIP) -I $(OXZ_DEPS)/include -c $(OXZ_MINIZIP)/unzip.c -o $(OUT)/oolite_oxz_schema_unzip.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite_oxz_schema_gate.o $(OUT)/oolite_oxz_schema_ioapi.o $(OUT)/oolite_oxz_schema_unzip.o -L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive $(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: oxz-schema
oxz-schema: $(OUT)/oolite-oxz-schema-68040

# Original upstream Oolite quaternion/matrix unit against native OpenGPU GL,
# requiring pinned upstream patches 0001 and 0009 (not fake SDL headers).
$(OUT)/oolite-math-68040: tests/oolite_math_68040_gate.c $(UPSTREAM)/src/Core/OOMaths.h $(UPSTREAM)/src/Core/OOQuaternion.h $(UPSTREAM)/src/Core/OOMatrix.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -DOOLITE_GFOS=1 -m68040 -m68881 -noixemul -I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(UPSTREAM)/src/Core/Tables -I $(PREFIX)/m68k-amigaos/include -I $(OXZ_DEPS)/include tests/oolite_math_68040_gate.c -L $(OXZ_DEPS)/lib -L $(GFOS_LIBNIX_LIB) -lm -lobjc -o $@
.PHONY: math-68040
math-68040: $(OUT)/oolite-math-68040

# Extract and parse *real* original upstream Oolite Config/shipdata.plist from
# an OXZ assembled locally from the pinned source. The OXZ asset is not
# committed here; stage as SYS:Oolite/real-assets.oxz for guest qualification.
$(OUT)/oolite-oxz-real-assets-68040: tests/oolite_oxz_real_assets_gate.m $(OUT)/oolite-oxz-extraction-68040
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -I $(GFOS_BASE_DIR)/Headers -I src/Core -c tests/oolite_oxz_real_assets_gate.m -o $(OUT)/oolite_oxz_real_assets_gate.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite_oxz_real_assets_gate.o $(OUT)/OOOXZGFOSExtraction.o $(OUT)/OOOXZArchivePreflight.o $(OUT)/oolite_oxz_ioapi.o $(OUT)/oolite_oxz_unzip.o -L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive $(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: oxz-real-assets
oxz-real-assets: $(OUT)/oolite-oxz-real-assets-68040

# Canonical GFOpenSDK javascript.library LVO ABI (no embedded JS runtime).
# Set GFOS_JS_SDK to the GFOpenSDK checkout root; do not copy SDK headers.
GFOS_JS_SDK ?= ../gfopensdk
GFOS_JS_INC := $(GFOS_JS_SDK)/Include
GFOS_JS_NDK := $(PREFIX)/m68k-amigaos/ndk-include.vendor
GFOS_JS_FLAGS := -std=gnu11 -O2 -m68040 -m68881 -noixemul -I $(GFOS_JS_INC) -I $(GFOS_JS_NDK) -I src/Core
$(OUT)/oolite-jslib-68040: tests/oolite_jslib_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h $(GFOS_JS_INC)/libraries/javascript.h $(GFOS_JS_INC)/inline/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_jslib_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: jslib-bridge
jslib-bridge: $(OUT)/oolite-jslib-68040

# Pinned upstream Oolite SpiderMonkey 1.8.5 native interpreter gate.
# Requires the *separate* OoliteProject/mozillajs-linux pinned source tree
# already configured for big-endian 68040 and built with patch 0010.
# Do not substitute a Linux js-config.h or incompatible modern JS engine.
MOZJS ?= ../mozillajs-linux
MOZJS_BUILD ?= $(MOZJS)/js/src/gfos-cross-build
CROSS_CXX ?= $(PREFIX)/bin/m68k-amigaos-g++
MOZJS_STATIC := $(MOZJS_BUILD)/libjs_static.a
MOZJS_INCLUDE := -I $(MOZJS)/js/src -I $(MOZJS_BUILD)
$(OUT)/oolite-mozjs-68040: tests/oolite_js_runtime_gate.cpp $(MOZJS_STATIC)
	@mkdir -p $(OUT)
	$(CROSS_CXX) -std=gnu++11 -O2 -m68040 -m68881 -noixemul $(MOZJS_INCLUDE) -c tests/oolite_js_runtime_gate.cpp -o $(OUT)/oolite_js_runtime_gate.o
	$(CROSS_CXX) -O2 -m68040 -m68881 -noixemul $(OUT)/oolite_js_runtime_gate.o -L $(GFOS_DEPS_LIB) -Wl,--start-group $(MOZJS_STATIC) -lpthread -lm -lsocket -lz -Wl,--end-group -o $@
.PHONY: mozjs-68040
mozjs-68040: $(OUT)/oolite-mozjs-68040

# Additional CPU0 JavaScript object, array, string and forced GC guest gate.
$(OUT)/oolite-mozjs-stage-68040: tests/oolite_js_stage_gate.cpp $(MOZJS_STATIC)
	@mkdir -p $(OUT)
	$(CROSS_CXX) -std=gnu++11 -O2 -m68040 -m68881 -noixemul $(MOZJS_INCLUDE) -c tests/oolite_js_stage_gate.cpp -o $(OUT)/oolite_js_stage_gate.o
	$(CROSS_CXX) -O2 -m68040 -m68881 -noixemul $(OUT)/oolite_js_stage_gate.o -L $(GFOS_DEPS_LIB) -Wl,--start-group $(MOZJS_STATIC) -lpthread -lm -lsocket -lz -Wl,--end-group -o $@
.PHONY: mozjs-stage
mozjs-stage: $(OUT)/oolite-mozjs-stage-68040

# Full *upstream* OOOXZManager.m object qualification for first 68040 boot.
# Apply pinned Oolite patches 0001, 0004, 0006, 0007, 0008, 0009, 0011
# and separate pinned mozillajs-linux patch 0010. This is NOT whole-game link.
# Sound is explicitly disabled until the real OpenAudio bridge is qualified;
# no fabricated OpenAL or SDL3 headers are used in this native bootstrap.
$(OUT)/OOOXZManager-68040.o: $(UPSTREAM)/src/Core/OOOXZManager.m $(UPSTREAM)/src/Core/OOSound.h $(UPSTREAM)/src/Core/Universe.h $(MOZJS_BUILD)/js-config.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul \
	-DOOLITE_GFOS=1 -DOOLITE_OPENAL=0 -DOOLITE_ESPEAK=0 -DGNUSTEP_BASE_LIBRARY=1 \
	-I $(GFOS_BASE_DIR)/Headers -I src/Core -I $(UPSTREAM)/src/Core \
	-I $(UPSTREAM)/src/Core/Entities -I $(UPSTREAM)/src/Core/Materials \
	-I $(UPSTREAM)/src/Core/Tables -I $(UPSTREAM)/src/Core/MiniZip \
	-I $(UPSTREAM)/src/Core/Scripting -I $(UPSTREAM)/src/SDL \
	-I $(MOZJS)/js/src -I $(MOZJS_BUILD) \
	-I $(PREFIX)/m68k-amigaos/include -I $(PREFIX)/m68k-amigaos/ndk-include.vendor \
	-I $(PREFIX)/m68k-amigaos/port-deps/include -c $< -o $@
.PHONY: oxz-manager-object
oxz-manager-object: $(OUT)/OOOXZManager-68040.o

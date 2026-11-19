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

# Native script-to-Oolite-host-object binding gate, on the same shared engine.
$(OUT)/oolite-jslib-hostclass-68040: tests/oolite_jslib_hostclass_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h $(GFOS_JS_INC)/libraries/javascript.h $(GFOS_JS_INC)/inline/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_jslib_hostclass_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: jslib-hostclass
jslib-hostclass: $(OUT)/oolite-jslib-hostclass-68040

# Nonblocking on-guest installation check, before starting the heavy engine.
$(OUT)/oolite-jslib-install-68040: tests/oolite_jslib_install_probe.c
	@mkdir -p $(OUT)
	$(CROSS_CC) -std=gnu11 -O2 -m68040 -m68881 -noixemul -I $(GFOS_JS_NDK) $< -o $@
.PHONY: jslib-install
jslib-install: $(OUT)/oolite-jslib-install-68040

# Load unmodified pinned Oolite Resources/Scripts/*.js from SYS:Oolite/Scripts
# and execute through the canonical shared javascript.library, without bundling JS.
$(OUT)/oolite-real-js-68040: tests/oolite_real_js_guest_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h $(GFOS_JS_INC)/libraries/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_real_js_guest_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: real-js
real-js: $(OUT)/oolite-real-js-68040

# Exercise the original upstream oolite-default-ship-script.js shipSpawned
# event using a canonical GFOS JS host class/native C legacy action callback.
# Stage the script at SYS:Oolite/Scripts/ (do not vendor upstream assets).
$(OUT)/oolite-ship-event-68040: tests/oolite_ship_script_event_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h $(GFOS_JS_INC)/libraries/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_ship_script_event_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: ship-event
ship-event: $(OUT)/oolite-ship-event-68040

# Actual Oolite default ship script: setup, launch, death and missile events
# through canonical GFOS host classes; stage pinned upstream source separately.
$(OUT)/oolite-ship-actions-68040: tests/oolite_ship_script_actions_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h $(GFOS_JS_INC)/libraries/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_ship_script_actions_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: ship-actions
ship-actions: $(OUT)/oolite-ship-actions-68040

# Real Oolite tutorial, populator and equipment conditions script gate.
# Stage originals from pinned Oolite upstream in SYS:Oolite/Scripts.
$(OUT)/oolite-world-js-68040: tests/oolite_world_scripts_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h $(GFOS_JS_INC)/libraries/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_world_scripts_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: world-js
world-js: $(OUT)/oolite-world-js-68040

# Verify two wrappers of the same Oolite native ship preserve identity while
# a distinct ship stays distinct. Requires working shared javascript.library.
$(OUT)/oolite-js-identity-68040: tests/oolite_jslib_identity_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h $(GFOS_JS_INC)/libraries/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_jslib_identity_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-identity
js-identity: $(OUT)/oolite-js-identity-68040

# JS host-object release on script task shutdown (does not prove in-task GC).
$(OUT)/oolite-js-release-68040: tests/oolite_jslib_release_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h $(GFOS_JS_INC)/libraries/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_jslib_release_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-release
js-release: $(OUT)/oolite-js-release-68040

# Original OOJSShip numeric property names via safe script-task accessors;
# guest test uses a controlled backend, NOT the full ShipEntity implementation.
$(OUT)/oolite-ship-properties-68040: tests/oolite_ship_properties_gate.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSShipProperties.h src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h $(GFOS_JS_INC)/libraries/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_ship_properties_gate.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: ship-properties
ship-properties: $(OUT)/oolite-ship-properties-68040

# Reject out-of-range and stale OOJSShip property operations without mutation.
$(OUT)/oolite-ship-reject-68040: tests/oolite_ship_properties_reject_gate.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSShipProperties.h src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_ship_properties_reject_gate.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: ship-reject
ship-reject: $(OUT)/oolite-ship-reject-68040

# Experimental bounded Oolite game-task property IPC using Exec message ports.
# Javascript executes on the shared JS task; owner services requests on game task.
$(OUT)/oolite-js-game-dispatch-68040: tests/oolite_js_game_dispatch_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSGameDispatch.h src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-game-dispatch
js-game-dispatch: $(OUT)/oolite-js-game-dispatch-68040

# Reject invalid ship writes across actual JS task-to-game task Exec IPC.
$(OUT)/oolite-js-game-reject-68040: tests/oolite_js_game_dispatch_reject_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSGameDispatch.h src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_reject_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-game-reject
js-game-reject: $(OUT)/oolite-js-game-reject-68040

# Two independent JavaScript task heaps issuing concurrent Exec RPCs to one game owner.
$(OUT)/oolite-js-two-tasks-68040: tests/oolite_js_game_dispatch_two_tasks_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSGameDispatch.h src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_two_tasks_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-two-tasks
js-two-tasks: $(OUT)/oolite-js-two-tasks-68040

# Exec-port shutdown rejects active clients, self-deadlock and post-close access.
$(OUT)/oolite-js-safe-close-68040: tests/oolite_js_game_dispatch_safe_close_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSGameDispatch.h src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_safe_close_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-safe-close
js-safe-close: $(OUT)/oolite-js-safe-close-68040

# Seal game task IPC so queued JS jobs can complete without admitting new ship operations.
$(OUT)/oolite-js-seal-68040: tests/oolite_js_game_dispatch_seal_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSGameDispatch.h src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_seal_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-seal
js-seal: $(OUT)/oolite-js-seal-68040

# Two ship tokens, game-owner routing and stale token rejection through JS Exec IPC.
$(OUT)/oolite-js-ship-routes-68040: tests/oolite_js_game_dispatch_route_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSGameDispatch.h src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_route_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-ship-routes
js-ship-routes: $(OUT)/oolite-js-ship-routes-68040

# Controlled game owner cancellation of queued JS property messages during seal.
$(OUT)/oolite-js-cancel-68040: tests/oolite_js_game_dispatch_cancel_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSGameDispatch.h src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_cancel_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-cancel
js-cancel: $(OUT)/oolite-js-cancel-68040

# Native 68040 Oolite game-task 4095-slot generation tokens; stale slot reuse.
$(OUT)/oolite-js-token-table-68040: tests/oolite_js_game_dispatch_token_table_gate.c src/Core/OOGFOSShipTokens.c src/Core/OOGFOSShipTokens.h src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_token_table_gate.c src/Core/OOGFOSShipTokens.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-token-table
js-token-table: $(OUT)/oolite-js-token-table-68040

# Standalone full-capacity, overflow and generation retirement test.
$(OUT)/oolite-js-token-capacity-68040: tests/oolite_ship_token_capacity_gate.c src/Core/OOGFOSShipTokens.c src/Core/OOGFOSShipTokens.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_ship_token_capacity_gate.c src/Core/OOGFOSShipTokens.c -o $@
.PHONY: js-token-capacity
js-token-capacity: $(OUT)/oolite-js-token-capacity-68040

# GFOS shared javascript.library -> Exec game task -> actual GNU Objective-C
# selectors, with pinned Oolite OOTypes; test uses a controlled ShipEntity class.
$(OUT)/OOGFOSShipEntityBackend-objc.o: src/Core/OOGFOSShipEntityBackend.m src/Core/OOGFOSShipEntitySelectors.h $(UPSTREAM)/src/Core/OOTypes.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul -DGNUSTEP_BASE_LIBRARY=1 \
	-I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(UPSTREAM)/src/Core/Tables \
	-I $(UPSTREAM)/src/SDL -I src/Core -c $< -o $@
$(OUT)/oolite-objc-ship-mock.o: tests/oolite_objc_ship_mock.m src/Core/OOGFOSShipEntitySelectors.h $(UPSTREAM)/src/Core/OOTypes.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul -DGNUSTEP_BASE_LIBRARY=1 \
	-I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(UPSTREAM)/src/Core/Tables \
	-I $(UPSTREAM)/src/SDL -I src/Core -c $< -o $@
$(OUT)/oolite-js-objc-bridge-68040: tests/oolite_js_shipentity_objc_gate.c src/Core/OOGFOSShipEntityBackend.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipTokens.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-ship-mock.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_shipentity_objc_gate.c -o $(OUT)/oolite-js-objc-gate.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSShipEntityBackend.c -o $(OUT)/oolite-objc-game-shim.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSGameDispatch.c -o $(OUT)/oolite-objc-dispatch.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSShipTokens.c -o $(OUT)/oolite-objc-tokens.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSShipProperties.c -o $(OUT)/oolite-objc-properties.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSJavaScript.c -o $(OUT)/oolite-objc-jslib.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-js-objc-gate.o $(OUT)/oolite-objc-ship-mock.o $(OUT)/oolite-objc-game-shim.o $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-properties.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-objc-bridge
js-objc-bridge: $(OUT)/oolite-js-objc-bridge-68040

# Oolite ObjC business-rule qualification: clamp bounty/home, reject maxEscorts>16.
$(OUT)/oolite-js-objc-rules-68040: tests/oolite_js_shipentity_objc_rules_gate.c $(OUT)/oolite-js-objc-bridge-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_shipentity_objc_rules_gate.c -o $(OUT)/oolite-js-objc-rules-gate.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-js-objc-rules-gate.o $(OUT)/oolite-objc-ship-mock.o $(OUT)/oolite-objc-game-shim.o $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-properties.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-objc-rules
js-objc-rules: $(OUT)/oolite-js-objc-rules-68040

# Native GFOS shared-JS/Exec/ObjC guard test: player escorts, group floor,
# oversized 64-bit bounty rejected rather than silently narrowing to LONG.
$(OUT)/oolite-js-objc-guards-68040: tests/oolite_js_shipentity_objc_guard_gate.c tests/oolite_objc_ship_mock.m $(OUT)/oolite-js-objc-bridge-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_shipentity_objc_guard_gate.c -o $(OUT)/oolite-js-objc-guards-gate.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-js-objc-guards-gate.o $(OUT)/oolite-objc-ship-mock.o $(OUT)/oolite-objc-game-shim.o $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-properties.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-objc-guards
js-objc-guards: $(OUT)/oolite-js-objc-guards-68040

# Native ObjC registry strong-reference lifecycle: release caller owner,
# exercise GFOS JS over game Exec IPC, free only on token unregister.
$(OUT)/oolite-js-ownership-68040: tests/oolite_js_shipentity_ownership_gate.c tests/oolite_objc_ship_mock.m src/Core/OOGFOSShipEntityBackend.c src/Core/OOGFOSShipEntityBackend.m $(OUT)/oolite-js-objc-bridge-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_shipentity_ownership_gate.c -o $(OUT)/oolite-js-ownership-gate.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-js-ownership-gate.o $(OUT)/oolite-objc-ship-mock.o $(OUT)/oolite-objc-game-shim.o $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-properties.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-ownership
js-ownership: $(OUT)/oolite-js-ownership-68040

# Unmodified upstream Oolite default ship script -> GFOS JavaScript event host
# -> Exec game task -> native Objective-C receiver (controlled test fixture).
# Stage pinned upstream script at SYS:Oolite/Scripts/oolite-default-ship-script.js.
$(OUT)/oolite-original-objc-event-68040: tests/oolite_js_original_ship_event_objc_gate.c src/Core/OOGFOSShipEvents.c src/Core/OOGFOSShipEvents.h src/Core/OOGFOSGameDispatch.c $(OUT)/oolite-js-objc-bridge-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_original_ship_event_objc_gate.c -o $(OUT)/oolite-original-objc-event-gate.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSShipEvents.c -o $(OUT)/oolite-ship-events.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-original-objc-event-gate.o $(OUT)/oolite-ship-events.o $(OUT)/oolite-objc-ship-mock.o $(OUT)/oolite-objc-game-shim.o $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-properties.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-original-objc-event
js-original-objc-event: $(OUT)/oolite-original-objc-event-68040

# Two independent original default-ship JS contexts dispatch to different ObjC
# ships through a single game-owner Exec port; verifies zero action cross-talk.
$(OUT)/oolite-two-original-objc-68040: tests/oolite_js_two_original_ship_objc_gate.c $(OUT)/oolite-original-objc-event-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_two_original_ship_objc_gate.c -o $(OUT)/oolite-two-original-gate.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-two-original-gate.o $(OUT)/oolite-ship-events.o $(OUT)/oolite-objc-ship-mock.o $(OUT)/oolite-objc-game-shim.o $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-properties.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-two-original-objc
js-two-original-objc: $(OUT)/oolite-two-original-objc-68040

# Production-shaped per-ship JS objects on ONE shared JS task: native object
# creation, unmodified upstream source load, JS_Call event dispatch, optional
# one-shot events, distinct game-owner Objective-C ships.
$(OUT)/oolite-script-objects-68040: tests/oolite_js_script_objects_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSScriptObjects.h $(OUT)/oolite-original-objc-event-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_script_objects_gate.c -o $(OUT)/oolite-script-objects-gate.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSScriptObjects.c -o $(OUT)/oolite-script-objects-core.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-script-objects-gate.o $(OUT)/oolite-script-objects-core.o $(OUT)/oolite-ship-events.o $(OUT)/oolite-objc-ship-mock.o $(OUT)/oolite-objc-game-shim.o $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-properties.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-script-objects
js-script-objects: $(OUT)/oolite-script-objects-68040

# Remove just one script's shared-JSTask global object, invalidate its game
# token, reject stale native access, and keep another ship's original script
# events running on the same JS task.
$(OUT)/oolite-script-removal-68040: tests/oolite_js_script_removal_gate.c src/Core/OOGFOSScriptObjects.c $(OUT)/oolite-script-objects-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_script_removal_gate.c -o $(OUT)/oolite-script-removal-gate.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-script-removal-gate.o $(OUT)/oolite-script-objects-core.o $(OUT)/oolite-ship-events.o $(OUT)/oolite-objc-ship-mock.o $(OUT)/oolite-objc-game-shim.o $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-properties.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-script-removal
js-script-removal: $(OUT)/oolite-script-removal-68040

# Queued requests MUST NOT reach owner after Seal, even when using Serve not CancelQueued.
$(OUT)/oolite-js-seal-queued-68040: tests/oolite_js_game_dispatch_seal_queued_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSGameDispatch.h src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_seal_queued_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-seal-queued
js-seal-queued: $(OUT)/oolite-js-seal-queued-68040

# Frame-loop-safe nonblocking GFOS JS -> game owner Exec message queue pump.
$(OUT)/oolite-js-game-poll-68040: tests/oolite_js_game_dispatch_poll_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSGameDispatch.h src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_poll_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-game-poll
js-game-poll: $(OUT)/oolite-js-game-poll-68040

# Frame-loop-safe bounded GFOS JS game owner message pump.
$(OUT)/oolite-js-frame-budget-68040: tests/oolite_js_game_dispatch_frame_budget_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSGameDispatch.h src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_dispatch_frame_budget_gate.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-frame-budget
js-frame-budget: $(OUT)/oolite-js-frame-budget-68040

# Optional, game-task-owned frame hook for pinned original Universe.m update.
$(OUT)/oolite-js-game-frame-hook-68040: tests/oolite_js_game_frame_hook_gate.c src/Core/OOGFOSGameFrame.c src/Core/OOGFOSGameFrame.h src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_game_frame_hook_gate.c src/Core/OOGFOSGameFrame.c src/Core/OOGFOSGameDispatch.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-game-frame-hook
js-game-frame-hook: $(OUT)/oolite-js-game-frame-hook-68040

# Compile-only true Objective-C/Foundation + GFOS frame-header ABI gate.
# No Exec BOOL typedef may leak into Universe.m; ObjC BOOL is 8-bit.
$(OUT)/oolite-universe-frame-header-68040.o: tests/oolite_universe_frame_header_gate.m src/Core/OOGFOSGameFrame.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -std=gnu11 -Wall -Wextra -Werror -Wno-expansion-to-defined -O2 -m68040 -m68881 -noixemul -I $(GFOS_BASE_DIR)/Headers -I src/Core -c $< -o $@
.PHONY: universe-frame-objc-abi
universe-frame-objc-abi: $(OUT)/oolite-universe-frame-header-68040.o

# Pinned unmodified upstream Oolite Universe.m + opt-in GFOS frame pump.
# The upstream checkout stays untouched. All includes are REAL pinned target
# libraries, never fabricated SDL/OpenAL or JSAPI headers.
GFOS_SDL3_HEADERS ?= ../gfopenrtg/build/include
GFOS_OPENAL_HEADERS ?= ../gfopenrtg/library/modules/openal/include
GFOS_JS185_HEADERS ?= ../mozjs/js/src/build/dist/include
UNIVERSE_PATCH := patches/oolite/0001-gfos-universe-frame-pump.patch
UNIVERSE_WORK := $(OUT)/universe-gfos-patched
$(UNIVERSE_WORK)/src/Core/Universe.m: $(UPSTREAM)/src/Core/Universe.m $(UNIVERSE_PATCH)
	@mkdir -p $(UNIVERSE_WORK)/src/Core
	@test "$$(git -C $(UPSTREAM) rev-parse HEAD)" = 21978c37b2f11c8664adf3e2062efc1d0c875979 || { echo 'Universe upstream SHA mismatch; cannot apply pinned patch' >&2; exit 1; }
	@cp $(UPSTREAM)/src/Core/Universe.m $@
	@cd $(UNIVERSE_WORK) && patch -s -p1 < $(abspath $(UNIVERSE_PATCH))
$(OUT)/Universe-gfos-68040.o: $(UNIVERSE_WORK)/src/Core/Universe.m src/Core/OOGFOSGameFrame.h
	@mkdir -p $(OUT)
	@test -r $(GFOS_SDL3_HEADERS)/SDL3/SDL_video.h && test -r $(GFOS_OPENAL_HEADERS)/AL/al.h && test -r $(GFOS_JS185_HEADERS)/jsapi.h || { echo 'Need real GFOS SDL3/OpenAL and pinned SpiderMonkey 1.8.5 headers' >&2; exit 1; }
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -fobjc-exceptions -DNDEBUG -m68040 -m68881 -noixemul -DOOLITE_GFOS=1 -DOOLITE_BIG_ENDIAN=1 -DGL_GLEXT_PROTOTYPES=1 -DOOLITE_GFOS_GAME_DISPATCH=1 -DGNUSTEP_BASE_LIBRARY=1 -I src/Core -I $(GFOS_JS_INC) -I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(UPSTREAM)/src/Core/Tables -I $(UPSTREAM)/src/Core/Entities -I $(UPSTREAM)/src/Core/Materials -I $(UPSTREAM)/src/Core/Scripting -I $(UPSTREAM)/src/Core/Debug -I $(UPSTREAM)/src/SDL -I $(PREFIX)/m68k-amigaos/ndk-include.vendor -I $(PREFIX)/m68k-amigaos/include -I $(PREFIX)/m68k-amigaos/port-deps/include -I $(GFOS_JS185_HEADERS) -I $(GFOS_SDL3_HEADERS) -I $(GFOS_OPENAL_HEADERS) -c $< -o $@
.PHONY: universe-gfos-object
universe-gfos-object: $(OUT)/Universe-gfos-68040.o

# Four real original Oolite engine application objects (NOT a full link).
# Uses pinned GFOpenGPU SDL3/OpenAL headers and target SpiderMonkey 1.8.5.
GFOS_OOLITE_ID ?= 1.93.1-255
GFOS_OOLITE_BUILDER ?= AmiCradle Team
GFOS_OOLITE_DATE ?= 2026-11-18
GFOS_OOLITE_OBJFLAGS := -x objective-c -std=gnu11 -O2 -fobjc-exceptions -DNDEBUG -m68040 -m68881 -noixemul -DOOLITE_GFOS=1 -DOOLITE_BIG_ENDIAN=1 -DGL_GLEXT_PROTOTYPES=1 -DGNUSTEP_BASE_LIBRARY=1 -DOO_VERSION_FULL='"$(GFOS_OOLITE_ID)"' -DOO_BUILDER='"$(GFOS_OOLITE_BUILDER)"' -DOO_BUILD_DATE='"$(GFOS_OOLITE_DATE)"'
GFOS_OOLITE_INCLUDES := -I src/Core -I $(GFOS_JS_INC) -I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(UPSTREAM)/src/Core/Tables -I $(UPSTREAM)/src/Core/Entities -I $(UPSTREAM)/src/Core/Materials -I $(UPSTREAM)/src/Core/Scripting -I $(UPSTREAM)/src/Core/Debug -I $(UPSTREAM)/src/Core/OXPVerifier -I $(UPSTREAM)/src/SDL -I $(PREFIX)/m68k-amigaos/ndk-include.vendor -I $(PREFIX)/m68k-amigaos/include -I $(PREFIX)/m68k-amigaos/port-deps/include -I $(GFOS_JS185_HEADERS) -I $(GFOS_SDL3_HEADERS) -I $(GFOS_OPENAL_HEADERS)
$(OUT)/MyOpenGLView-gfos-68040.o: $(UPSTREAM)/src/SDL/MyOpenGLView.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -c $< -o $@
$(OUT)/GameController-gfos-68040.o: $(UPSTREAM)/src/Core/GameController.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -c $< -o $@
$(OUT)/oolite-main-gfos-68040.o: $(UPSTREAM)/src/SDL/main.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -c $< -o $@
.PHONY: oolite-core-four-objects
oolite-core-four-objects: $(OUT)/Universe-gfos-68040.o $(OUT)/MyOpenGLView-gfos-68040.o $(OUT)/GameController-gfos-68040.o $(OUT)/oolite-main-gfos-68040.o

# Three additional genuine Oolite core modules, with MiniZip from the
# already-pinned Oolite upstream rather than synthetic ZIP headers.
$(OUT)/ResourceManager-gfos-68040.o: $(UPSTREAM)/src/Core/ResourceManager.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(UPSTREAM)/src/Core/MiniZip -c $< -o $@
$(OUT)/OOShipRegistry-gfos-68040.o: $(UPSTREAM)/src/Core/OOShipRegistry.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -c $< -o $@
$(OUT)/OOSoundSource-gfos-68040.o: $(UPSTREAM)/src/Core/OOSoundSource.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -c $< -o $@
.PHONY: oolite-core-seven-objects
oolite-core-seven-objects: oolite-core-four-objects $(OUT)/ResourceManager-gfos-68040.o $(OUT)/OOShipRegistry-gfos-68040.o $(OUT)/OOSoundSource-gfos-68040.o

# GFOS Oolite native AmigaOS system reporting; do not compile x86 CPUID on 68k.
OO_GFOS_OS_PATCH := patches/oolite/0002-gfos-native-cpu-memory-report.patch
OO_GFOS_OS_DIR := $(OUT)/gfos-os-patched
$(OO_GFOS_OS_DIR)/.ready: $(OO_GFOS_OS_PATCH) $(UPSTREAM)/src/Core/OOCPUInfo.m $(UPSTREAM)/src/Core/OOCPUInfo.h $(UPSTREAM)/src/Core/OOLogHeader.m
	@mkdir -p $(OO_GFOS_OS_DIR)/src/Core
	@test "$$(git -C $(UPSTREAM) rev-parse HEAD)" = 21978c37b2f11c8664adf3e2062efc1d0c875979 || { echo 'Oolite OS-patch upstream mismatch' >&2; exit 1; }
	@cp $(UPSTREAM)/src/Core/OOCPUInfo.m $(UPSTREAM)/src/Core/OOCPUInfo.h $(UPSTREAM)/src/Core/OOLogHeader.m $(OO_GFOS_OS_DIR)/src/Core/
	@cd $(OO_GFOS_OS_DIR) && patch -s -p1 < $(abspath $(OO_GFOS_OS_PATCH))
	@touch $@
$(OUT)/OOGFOSSystemInfo-68040.o: src/Core/OOGFOSSystemInfo.c src/Core/OOGFOSSystemInfo.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c $< -o $@
$(OUT)/OOCPUInfo-gfos-68040.o: $(OO_GFOS_OS_DIR)/.ready src/Core/OOGFOSSystemInfo.h
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(OO_GFOS_OS_DIR)/src/Core -c $(OO_GFOS_OS_DIR)/src/Core/OOCPUInfo.m -o $@
$(OUT)/OOLogHeader-gfos-68040.o: $(OO_GFOS_OS_DIR)/.ready
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(OO_GFOS_OS_DIR)/src/Core -c $(OO_GFOS_OS_DIR)/src/Core/OOLogHeader.m -o $@
$(OUT)/oolite-gfos-system-info-68040: tests/oolite_gfos_system_info_gate.c src/Core/OOGFOSSystemInfo.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) $^ -o $@
.PHONY: oolite-core-nine-objects oolite-system-info-gate
oolite-core-nine-objects: oolite-core-seven-objects $(OUT)/OOCPUInfo-gfos-68040.o $(OUT)/OOLogHeader-gfos-68040.o $(OUT)/OOGFOSSystemInfo-68040.o
oolite-system-info-gate: $(OUT)/oolite-gfos-system-info-68040

# Oolite original OOOpenGL error reporting with real SGI GLU on GFOpenGPU.
GFOS_GLU_KIT ?= ../gfopenrtg/build/glu
OO_GFOS_GLU_PATCH := patches/oolite/0003-gfos-native-glu-header.patch
OO_GFOS_GLU_DIR := $(OUT)/gfos-glu-patched
$(OO_GFOS_GLU_DIR)/src/Core/OOOpenGL.m: $(UPSTREAM)/src/Core/OOOpenGL.m $(OO_GFOS_GLU_PATCH)
	@mkdir -p $(OO_GFOS_GLU_DIR)/src/Core
	@test "$$(git -C $(UPSTREAM) rev-parse HEAD)" = 21978c37b2f11c8664adf3e2062efc1d0c875979 || { echo 'Oolite GLU upstream mismatch' >&2; exit 1; }
	@cp $(UPSTREAM)/src/Core/OOOpenGL.m $@
	@cd $(OO_GFOS_GLU_DIR) && patch -s -p1 < $(abspath $(OO_GFOS_GLU_PATCH))
$(OUT)/OOOpenGL-gfos-68040.o: $(OO_GFOS_GLU_DIR)/src/Core/OOOpenGL.m
	@test -r $(GFOS_GLU_KIT)/include/GL/glu.h && test -r $(GFOS_GLU_KIT)/lib/libGLU.a || { echo 'Need genuine GFOpenGPU GLU kit' >&2; exit 1; }
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
$(OUT)/oolite-gfos-glu-68040: tests/oolite_glu_error_gate.c
	@test -r $(GFOS_GLU_KIT)/lib/libGLU.a || { echo 'Missing GFOpenGPU GLU kit' >&2; exit 1; }
	@mkdir -p $(OUT)
	$(CROSS_CC) -std=gnu11 -O2 -Wall -Wextra -Werror -m68040 -m68881 -noixemul -I $(GFOS_GLU_KIT)/include $< -L $(GFOS_GLU_KIT)/lib -lGLU -L $(PREFIX)/m68k-amigaos/lib -lGL -lm -o $@
.PHONY: oolite-core-ten-objects oolite-glu-native-gate
oolite-core-ten-objects: oolite-core-nine-objects $(OUT)/OOOpenGL-gfos-68040.o
oolite-glu-native-gate: $(OUT)/oolite-gfos-glu-68040

# First truly visible Oolite-owned native 68040 window from original assets.
# Separate splash preview, not a claim that the engine is linked.
$(OUT)/oolite-native-splash-68040: tests/oolite_native_splash_view.c
	@mkdir -p $(OUT)
	@test -r $(GFOS_SDL3_HEADERS)/SDL3/SDL.h && test -r $(GFOS_SDL3_LIB_DIR)/libSDL3.a || { echo 'Need actual GFOpenGPU SDL3 kit header and stub library' >&2; exit 1; }
	$(CROSS_CC) -std=gnu11 -O2 -Wall -Wextra -Werror -m68040 -m68881 -noixemul -I $(GFOS_SDL3_HEADERS) $< -L $(GFOS_SDL3_LIB_DIR) -lSDL3 -lm -lamiga -o $@
GFOS_SDL3_LIB_DIR ?= ../gfopenrtg/build
.PHONY: oolite-native-splash
oolite-native-splash: $(OUT)/oolite-native-splash-68040

# Original Oolite ship, player, GUI, GL-context and SDL input objects.
# This is a native object compilation gate, NOT a playable game link.
GFOS_VISUAL_SOURCES := ShipEntity PlayerEntity GuiDisplayGen OOOpenGLExtensionManager OOOpenGLStateManager MyOpenGLView+Input GameController+SDLFullScreen
GFOS_VISUAL_OBJECTS := $(addprefix $(OUT)/,$(addsuffix -gfos-68040.o,$(GFOS_VISUAL_SOURCES)))
$(OUT)/ShipEntity-gfos-68040.o: $(UPSTREAM)/src/Core/Entities/ShipEntity.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
$(OUT)/PlayerEntity-gfos-68040.o: $(UPSTREAM)/src/Core/Entities/PlayerEntity.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
$(OUT)/GuiDisplayGen-gfos-68040.o: $(UPSTREAM)/src/Core/GuiDisplayGen.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
$(OUT)/OOOpenGLExtensionManager-gfos-68040.o: $(UPSTREAM)/src/Core/OOOpenGLExtensionManager.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
$(OUT)/OOOpenGLStateManager-gfos-68040.o: $(UPSTREAM)/src/Core/OOOpenGLStateManager.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
$(OUT)/MyOpenGLView+Input-gfos-68040.o: $(UPSTREAM)/src/SDL/MyOpenGLView+Input.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
$(OUT)/GameController+SDLFullScreen-gfos-68040.o: $(UPSTREAM)/src/SDL/GameController+SDLFullScreen.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
.PHONY: oolite-core-seventeen-objects
oolite-core-seventeen-objects: oolite-core-ten-objects $(GFOS_VISUAL_OBJECTS)

# Additional actual Oolite HUD/entity/material/audio/controller code.
GFOS_VISUAL_MORE_CORE := HeadUpDisplay OOOpenGLMatrixManager OOOpenALController OOMusicController OOCacheManager
GFOS_VISUAL_MORE_ENTITIES := Entity OOEntityWithDrawable StationEntity
GFOS_VISUAL_MORE_MATERIALS := OOMaterial OOTexture OOShaderProgram
GFOS_VISUAL_MORE_SDL := OOSDLJoystickManager
GFOS_VISUAL_MORE_OBJECTS := $(addprefix $(OUT)/,$(addsuffix -gfos-68040.o,$(GFOS_VISUAL_MORE_CORE) $(GFOS_VISUAL_MORE_ENTITIES) $(GFOS_VISUAL_MORE_MATERIALS) $(GFOS_VISUAL_MORE_SDL)))
$(addprefix $(OUT)/,$(addsuffix -gfos-68040.o,$(GFOS_VISUAL_MORE_CORE))): $(OUT)/%-gfos-68040.o: $(UPSTREAM)/src/Core/%.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
$(addprefix $(OUT)/,$(addsuffix -gfos-68040.o,$(GFOS_VISUAL_MORE_ENTITIES))): $(OUT)/%-gfos-68040.o: $(UPSTREAM)/src/Core/Entities/%.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
$(addprefix $(OUT)/,$(addsuffix -gfos-68040.o,$(GFOS_VISUAL_MORE_MATERIALS))): $(OUT)/%-gfos-68040.o: $(UPSTREAM)/src/Core/Materials/%.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
$(addprefix $(OUT)/,$(addsuffix -gfos-68040.o,$(GFOS_VISUAL_MORE_SDL))): $(OUT)/%-gfos-68040.o: $(UPSTREAM)/src/SDL/%.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
.PHONY: oolite-core-twentynine-objects
oolite-core-twentynine-objects: oolite-core-seventeen-objects $(GFOS_VISUAL_MORE_OBJECTS)

# Original Oolite SpiderMonkey-facing script object compilation (not runtime
# compatibility with GFOS javascript.library; that remains a separate gate).
GFOS_OOLITE_SCRIPT_SET := OOJSGlobal OOJSOolite OOJSShip OOJSPlayerShip OOJSPlayer OOJSScript OOJSFrameCallbacks OOJSEntity OOJSSystem OOJSMission OOJSMissionVariables OOJSCall
GFOS_OOLITE_SCRIPT_OBJECTS := $(addprefix $(OUT)/,$(addsuffix -gfos-68040.o,$(GFOS_OOLITE_SCRIPT_SET)))
$(GFOS_OOLITE_SCRIPT_OBJECTS): $(OUT)/%-gfos-68040.o: $(UPSTREAM)/src/Core/Scripting/%.m
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_OOLITE_OBJFLAGS) $(GFOS_OOLITE_INCLUDES) -I $(GFOS_GLU_KIT)/include -c $< -o $@
.PHONY: oolite-core-fortyone-objects
oolite-core-fortyone-objects: oolite-core-twentynine-objects $(GFOS_OOLITE_SCRIPT_OBJECTS)

# Regression: closing one Oolite script owner must not clear the shared
# javascript.library base while another owner is still alive. Guest runtime
# requires the canonical library and JavaScriptEngine to be installed.
$(OUT)/oolite-jslib-two-owners-68040: tests/oolite_jslib_two_owners_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h $(GFOS_JS_INC)/libraries/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_jslib_two_owners_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: jslib-two-owners
jslib-two-owners: $(OUT)/oolite-jslib-two-owners-68040

# Source-compatible value conversion seam; this uses real GFOS JSValue handles
# rather than pretending SpiderMonkey jsval has the same ABI.
$(OUT)/oolite-js-values-68040: tests/oolite_js_values_gate.c src/Core/OOGFOSJSValues.c src/Core/OOGFOSJSValues.h src/Core/OOGFOSJavaScript.c $(GFOS_JS_INC)/libraries/javascript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_values_gate.c src/Core/OOGFOSJSValues.c src/Core/OOGFOSJavaScript.c -lm -o $@
.PHONY: js-values
js-values: $(OUT)/oolite-js-values-68040

# Native AC090 property/index/function seam qualification against the canonical
# shared JavaScriptEngine; no embedded SpiderMonkey or fake JSAPI symbols.
$(OUT)/oolite-js-properties-68040: tests/oolite_js_properties_gate.c src/Core/OOGFOSJSProperties.c src/Core/OOGFOSJSProperties.h src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_properties_gate.c src/Core/OOGFOSJSProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-properties
js-properties: $(OUT)/oolite-js-properties-68040

# Transactional Oolite JS task recreation on the canonical shared engine.
$(OUT)/oolite-js-reset-68040: tests/oolite_js_reset_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_reset_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-reset
js-reset: $(OUT)/oolite-js-reset-68040

$(OUT)/oolite-js-host-reset-68040: tests/oolite_js_host_reset_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_host_reset_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-host-reset
js-host-reset: $(OUT)/oolite-js-host-reset-68040

# Exact ECMA-262 ToInt32 numeric coercion (not JS_ToLong alias).
$(OUT)/oolite-js-int32-68040: tests/oolite_js_int32_gate.c src/Core/OOGFOSJSValues.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_int32_gate.c src/Core/OOGFOSJSValues.c src/Core/OOGFOSJavaScript.c -lm -o $@
.PHONY: js-int32
js-int32: $(OUT)/oolite-js-int32-68040

$(OUT)/oolite-js-errors-68040: tests/oolite_js_errors_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_errors_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-errors
js-errors: $(OUT)/oolite-js-errors-68040

$(OUT)/oolite-js-retain-68040: tests/oolite_js_retain_gate.c src/Core/OOGFOSJSProperties.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_retain_gate.c src/Core/OOGFOSJSProperties.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-retain
js-retain: $(OUT)/oolite-js-retain-68040

$(OUT)/oolite-js-inherit-68040: tests/oolite_js_inherit_gate.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_inherit_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-inherit
js-inherit: $(OUT)/oolite-js-inherit-68040

# Oolite OOJavaScriptEngine context reset: candidate initialization, rollback
# on prepare failure, notifications, and old wrapper teardown order.
$(OUT)/oolite-js-prepared-reset-68040: tests/oolite_js_prepared_reset_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_prepared_reset_gate.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-prepared-reset
js-prepared-reset: $(OUT)/oolite-js-prepared-reset-68040

# Original OOJSScript UTF-16 source -> native GFOS JavaScript UTF-8.
$(OUT)/oolite-js-utf16-68040: tests/oolite_js_utf16_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSScriptObjects.h src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_utf16_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-utf16
js-utf16: $(OUT)/oolite-js-utf16-68040

# Original UTF-8 Oolite script (converted to UTF-16BE on media to reproduce
# OOJSScript's unichar input) -> per-ship JS object -> Exec -> ObjC selectors.
# Guest data: SYS:Oolite/Scripts/oolite-default-ship-script.utf16be.
$(OUT)/oolite-original-utf16-objc-68040: tests/oolite_js_original_utf16_objc_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSScriptObjects.h $(OUT)/oolite-original-objc-event-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_original_utf16_objc_gate.c -o $(OUT)/oolite-original-utf16-objc-gate.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSScriptObjects.c -o $(OUT)/oolite-original-utf16-script-objects.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-original-utf16-objc-gate.o $(OUT)/oolite-original-utf16-script-objects.o $(OUT)/oolite-ship-events.o $(OUT)/oolite-objc-ship-mock.o $(OUT)/oolite-objc-game-shim.o $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-properties.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-original-utf16-objc
js-original-utf16-objc: $(OUT)/oolite-original-utf16-objc-68040

# Two original UTF-16 Oolite ship scripts, distinct ship owners, single
# canonical JS task, routed game-task ObjC callbacks without cross-talk.
$(OUT)/oolite-two-original-utf16-objc-68040: tests/oolite_js_two_original_utf16_objc_gate.c src/Core/OOGFOSScriptObjects.c $(OUT)/oolite-original-utf16-objc-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_two_original_utf16_objc_gate.c -o $(OUT)/oolite-two-original-utf16-gate.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-two-original-utf16-gate.o $(OUT)/oolite-original-utf16-script-objects.o $(OUT)/oolite-ship-events.o $(OUT)/oolite-objc-ship-mock.o $(OUT)/oolite-objc-game-shim.o $(OUT)/OOGFOSShipEntityBackend-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-properties.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-two-original-utf16-objc
js-two-original-utf16-objc: $(OUT)/oolite-two-original-utf16-objc-68040

# Pinned original OOJSEntity float energy/maxEnergy selectors through Exec,
# carried as IEEE-754 binary32 bits inside the 32-bit routed scalar message.
$(OUT)/oolite-js-entity-energy-68040: tests/oolite_js_entity_energy_gate.c src/Core/OOGFOSShipProperties.c src/Core/OOGFOSShipEntityBackend.c src/Core/OOGFOSShipEntityBackend.m tests/oolite_objc_ship_mock.m $(OUT)/oolite-js-objc-bridge-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_entity_energy_gate.c -o $(OUT)/oolite-energy-gate.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSShipProperties.c -o $(OUT)/oolite-energy-properties.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSShipEntityBackend.c -o $(OUT)/oolite-energy-game-backend.o
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul -DGNUSTEP_BASE_LIBRARY=1 -I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(UPSTREAM)/src/Core/Tables -I $(UPSTREAM)/src/SDL -I src/Core -c src/Core/OOGFOSShipEntityBackend.m -o $(OUT)/oolite-energy-backend-objc.o
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul -DGNUSTEP_BASE_LIBRARY=1 -I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(UPSTREAM)/src/Core/Tables -I $(UPSTREAM)/src/SDL -I src/Core -c tests/oolite_objc_ship_mock.m -o $(OUT)/oolite-energy-mock-objc.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-energy-gate.o $(OUT)/oolite-energy-properties.o $(OUT)/oolite-energy-game-backend.o $(OUT)/oolite-energy-backend-objc.o $(OUT)/oolite-energy-mock-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a \
	-lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-entity-energy
js-entity-energy: $(OUT)/oolite-js-entity-energy-68040

# Original OOJSEntity Boolean type flags, including STATUS_DEAD validity.
$(OUT)/oolite-js-entity-flags-68040: tests/oolite_js_entity_flags_gate.c $(OUT)/oolite-js-entity-energy-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_entity_flags_gate.c -o $(OUT)/oolite-entity-flags-gate.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-entity-flags-gate.o $(OUT)/oolite-energy-properties.o $(OUT)/oolite-energy-game-backend.o $(OUT)/oolite-energy-backend-objc.o $(OUT)/oolite-energy-mock-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-entity-flags
js-entity-flags: $(OUT)/oolite-js-entity-flags-68040

# Production-facing OOJavaScriptEngine callJSFunction scheduling: synchronous
# JS task execution without deadlocking the Oolite game-owner Exec RPC port.
$(OUT)/oolite-js-sync-call-68040: tests/oolite_js_sync_call_gate.c src/Core/OOGFOSScriptInvoke.c src/Core/OOGFOSScriptInvoke.h $(OUT)/oolite-js-entity-flags-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_sync_call_gate.c -o $(OUT)/oolite-sync-call-gate.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSScriptInvoke.c -o $(OUT)/oolite-sync-invoke.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-sync-call-gate.o $(OUT)/oolite-sync-invoke.o $(OUT)/oolite-energy-properties.o $(OUT)/oolite-energy-game-backend.o $(OUT)/oolite-energy-backend-objc.o $(OUT)/oolite-energy-mock-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-sync-call
js-sync-call: $(OUT)/oolite-js-sync-call-68040

# Original Oolite default ship script: per-ship UTF-16 event invocations with
# synchronous game-task Exec pump and guaranteed completion on errors.
$(OUT)/oolite-js-event-invoke-68040: tests/oolite_js_event_invoke_gate.c src/Core/OOGFOSScriptEventInvoke.c src/Core/OOGFOSScriptInvoke.c src/Core/OOGFOSScriptObjects.c $(OUT)/oolite-js-sync-call-68040 $(OUT)/oolite-original-objc-event-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_event_invoke_gate.c -o $(OUT)/oolite-event-invoke-gate.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSScriptEventInvoke.c -o $(OUT)/oolite-event-invoke.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSScriptInvoke.c -o $(OUT)/oolite-sync-invoke.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSScriptObjects.c -o $(OUT)/oolite-original-utf16-script-objects.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-event-invoke-gate.o $(OUT)/oolite-event-invoke.o $(OUT)/oolite-sync-invoke.o $(OUT)/oolite-original-utf16-script-objects.o $(OUT)/oolite-ship-events.o $(OUT)/oolite-energy-properties.o $(OUT)/oolite-energy-game-backend.o $(OUT)/oolite-energy-backend-objc.o $(OUT)/oolite-energy-mock-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-event-invoke
js-event-invoke: $(OUT)/oolite-js-event-invoke-68040

# Source-level replacement for original OOJavaScriptEngine callJSFunction:
# Oolite game task calls JS function with real receiver/argv and retains the
# arbitrarily typed result as an owned JSTask-scoped JSValue.
$(OUT)/oolite-js-call-function-68040: tests/oolite_js_call_function_gate.c src/Core/OOGFOSScriptInvoke.c src/Core/OOGFOSScriptInvoke.h $(OUT)/oolite-js-sync-call-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_call_function_gate.c -o $(OUT)/oolite-call-function-gate.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSScriptInvoke.c -o $(OUT)/oolite-sync-invoke.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-call-function-gate.o $(OUT)/oolite-sync-invoke.o $(OUT)/oolite-energy-properties.o $(OUT)/oolite-energy-game-backend.o $(OUT)/oolite-energy-backend-objc.o $(OUT)/oolite-energy-mock-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-call-function
js-call-function: $(OUT)/oolite-js-call-function-68040

# Unmodified upstream oolite-global-prefix.js: canonical JSHostClass Ship
# prototype exposed as an unconstructable named JS constructor, so the
# original script installs Ship.spawnOne() without a SpiderMonkey ABI.
# Guest needs SYS:Oolite/Scripts/oolite-global-prefix.js (pinned upstream).
$(OUT)/oolite-js-global-prefix-68040: tests/oolite_js_global_prefix_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSJavaScript.h src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSWorldScripts.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_global_prefix_gate.c src/Core/OOGFOSJavaScript.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSWorldScripts.c -o $@
.PHONY: js-global-prefix
js-global-prefix: $(OUT)/oolite-js-global-prefix-68040

# Optional GFOS JS ABI v2 feature test: the real registered host-class
# prototype MUST be available before a game-owned ShipEntity exists.
# Use GFOS_JS_V2_INC=path/to/version-2/Include (e.g. the gfopensdk v2 branch).
# A v2 library paired with old v1 engine prints REQUIRES_V2_ENGINE, exit 77.
GFOS_JS_V2_INC ?= $(GFOS_JS_INC)
$(OUT)/oolite-js-class-bootstrap-v2-68040: tests/oolite_js_class_bootstrap_v2_gate.c src/Core/OOGFOSJSClassBootstrap.c src/Core/OOGFOSJSClassBootstrap.h src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) -I $(GFOS_JS_V2_INC) $(GFOS_JS_FLAGS) tests/oolite_js_class_bootstrap_v2_gate.c src/Core/OOGFOSJSClassBootstrap.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-class-bootstrap-v2
js-class-bootstrap-v2: $(OUT)/oolite-js-class-bootstrap-v2-68040

# ABI v2 old-engine safety: library vector -210 checks older task table's
# jt_Size before dispatching; use GFOS_JS_V2_INC generated SDK headers.
$(OUT)/oolite-js-abi-v2-fallback-68040: tests/oolite_js_abi_v2_fallback_gate.c
	@mkdir -p $(OUT)
	$(CROSS_CC) -I $(GFOS_JS_V2_INC) $(GFOS_JS_FLAGS) tests/oolite_js_abi_v2_fallback_gate.c -o $@
.PHONY: js-abi-v2-fallback
js-abi-v2-fallback: $(OUT)/oolite-js-abi-v2-fallback-68040

# Original OOJSScript -callMethod:result: arbitrary owned JSValue returns
# after synchronous game task / JS task dispatch, errors, and recovery.
$(OUT)/oolite-js-script-result-68040: tests/oolite_js_script_result_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSScriptEventInvoke.c $(OUT)/oolite-js-event-invoke-68040 $(OUT)/oolite-js-entity-flags-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_script_result_gate.c -o $(OUT)/oolite-script-result-gate.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSScriptObjects.c -o $(OUT)/oolite-script-result-objects.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSScriptEventInvoke.c -o $(OUT)/oolite-script-result-invoke.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-script-result-gate.o $(OUT)/oolite-script-result-objects.o $(OUT)/oolite-script-result-invoke.o $(OUT)/oolite-sync-invoke.o $(OUT)/oolite-energy-properties.o $(OUT)/oolite-energy-game-backend.o $(OUT)/oolite-energy-backend-objc.o $(OUT)/oolite-energy-mock-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-script-result
js-script-result: $(OUT)/oolite-js-script-result-68040

# OOJSScript instances inherit the genuine Script.prototype extensions
# from original oolite-global-prefix.js, while early scripts without a
# Script constructor retain normal Object.prototype fallback.
$(OUT)/oolite-js-script-prototype-68040: tests/oolite_js_script_prototype_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_script_prototype_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-script-prototype
js-script-prototype: $(OUT)/oolite-js-script-prototype-68040

# Original OOJSScript GC-root removal: script global lookup must be truly
# deleted, not left as enumerable undefined; retainers remain valid, and a
# new script with the same name can be created without stale identity.
$(OUT)/oolite-js-script-drop-68040: tests/oolite_js_script_drop_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_script_drop_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-script-drop
js-script-drop: $(OUT)/oolite-js-script-drop-68040

# Four pinned original OOJSEntity GLfloat getters on the real game task:
# collisionRadius, distanceTravelled, mass, spawnTime. Strictly reject
# writes to read-only JSHostClass fields.
$(OUT)/oolite-js-entity-scalars-68040: tests/oolite_js_entity_scalars_gate.c $(OUT)/oolite-js-entity-energy-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_entity_scalars_gate.c -o $(OUT)/oolite-entity-scalars-gate.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-entity-scalars-gate.o $(OUT)/oolite-energy-properties.o $(OUT)/oolite-energy-game-backend.o $(OUT)/oolite-energy-backend-objc.o $(OUT)/oolite-energy-mock-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-entity-scalars
js-entity-scalars: $(OUT)/oolite-js-entity-scalars-68040

# Original OOJSWorldScripts: a read-only enumerable global registry of
# per-script JS objects, including hyphenated/dotted world-script names.
$(OUT)/oolite-js-worldscripts-68040: tests/oolite_js_worldscripts_gate.c src/Core/OOGFOSWorldScripts.c src/Core/OOGFOSWorldScripts.h src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_worldscripts_gate.c src/Core/OOGFOSWorldScripts.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-worldscripts
js-worldscripts: $(OUT)/oolite-js-worldscripts-68040

# Entire unmodified upstream Oolite locale-functions.js including normal
# formatting calls, per-script this and real OOJSWorldScripts registration.
# Stage SYS:Oolite/Scripts/oolite-locale-functions.js from pinned upstream.
$(OUT)/oolite-js-original-locale-68040: tests/oolite_js_original_locale_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSWorldScripts.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_original_locale_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSWorldScripts.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-original-locale
js-original-locale: $(OUT)/oolite-js-original-locale-68040

# Oolite engine reset while JS candidate boot invokes real game-task ship
# selectors. Aborted prepare preserves the original script object graph.
$(OUT)/oolite-js-routed-reset-68040: tests/oolite_js_routed_reset_gate.c src/Core/OOGFOSScriptReset.c src/Core/OOGFOSScriptReset.h src/Core/OOGFOSScriptInvoke.c $(OUT)/oolite-js-sync-call-68040
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c tests/oolite_js_routed_reset_gate.c -o $(OUT)/oolite-routed-reset-gate.o
	$(CROSS_CC) $(GFOS_JS_FLAGS) -c src/Core/OOGFOSScriptReset.c -o $(OUT)/oolite-routed-reset.o
	$(CROSS_CC) -m68040 -m68881 -noixemul $(OUT)/oolite-routed-reset-gate.o $(OUT)/oolite-routed-reset.o $(OUT)/oolite-sync-invoke.o $(OUT)/oolite-energy-properties.o $(OUT)/oolite-energy-game-backend.o $(OUT)/oolite-energy-backend-objc.o $(OUT)/oolite-energy-mock-objc.o $(OUT)/oolite-objc-tokens.o $(OUT)/oolite-objc-dispatch.o $(OUT)/oolite-objc-jslib.o \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group -Wl,--whole-archive $(GFOS_BASE_ARCHIVE) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a $(GFOS_FFI_DIR)/lib/libffi.a -lobjc -lpthread -lm -lz -lsocket -Wl,--end-group -o $@
.PHONY: js-routed-reset
js-routed-reset: $(OUT)/oolite-js-routed-reset-68040

# Entire unchanged original Oolite Constrictor ship-library script mission
# gating: missionVariables.conhunt game-save state transitions plus exact
# return-value handling; stage the pinned original JS file on guest SYS.
$(OUT)/oolite-js-constrictor-mission-68040: tests/oolite_js_constrictor_mission_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSWorldScripts.c src/Core/OOGFOSJavaScript.c
	@mkdir -p $(OUT)
	$(CROSS_CC) $(GFOS_JS_FLAGS) tests/oolite_js_constrictor_mission_gate.c src/Core/OOGFOSScriptObjects.c src/Core/OOGFOSWorldScripts.c src/Core/OOGFOSJavaScript.c -o $@
.PHONY: js-constrictor-mission
js-constrictor-mission: $(OUT)/oolite-js-constrictor-mission-68040

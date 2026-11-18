# Full original Oolite 68040 link gate; run with make -f Makefile -f build/full-link.mk
# This never modifies the pinned upstream checkout or shared SDK.
FULL_OGG_ROOT ?= ../ogg-vorbis-stage
FULL_JS185_LIB ?= ../mozjs-build/libjs_static.a
FULL_SDL3_LIB ?= $(GFOS_SDL3_LIB_DIR)/libSDL3.a
FULL_OPENAL_LIB ?= ../gfopenrtg/build/openal/libopenal.a
FULL_GLU_LIB ?= $(GFOS_GLU_KIT)/lib/libGLU.a
FULL_OOLITE_OBJECTS := $(OUT)/oolite-complete-objects
FULL_OOLITE_EXE := $(OUT)/oolite-original-68040
FULL_LIST := build/oolite-meson-242.list
FULL_FFI_LIB ?= $(GFOS_FFI_DIR)/lib/libffi.a
FULL_NSMESSAGE_OBJ ?= $(GFOS_BASE_DIR)/Source/obj/libgnustep-base.obj/NSMessagePort.m.o
FULL_GNUSTEP_PATCH := patches/gnustep/0001-gfos-messageport-objc-bool.patch
FULL_EXTRACTOR := src/Core/OOOXZGFOSExtraction.m
FULL_PREFLIGHT := src/Core/OOOXZArchivePreflight.c
FULL_OGG_INC := $(FULL_OGG_ROOT)/include
FULL_OGG_LIB := $(FULL_OGG_ROOT)/lib
FULL_STARTUP_TRACE ?= 0
FULL_DEFINES := -DOOLITE_GFOS_STARTUP_TRACE=$(FULL_STARTUP_TRACE) -DOOLITE_GFOS=1 -DOOLITE_BIG_ENDIAN=1 -DGL_GLEXT_PROTOTYPES=1 -DGNUSTEP_BASE_LIBRARY=1 -DNDEBUG -DOO_VERSION_FULL='"1.93.1-255"' -DOO_BUILDER='"AmiCradle Team"' -DOO_BUILD_DATE='"2026-11-18"'
FULL_INCLUDES := -I src/Core -I $(GFOS_JS_INC) -I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core -I $(UPSTREAM)/src/Core/Tables -I $(UPSTREAM)/src/Core/Entities -I $(UPSTREAM)/src/Core/Materials -I $(UPSTREAM)/src/Core/Scripting -I $(UPSTREAM)/src/Core/Debug -I $(UPSTREAM)/src/Core/OXPVerifier -I $(UPSTREAM)/src/Core/MiniZip -I $(UPSTREAM)/src/SDL -I $(UPSTREAM)/src/SDL/EXRSnapshotSupport -I $(PREFIX)/m68k-amigaos/ndk-include.vendor -I $(PREFIX)/m68k-amigaos/include -I $(PREFIX)/m68k-amigaos/port-deps/include -I $(GFOS_JS185_HEADERS) -I $(GFOS_SDL3_HEADERS) -I $(GFOS_OPENAL_HEADERS) -I $(GFOS_GLU_KIT)/include -I $(FULL_OGG_INC)

# This step compiles the full original source list; no stand-in engine modules.
# Previously qualified Oolite patches, including SDL3 joystick lifecycle, are applied to private copies.
.PHONY: oolite-full-original-objects
oolite-full-original-objects:
	@test "$$(git -C $(UPSTREAM) rev-parse HEAD)" = 21978c37b2f11c8664adf3e2062efc1d0c875979 || { echo 'Pinned Oolite upstream mismatch' >&2; exit 1; }
	@test -r $(FULL_OGG_LIB)/libvorbisfile.a && test -r $(GFOS_JS185_HEADERS)/jsapi.h && test -r $(GFOS_SDL3_HEADERS)/SDL3/SDL_video.h && test -r $(GFOS_OPENAL_HEADERS)/AL/al.h && test -r $(GFOS_GLU_KIT)/include/GL/glu.h || { echo 'Missing real native build dependencies' >&2; exit 1; }
	@mkdir -p $(FULL_OOLITE_OBJECTS) $(OUT)/oolite-patched-src/src/Core $(OUT)/oolite-patched-src/src/SDL/EXRSnapshotSupport
	@for name in Core/GameController.m Core/OOLogOutputHandler.m Core/OOLogging.m Core/Universe.m Core/OOCPUInfo.m Core/OOCPUInfo.h Core/OOLogHeader.m Core/OOOpenGL.m Core/OOPolygonSprite.m SDL/EXRSnapshotSupport/miniz.c SDL/main.m SDL/OOSDLJoystickManager.m; do mkdir -p $(OUT)/oolite-patched-src/src/$$(dirname "$$name"); cp $(UPSTREAM)/src/$$name $(OUT)/oolite-patched-src/src/$$name; done
	@cd $(OUT)/oolite-patched-src && for patch in $(abspath patches/oolite/0001-gfos-universe-frame-pump.patch) $(abspath patches/oolite/0002-gfos-native-cpu-memory-report.patch) $(abspath patches/oolite/0003-gfos-native-glu-header.patch) $(abspath patches/oolite/0004-gfos-polygon-glu.patch) $(abspath patches/oolite/0005-gfos-miniz-stdio.patch) $(abspath patches/oolite/0006-gfos-gnustep-process-bootstrap.patch) $(abspath patches/oolite/0008-gfos-native-synchronous-logging.patch) $(abspath patches/oolite/0010-gfos-defer-complex-log-header.patch) $(abspath patches/oolite/0011-gfos-defer-early-log-preferences.patch) $(abspath patches/oolite/0013-gfos-controller-default-tick.patch) $(abspath patches/oolite/0014-gfos-sdl3-joystick-lifecycle.patch); do patch -s -N -p1 < "$$patch" || exit 1; done
	@while IFS= read -r name; do test -n "$$name" || continue; src=$(UPSTREAM)/src/$$name; test ! -r $(OUT)/oolite-patched-src/src/$$name || src=$(OUT)/oolite-patched-src/src/$$name; dest=$(FULL_OOLITE_OBJECTS)/$$(printf '%s' "$$name" | tr '/+.' '___').o; case "$$name" in *.m) cc=$(CROSS_CC); lang='-x objective-c -std=gnu11 -fobjc-exceptions';; *.c) cc=$(CROSS_CC); lang='-x c -std=gnu11';; *.cpp|*.cc) cc=$(PREFIX)/bin/m68k-amigaos-g++; lang='-x c++ -std=gnu++17';; *) echo "Unsupported source: $$name"; exit 1;; esac; if test -s "$$dest" && test "$$dest" -nt "$$src"; then continue; fi; echo "Compiling $$name"; "$$cc" $$lang -O2 -m68040 -m68881 -noixemul $(FULL_DEFINES) -I $(OUT)/oolite-patched-src/src/Core $(FULL_INCLUDES) -c "$$src" -o "$$dest" || exit 1; done < $(FULL_LIST)

# Run this only with the matched native Foundation/SpiderMonkey/GL/GFOS SDK.
# GNUstep's original archive needs a symbol index; never rewrite it in place.
.PHONY: oolite-full-original-link
oolite-full-original-link: oolite-full-original-objects
	@cp $(GFOS_BASE_ARCHIVE) $(OUT)/libgnustep-base-indexed.a
	@$(PREFIX)/bin/m68k-amigaos-ranlib $(OUT)/libgnustep-base-indexed.a
	@mkdir -p $(OUT)/gnustep-patched/Source
	@cp $(GFOS_BASE_DIR)/Source/NSMessagePortNameServer.m $(OUT)/gnustep-patched/Source/
	@cd $(OUT)/gnustep-patched && patch -s -N -p1 < $(abspath $(FULL_GNUSTEP_PATCH))
	$(CROSS_CC) -x objective-c -std=gnu11 -O2 -fobjc-exceptions -m68040 -m68881 -noixemul -I $(GFOS_BASE_DIR)/Source -I $(GFOS_BASE_DIR)/Headers -I $(PREFIX)/m68k-amigaos/ndk-include.vendor -c $(OUT)/gnustep-patched/Source/NSMessagePortNameServer.m -o $(OUT)/NSMessagePortNameServer.o
	$(CROSS_CC) -x objective-c -O2 -fobjc-exceptions -m68040 -m68881 -noixemul -I src/Core -I $(GFOS_BASE_DIR)/Headers -I $(UPSTREAM)/src/Core/MiniZip -I $(PREFIX)/m68k-amigaos/port-deps/include -c $(FULL_EXTRACTOR) -o $(OUT)/OOOXZGFOSExtraction.o
	$(CROSS_CC) -x c -std=gnu11 -O2 -m68040 -m68881 -noixemul -I src/Core -I $(UPSTREAM)/src/Core/MiniZip -I $(PREFIX)/m68k-amigaos/port-deps/include -c $(FULL_PREFLIGHT) -o $(OUT)/OOOXZArchivePreflight.o
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -I src/Core -I $(GFOS_JS_INC) -I $(PREFIX)/m68k-amigaos/ndk-include.vendor -c src/Core/OOGFOSGameDispatch.c -o $(OUT)/OOGFOSGameDispatch.o
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -I src/Core -I $(GFOS_JS_INC) -I $(PREFIX)/m68k-amigaos/ndk-include.vendor -c src/Core/OOGFOSGameFrame.c -o $(OUT)/OOGFOSGameFrame.o
	$(CROSS_CC) -O2 -m68040 -m68881 -noixemul -I src/Core -I $(PREFIX)/m68k-amigaos/ndk-include.vendor -c src/Core/OOGFOSSystemInfo.c -o $(OUT)/OOGFOSSystemInfo.o
	$(PREFIX)/bin/m68k-amigaos-g++ -m68040 -m68881 -noixemul $(FULL_OOLITE_OBJECTS)/*.o $(OUT)/OOGFOSGameDispatch.o $(OUT)/OOGFOSGameFrame.o $(OUT)/OOGFOSSystemInfo.o $(OUT)/OOOXZGFOSExtraction.o $(OUT)/OOOXZArchivePreflight.o $(FULL_NSMESSAGE_OBJ) $(OUT)/NSMessagePortNameServer.o -Wl,--start-group -Wl,--whole-archive $(OUT)/libgnustep-base-indexed.a -Wl,--no-whole-archive $(FULL_JS185_LIB) $(FULL_SDL3_LIB) $(FULL_OPENAL_LIB) $(FULL_GLU_LIB) $(FULL_OGG_LIB)/libvorbisfile.a $(FULL_OGG_LIB)/libvorbis.a $(FULL_OGG_LIB)/libogg.a $(PREFIX)/m68k-amigaos/port-deps/lib/libpng.a -lGL -lobjc $(FULL_FFI_LIB) $(PREFIX)/m68k-amigaos/port-deps/lib/libz.a -lpthread -L $(GFOS_LIBNIX_LIB) -lsocket -lm -Wl,--end-group -o $(FULL_OOLITE_EXE)

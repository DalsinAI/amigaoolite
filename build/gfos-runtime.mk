# GFOS runtime gates for the Oolite port: the game on javascript.library's
# script task, and the Foundation the game needs (user defaults, NSLog, time
# zones, exceptions, Amiga paths).
# Run with: make -f Makefile -f build/gfos-runtime.mk gfos-runtime-gates
#
# Rules every GFOS Objective-C build needs (found 19 Nov 2026):
#  - compile with -fobjc-exceptions -fexceptions -D_NATIVE_OBJC_EXCEPTIONS;
#  - LINK with -fexceptions too: without it the program never registers its
#    exception frames and every @throw aborts ("Program aborted");
#  - link the GFOS Foundation from tools/gfos-foundation (68040+FPU, native
#    exceptions, Amiga path roots) and use its Headers;
#  - link src/Core/OOGFOSPasswd.c ahead of libsocket (user, group, uid);
#  - call OOGFOSAbsoluteArgv() before GSInitializeProcess().

GFOS_FOUNDATION_DIR ?= $(OUT)/gnustep-base-gfos
GFOS_FOUNDATION_LIB := $(GFOS_FOUNDATION_DIR)/libgnustep-base.a
GFOS_FOUNDATION_HEADERS := $(GFOS_FOUNDATION_DIR)/Headers
GFOS_PTHREAD_LIB := $(OUT)/libpthread-gfos.a
GFOS_EH_FLAGS := -fobjc-exceptions -fexceptions -D_NATIVE_OBJC_EXCEPTIONS
GFOS_OBJC_FLAGS := -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul $(GFOS_EH_FLAGS) \
	-DGNUSTEP_BASE_LIBRARY=1 -I $(GFOS_FOUNDATION_HEADERS) -I src/Core
GFOS_RUNTIME_OBJS := $(OUT)/OOGFOSPasswd.o $(OUT)/OOGFOSProcess.o
GFOS_RUNTIME_LINK = -fexceptions -m68040 -m68881 -noixemul $(GFOS_RUNTIME_OBJS) \
	-L $(GFOS_DEPS_LIB) -L $(GFOS_LIBNIX_LIB) -Wl,--start-group \
	-Wl,--whole-archive $(GFOS_FOUNDATION_LIB) -Wl,--no-whole-archive \
	$(GFOS_LIB_DIR)/libgfosfoundationports.a $(GFOS_LIB_DIR)/libgfosfoundationipc.a \
	$(GFOS_FFI_DIR)/lib/libffi.a -lobjc $(GFOS_PTHREAD_LIB) -lm -lz -lsocket -Wl,--end-group

$(GFOS_FOUNDATION_LIB):
	OUT=$(abspath $(GFOS_FOUNDATION_DIR)) PREFIX=$(PREFIX) sh tools/gfos-foundation/build-gnustep-base-gfos.sh

# libnix pthreads with its thread table global, so the game job can take over
# the main-thread slot on the script task (see the gate's comments).
$(GFOS_PTHREAD_LIB):
	@mkdir -p $(OUT)
	$(PREFIX)/bin/m68k-amigaos-objcopy --globalize-symbol=__threads $(PREFIX)/m68k-amigaos/lib/libpthread.a $@

$(OUT)/OOGFOSPasswd.o: src/Core/OOGFOSPasswd.c
	@mkdir -p $(OUT)
	$(CROSS_CC) -std=gnu11 -O2 -m68040 -m68881 -noixemul -idirafter $(PREFIX)/m68k-amigaos/ndk-include.vendor -c $< -o $@
$(OUT)/OOGFOSProcess.o: src/Core/OOGFOSProcess.c src/Core/OOGFOSProcess.h
	@mkdir -p $(OUT)
	$(CROSS_CC) -std=gnu11 -O2 -m68040 -m68881 -noixemul -I src/Core -I $(PREFIX)/m68k-amigaos/ndk-include.vendor -c $< -o $@

$(OUT)/oolite-game-on-script-task-68040: tests/oolite_js_game_on_script_task_gate.m $(GFOS_FOUNDATION_LIB) $(GFOS_PTHREAD_LIB) $(GFOS_RUNTIME_OBJS)
	$(CROSS_CC) $(GFOS_OBJC_FLAGS) -DOOGFOS_GATE_EXCEPTIONS=1 -I $(GFOS_JS_INC) -I $(PREFIX)/m68k-amigaos/ndk-include.vendor -c $< -o $(OUT)/gate-ost.o
	$(CROSS_CC) $(OUT)/gate-ost.o $(GFOS_RUNTIME_LINK) -o $@

$(OUT)/gfos-defaults-probe-68040: tests/gfos_defaults_probe.m $(GFOS_FOUNDATION_LIB) $(GFOS_PTHREAD_LIB) $(GFOS_RUNTIME_OBJS)
	$(CROSS_CC) $(GFOS_OBJC_FLAGS) -c $< -o $(OUT)/defaults-probe.o
	$(CROSS_CC) $(OUT)/defaults-probe.o $(GFOS_RUNTIME_LINK) -o $@

$(OUT)/gfos-objc-eh-probe-68040: tests/gfos_objc_eh_probe.m
	@mkdir -p $(OUT)
	$(CROSS_CC) -x objective-c -O2 -m68040 -m68881 -noixemul $(GFOS_EH_FLAGS) -c $< -o $(OUT)/objc-eh-probe.o
	$(CROSS_CC) -fexceptions -m68040 -m68881 -noixemul $(OUT)/objc-eh-probe.o -lobjc -lpthread -o $@

.PHONY: gfos-runtime-gates gfos-foundation
gfos-foundation: $(GFOS_FOUNDATION_LIB)
gfos-runtime-gates: $(OUT)/oolite-game-on-script-task-68040 $(OUT)/gfos-defaults-probe-68040 $(OUT)/gfos-objc-eh-probe-68040

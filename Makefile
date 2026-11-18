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

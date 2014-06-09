VERBOSE ?= @

BDEVERIFY ?= $(BDE_VERIFY_DIR)/scripts/bde_verify

ALL ?= off

define CHECKARGS
    -nodefdef                                                                 \
    -config=/dev/null                                                         \
    -cl='namespace bde_verify'                                                \
    -cl='all $(ALL)'                                                          \
    $(foreach N,$(CHECKNAME),-cl='check $(N) on')                             \
    -fcxx-exceptions                                                          \
    -Wno-string-plus-int                                                      \
    -std=c++0x                                                                \
    -D_X86INTRIN_H_INCLUDED                                                   \
    -I $(BDE_VERIFY_DIR)/groups/csa/csadep                                    \
    -I $(BDE_VERIFY_DIR)/checks/include                                       \
    $(BDE_VERIFY_ARGS)
endef

define CHECK
	$(VERBOSE) $(BDEVERIFY) $(CHECKARGS) $(basename $@) 2>&1 | \
        diff -q - *.exp >/dev/null && echo OK $(basename $@) || \
    $(BDEVERIFY) $(CHECKARGS) $(basename $@) 2>&1 | \
        diff - *.exp
endef

define RUN
	$(VERBOSE) $(BDEVERIFY) $(CHECKARGS) -fcolor-diagnostics $(basename $@)
endef

CHECKFILES = $(patsubst %,%.check,$(FILES))
RUNFILES   = $(patsubst %,%.run,$(FILES))

.PHONY: run check $(CHECKFILES) $(RUNFILES)

check: $(CHECKFILES)

$(CHECKFILES):
	$(CHECK)

run: $(RUNFILES)

$(RUNFILES):
	$(RUN)

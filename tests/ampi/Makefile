-include ../../include/conv-mach-opt.mak

DIRS = \
  megampi \
  stacksize \
  migration \
  intercomm_coll \
  datatype \
  privatization \
  jacobi3d \
  exit \
#  chkpt \

NONSCALEDIRS = \
  privatization \
  datatype \
  exit \

ifeq ($(BUILD_MPICH_TESTS),true)
DIRS += mpich-test
endif

FTDIRS = \
  jacobi3d

TESTDIRS = $(filter-out $(FTDIRS),$(DIRS))

TESTPDIRS = $(filter-out $(NONSCALEDIRS),$(TESTDIRS))

all: $(foreach i,$(DIRS),build-$i)

test: $(foreach i,$(TESTDIRS),test-$i)

testp: $(foreach i,$(TESTPDIRS),testp-$i)

syncfttest : $(foreach i,$(filter $(FTDIRS),$(DIRS)),test-$i)

mpisyncfttest :

clean: $(foreach i,$(DIRS),clean-$i)
	rm -f TAGS #*#
	rm -f core *~

$(foreach i,$(DIRS),build-$i):
	$(MAKE) -C $(subst build-,,$@) all OPTS='$(OPTS)'

$(foreach i,$(DIRS),test-$i):
	$(MAKE) -C $(subst test-,,$@) test OPTS='$(OPTS)' TESTOPTS='$(TESTOPTS)'

$(foreach i,$(TESTPDIRS),testp-$i):
	$(MAKE) -C $(subst testp-,,$@) testp OPTS='$(OPTS)' TESTOPTS='$(TESTOPTS)' P='$(P)'

$(foreach i,$(DIRS),clean-$i):
	$(MAKE) -C $(subst clean-,,$@) clean OPTS='$(OPTS)'


# additional dependencies

mpich-test/Makefile:
	cd mpich-test && ./configure.sh && cd ..

build-mpich-test: mpich-test/Makefile
test-mpich-test: mpich-test/Makefile

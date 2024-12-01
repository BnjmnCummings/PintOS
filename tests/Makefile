TESTS = examples devices threads userprog vm filesys

PATH := $(shell pwd)/../src/utils:$(PATH)

all:
	@echo "This Makefile has only \`check' targets."

check:
	$(MAKE) -C .. distclean
	for d in $(TESTS); do $(MAKE) $$d || exit 1; done
	@echo All tests passed.

clean:
	rm -rf $(TESTS)

define mk-sandbox
rm -rf $@ && mkdir $@ && cp -R ../src $@/src
cd $@/src && $(MAKE) -s clean
endef

define run-tests
cd $@/src/$(TASK) && time $(MAKE) check
endef

define grade-tests
cd $@/src/$(TASK) && $(MAKE) grade
cd $@/src/$(TASK) && grep -q PERFECT build/grade
cd $@/src/$(TASK) && grep -qv warning build/grade
endef

define compile
cd $@/src/$(TASK) && $(MAKE)
endef

define test-clean
cd $@/src/$(TASK)/build && $(MAKE) clean
cd $@/src/$(TASK)/build && set `find . -type f` > /dev/null && test $$# = 1
endef

define clean
rm -rf $@
endef

TASK = $@

# Tests that just compile the provided code.
devices::
	$(mk-sandbox)
	$(compile)
	$(run-tests)
	$(test-clean)
	$(clean)
threads::
	$(mk-sandbox)
	$(compile)
	$(run-tests) 
	$(test-clean)
	$(clean)
userprog vm filesys::
	$(mk-sandbox)
	$(compile)
	$(test-clean)
	$(clean)
examples::
	$(mk-sandbox)
	$(compile)
	$(clean)

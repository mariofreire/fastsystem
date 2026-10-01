SUBDIRS := boot fat libc libpci loader kernel user # efi (optional)

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS) run

$(SUBDIRS):
	$(MAKE) -C $@

run:
	$(MAKE) -C kernel run

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

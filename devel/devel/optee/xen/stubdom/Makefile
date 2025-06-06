XEN_ROOT = $(CURDIR)/..
MINI_OS = $(XEN_ROOT)/extras/mini-os

export XEN_ROOT
export XEN_OS=MiniOS
export PKG_CONFIG_DIR = $(CURDIR)/pkg-config

# Remove flags which are meant for tools, e.g. "-m64"
export EXTRA_CFLAGS_XEN_TOOLS=
export EXTRA_CFLAGS_QEMU_TRADITIONAL=

export stubdom=y
export debug=y
# Moved from config/StdGNU.mk
CFLAGS += -O1 -fno-omit-frame-pointer

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
  ifeq ($(wildcard $(MINI_OS)/Config.mk),)
    $(error Please run 'make mini-os-dir' in top-level directory)
  endif
  include $(XEN_ROOT)/Config.mk
endif

-include $(XEN_ROOT)/config/Stubdom.mk

include $(XEN_ROOT)/tools/libs/uselibs.mk

define LIB_deps
 LIBDEPS_$(1) = $$(foreach use,$$(USELIBS_$(1)),libxen$$(use))
endef
$(foreach lib,$(LIBS_LIBS),$(eval $(call LIB_deps,$(lib))))

GNU_TARGET_ARCH:=$(XEN_TARGET_ARCH)
ifeq ($(XEN_TARGET_ARCH),x86_32)
GNU_TARGET_ARCH:=i686
endif

ifeq ($(GNU_TARGET_ARCH), i686)
TARGET_CFLAGS=
NEWLIB_CFLAGS+=-D_I386MACH_ALLOW_HW_INTERRUPTS
STUBDOM_SUPPORTED=1
endif
ifeq ($(GNU_TARGET_ARCH), x86_64)
TARGET_CFLAGS=-mno-red-zone
NEWLIB_CFLAGS+=-D_I386MACH_ALLOW_HW_INTERRUPTS
STUBDOM_SUPPORTED=1
endif

CROSS_ROOT=cross-root-$(GNU_TARGET_ARCH)
CROSS_PREFIX=$(CURDIR)/$(CROSS_ROOT)

# Disable PIE/SSP if GCC supports them. They can break us.
TARGET_CFLAGS += $(CFLAGS)
TARGET_CPPFLAGS += $(CPPFLAGS)
$(call cc-options-add,TARGET_CFLAGS,CC,$(EMBEDDED_EXTRA_CFLAGS))

# Do not use host headers and libs
GCC_INSTALL = $(shell LANG=C gcc -print-search-dirs | sed -n -e 's/install: \(.*\)/\1/p')
TARGET_CPPFLAGS += -U __linux__ -U __FreeBSD__ -U __sun__
TARGET_CPPFLAGS += -nostdinc
TARGET_CPPFLAGS += -isystem $(MINI_OS)/include/posix
TARGET_CPPFLAGS += -isystem $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/include
TARGET_CPPFLAGS += -isystem $(GCC_INSTALL)include
TARGET_CPPFLAGS += -isystem $(CURDIR)/lwip-$(XEN_TARGET_ARCH)/src/include
TARGET_CPPFLAGS += -isystem $(CURDIR)/lwip-$(XEN_TARGET_ARCH)/src/include/ipv4
TARGET_CPPFLAGS += -I$(CURDIR)/include
TARGET_CPPFLAGS += -I$(XEN_ROOT)/xen/include

TARGET_LDFLAGS += -nostdlib -L$(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/lib

TARGETS=$(STUBDOM_TARGETS)

STUBDOMPATH="stubdompath.sh"
genpath-target = $(call buildmakevars2file,$(STUBDOMPATH))
$(eval $(genpath-target))

.PHONY: all
all: build
ifeq ($(STUBDOM_SUPPORTED),1)
build: $(STUBDOMPATH) $(STUBDOM_BUILD)
else
build: $(STUBDOMPATH)
endif

##############
# Cross-newlib
##############

newlib-$(NEWLIB_VERSION).tar.gz:
	$(FETCHER) $@ $(NEWLIB_URL)/$@

newlib-$(NEWLIB_VERSION): newlib-$(NEWLIB_VERSION).tar.gz
	tar xzf $<
	patch -d $@ -p0 < newlib.patch
	patch -d $@ -p0 < newlib-chk.patch
	patch -d $@ -p1 < newlib-stdint-size_max-fix-from-1.17.0.patch
	patch -d $@ -p1 < newlib-disable-texinfo.patch
	patch -d $@ -p1 < newlib-cygmon-gmon.patch
	patch -d $@ -p1 < newlib-makedoc.patch
	find $@ -type f | xargs perl -i.bak \
		-pe 's/\b_(tzname|daylight|timezone)\b/$$1/g'
	touch $@

NEWLIB_STAMPFILE=$(CROSS_ROOT)/$(GNU_TARGET_ARCH)-xen-elf/lib/libc.a
.PHONY: cross-newlib
cross-newlib: $(NEWLIB_STAMPFILE)
$(NEWLIB_STAMPFILE): mk-headers-$(XEN_TARGET_ARCH) newlib-$(NEWLIB_VERSION)
	mkdir -p newlib-$(XEN_TARGET_ARCH)
	( cd newlib-$(XEN_TARGET_ARCH) && \
	  CC_FOR_TARGET="$(CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(NEWLIB_CFLAGS)" AR_FOR_TARGET=$(AR) LD_FOR_TARGET=$(LD) RANLIB_FOR_TARGET=$(RANLIB) ../newlib-$(NEWLIB_VERSION)/configure --prefix=$(CROSS_PREFIX) --verbose --target=$(GNU_TARGET_ARCH)-xen-elf --enable-newlib-io-long-long --enable-newlib-io-c99-formats --disable-multilib && \
	  $(MAKE) DESTDIR= && \
	  $(MAKE) DESTDIR= install )

############
# Cross-zlib
############

zlib-$(ZLIB_VERSION).tar.gz:
	$(FETCHER) $@ $(ZLIB_URL)/$@

zlib-$(XEN_TARGET_ARCH): zlib-$(ZLIB_VERSION).tar.gz 
	tar xzf $<
	mv zlib-$(ZLIB_VERSION) $@

ZLIB_STAMPFILE=$(CROSS_ROOT)/$(GNU_TARGET_ARCH)-xen-elf/lib/libz.a
.PHONY: cross-zlib
cross-zlib: $(ZLIB_STAMPFILE)
$(ZLIB_STAMPFILE): zlib-$(XEN_TARGET_ARCH) $(NEWLIB_STAMPFILE)
	( cd $< && \
	  CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" CC=$(CC) ./configure --prefix=$(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf && \
	  $(MAKE) DESTDIR= libz.a && \
	  $(MAKE) DESTDIR= install )

##############
# Cross-libpci
##############

pciutils-$(LIBPCI_VERSION).tar.bz2:
	$(FETCHER) $@ $(LIBPCI_URL)/$@

pciutils-$(XEN_TARGET_ARCH): pciutils-$(LIBPCI_VERSION).tar.bz2
	tar xjf $<
	mv pciutils-$(LIBPCI_VERSION) $@
	patch -d $@ -p1 < pciutils.patch
	touch $@

LIBPCI_STAMPFILE=$(CROSS_ROOT)/$(GNU_TARGET_ARCH)-xen-elf/lib/libpci.a
.PHONY: cross-libpci
cross-libpci: $(LIBPCI_STAMPFILE)
$(LIBPCI_STAMPFILE): pciutils-$(XEN_TARGET_ARCH) $(NEWLIB_STAMPFILE) $(ZLIB_STAMPFILE)
	( cd $< && \
	  cp ../libpci.config.h lib/config.h && \
	  chmod u+w lib/config.h && \
	  echo '#define PCILIB_VERSION "$(LIBPCI_VERSION)"' >> lib/config.h && \
	  ln -sf ../../libpci.config.mak lib/config.mk && \
	  $(MAKE) DESTDIR= CC="$(CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -I$(call realpath,$(MINI_OS)/include)" lib/libpci.a && \
	  $(INSTALL_DATA) lib/libpci.a $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/lib/ && \
	  $(INSTALL_DIR) $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/include/pci && \
	  $(INSTALL_DATA) lib/config.h lib/header.h lib/pci.h lib/types.h $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/include/pci/ \
	)

######
# lwIP
######

lwip-$(LWIP_VERSION).tar.gz:
	$(FETCHER) $@ $(LWIP_URL)/$@

lwip-$(XEN_TARGET_ARCH): lwip-$(LWIP_VERSION).tar.gz
	tar xzf $<
	mv lwip $@
	patch -d $@ -p0 < lwip.patch-cvs
	patch -d $@ -p0 < lwip.dhcp_create_request-hwaddr_len.patch
	touch $@

#############
# cross-gmp
#############
gmp-$(GMP_VERSION).tar.bz2:
	$(FETCHER) $@ $(GMP_URL)/$@

.PHONY: cross-gmp
ifeq ($(XEN_TARGET_ARCH), x86_32)
   GMPEXT=ABI=32
endif
gmp-$(XEN_TARGET_ARCH): gmp-$(GMP_VERSION).tar.bz2 $(NEWLIB_STAMPFILE)
	tar xjf $<
	rm $@ -rf || :
	mv gmp-$(GMP_VERSION) $@
	#patch -d $@ -p0 < gmp.patch
	cd $@; CPPFLAGS="-isystem $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/include $(TARGET_CPPFLAGS)" CFLAGS="$(TARGET_CFLAGS)" CC=$(CC) $(GMPEXT) ./configure --disable-shared --enable-static --disable-fft --without-readline --prefix=$(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf --libdir=$(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/lib --build=`gcc -dumpmachine` --host=$(GNU_TARGET_ARCH)-xen-elf
	sed -i 's/#define HAVE_OBSTACK_VPRINTF 1/\/\/#define HAVE_OBSTACK_VPRINTF 1/' $@/config.h
	touch $@

GMP_STAMPFILE=$(CROSS_ROOT)/$(GNU_TARGET_ARCH)-xen-elf/lib/libgmp.a
cross-gmp: $(GMP_STAMPFILE)
$(GMP_STAMPFILE): gmp-$(XEN_TARGET_ARCH)
	( cd $< && \
	  $(MAKE) && \
	  $(MAKE) DESTDIR= install )

#############
# cross-polarssl
#############
polarssl-$(POLARSSL_VERSION)-gpl.tgz:
	$(FETCHER) $@ $(POLARSSL_URL)/$@

polarssl-$(XEN_TARGET_ARCH): polarssl-$(POLARSSL_VERSION)-gpl.tgz
	tar xzf $<
	mv polarssl-$(POLARSSL_VERSION) $@
	patch -d $@ -p1 < polarssl.patch
	touch $@

POLARSSL_STAMPFILE=$(CROSS_ROOT)/$(GNU_TARGET_ARCH)-xen-elf/lib/libpolarssl.a
cross-polarssl: $(POLARSSL_STAMPFILE)
$(POLARSSL_STAMPFILE): polarssl-$(XEN_TARGET_ARCH) $(NEWLIB_STAMPFILE) lwip-$(XEN_TARGET_ARCH)
	 ( cd $</library && \
	   make CC="$(CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -I $(realpath $(MINI_OS)/include)" && \
	   mkdir -p $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/include && \
	   cp -r ../include/* $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/include && \
	   mkdir -p $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/lib && \
	   $(INSTALL_DATA) libpolarssl.a $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/lib/ )

#############
# cross-tpmemu
#############
tpm_emulator-$(TPMEMU_VERSION).tar.gz:
	$(FETCHER) $@ $(TPMEMU_URL)/$@

tpm_emulator-$(XEN_TARGET_ARCH): tpm_emulator-$(TPMEMU_VERSION).tar.gz
	@if [ ! -f "$(CMAKE)" ]; then echo "cmake not installed! Install it and then rerun ./configure!"; false; fi
	tar xzf $<
	mv tpm_emulator-$(TPMEMU_VERSION) $@
	patch -d $@ -p1 < tpmemu-$(TPMEMU_VERSION).patch;
	patch -d $@ -p1 < vtpm-bufsize.patch
	patch -d $@ -p1 < vtpm-locality.patch
	patch -d $@ -p1 < vtpm-parent-sign-ek.patch
	patch -d $@ -p1 < vtpm-deepquote.patch
	patch -d $@ -p1 < vtpm-deepquote-anyloc.patch
	patch -d $@ -p1 < vtpm-cmake-Wextra.patch
	patch -d $@ -p1 < vtpm-implicit-fallthrough.patch
	patch -d $@ -p1 < vtpm_TPM_ChangeAuthAsymFinish.patch
	patch -d $@ -p1 < vtpm_extern.patch
	patch -d $@ -p1 < vtpm-microsecond-duration.patch
	patch -d $@ -p1 < vtpm-command-duration.patch
	patch -d $@ -p1 < vtpm-tpm_bn_t-addr.patch
	mkdir $@/build
	cd $@/build; CC=${CC} $(CMAKE) .. -DCMAKE_C_FLAGS:STRING="-std=c99 -DTPM_NO_EXTERN $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -Wno-declaration-after-statement"
	touch $@

TPMEMU_STAMPFILE=$(CROSS_ROOT)/$(GNU_TARGET_ARCH)-xen-elf/lib/libtpm.a
$(TPMEMU_STAMPFILE): tpm_emulator-$(XEN_TARGET_ARCH) $(GMP_STAMPFILE)
	( cd $</build && make VERBOSE=1 tpm_crypto tpm  )
	cp $</build/crypto/libtpm_crypto.a $(CROSS_ROOT)/$(GNU_TARGET_ARCH)-xen-elf/lib/libtpm_crypto.a
	cp $</build/tpm/libtpm.a $(TPMEMU_STAMPFILE)

.PHONY: cross-tpmemu
cross-tpmemu: $(TPMEMU_STAMPFILE)

#############
# Cross-ocaml
#############

CAMLLIB = $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/lib/ocaml
OCAML_STAMPFILE=$(CAMLLIB)/.dirstamp

ocaml-$(OCAML_VERSION).tar.gz:
	$(FETCHER) $@ $(OCAML_URL)/$@

ocaml-$(XEN_TARGET_ARCH)/.dirstamp: ocaml-$(OCAML_VERSION).tar.gz ocaml.patch
	tar xzf $<
	cd ocaml-$(OCAML_VERSION) && patch -p0 < ../ocaml.patch
	rm -rf ocaml-$(XEN_TARGET_ARCH)
	mv ocaml-$(OCAML_VERSION) ocaml-$(XEN_TARGET_ARCH)
	touch $@

MINIOS_HASNOT=IPV6 INET_ATON

.PHONY: cross-ocaml
cross-ocaml: $(OCAML_STAMPFILE)
$(OCAML_STAMPFILE): ocaml-$(XEN_TARGET_ARCH)/.dirstamp
	cd ocaml-$(XEN_TARGET_ARCH) &&  ./configure -prefix $(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf \
		-no-pthread -no-shared-libs -no-tk -no-curses \
		-cc "$(CC) -U_FORTIFY_SOURCE -fno-stack-protector -mno-red-zone"
	$(foreach i,$(MINIOS_HASNOT),sed -i 's,^\(#define HAS_$(i)\),//\1,' ocaml-$(XEN_TARGET_ARCH)/config/s.h ; )
	$(MAKE) DESTDIR= -C ocaml-$(XEN_TARGET_ARCH) world
	$(MAKE) DESTDIR= -C ocaml-$(XEN_TARGET_ARCH) opt
	$(MAKE) -C ocaml-$(XEN_TARGET_ARCH) install
	touch $@

#######
# Links
#######

.PHONY: $(CROSS_ROOT)
$(CROSS_ROOT): cross-newlib cross-zlib cross-libpci

QEMU_ROOT := $(shell if [ -d "$(QEMU_TRADITIONAL_LOC)" ]; then echo "$(QEMU_TRADITIONAL_LOC)"; else echo .; fi)

ifneq ($(filter ioemu,$(STUBDOM_TARGETS)),)
IOEMU_LINKFARM_TARGET := ioemu/linkfarm.stamp
endif

ifeq ($(QEMU_ROOT),.)
$(XEN_ROOT)/tools/qemu-xen-traditional-dir:
	$(MAKE) DESTDIR= -C $(XEN_ROOT)/tools qemu-xen-traditional-dir-find

ioemu/linkfarm.stamp: $(XEN_ROOT)/tools/qemu-xen-traditional-dir
	mkdir -p ioemu
	set -e;									\
	$(buildmakevars2shellvars);						\
	cd ioemu;								\
	src="$$XEN_ROOT/tools/qemu-xen-traditional-dir"; export src;		\
	(cd $$src && find * -type d						\
		$(addprefix ! -path , '*-softmmu*' '*-linux-user*') -print)	\
		| xargs mkdir -p;						\
	(cd $$src && find *	! -type l  -type f  $(addprefix ! -path ,	\
			'*.[oda1]' 'config-*' config.mak qemu-dm qemu-img-xen	\
			'*.html' '*.pod' '*-softmmu*' '*-linux-user*'		\
			)) >linkfarm.stamp.tmp;				\
	cmp -s linkfarm.stamp.tmp linkfarm.stamp &&			\
		rm linkfarm.stamp.tmp || {				\
		mv linkfarm.stamp.tmp linkfarm.stamp;			\
		cat linkfarm.stamp | while read f;			\
			do rm -f "$$f"; ln -s "$$src/$$f" "$$f"; done;	\
	}
else
export QEMU_ROOT

ioemu/linkfarm.stamp:
	mkdir -p ioemu
	touch ioemu/linkfarm.stamp
endif

#######
# libraries under tools/libs
#######

STUB_LIBS := toolcore toollog evtchn gnttab call foreignmemory devicemodel ctrl guest

LIBDEP_guest := cross-zlib

#######
# common handling
#######

define do_links
  mkdir -p $(dir $@)include
  cd $(dir $@); \
  ln -sf $(dir $<)include/*.h include/; \
  find $(dir $<) -maxdepth 1 -type f -name '*.[ch]' -exec ln -sf {} . \; ; \
  ln -sf $(dir $<)Makefile.common .
  touch $@
endef

define BUILD_lib
 .PHONY: libxen$(1) clean-libxen$(1)
 libxen$(1): libs-$$(XEN_TARGET_ARCH)/$(1)/libxen$(1).a
 libs-$$(XEN_TARGET_ARCH)/$(1)/libxen$(1).a: $$(LIBDEPS_$(1)) $$(LIBDEP_$(1))
 libs-$$(XEN_TARGET_ARCH)/$(1)/libxen$(1).a: mk-headers-$$(XEN_TARGET_ARCH) $$(NEWLIB_STAMPFILE) .phony
	CPPFLAGS="$$(TARGET_CPPFLAGS)" CFLAGS="$$(TARGET_CFLAGS)" $$(MAKE) CONFIG_LIBXC_MINIOS=y -f $(CURDIR)/xenlibs.mk -C $$(@D) $$(@F)

 clean-libxen$(1):
	if [ -e libs-$$(XEN_TARGET_ARCH)/$(1)/Makefile.common ]; then \
	    $$(MAKE) -f $(CURDIR)/xenlibs.mk -C libs-$$(XEN_TARGET_ARCH)/$(1) clean; \
	fi

 libs-$$(XEN_TARGET_ARCH)/$(1)/stamp: $$(XEN_ROOT)/tools/libs/$(1)/Makefile.common
	$$(do_links)
endef

$(foreach lib,$(STUB_LIBS),$(eval $(call BUILD_lib,$(lib))))

xenstore/stamp: $(XEN_ROOT)/tools/xenstored/Makefile.common
	$(do_links)

xenstorepvh/stamp: $(XEN_ROOT)/tools/xenstored/Makefile.common
	$(do_links)

LINK_DIRS := xenstore xenstorepvh $(foreach dir,$(STUB_LIBS),libs-$(XEN_TARGET_ARCH)/$(dir))
LINK_STAMPS := $(foreach dir,$(LINK_DIRS),$(dir)/stamp)

mk-headers-$(XEN_TARGET_ARCH): $(IOEMU_LINKFARM_TARGET) $(LINK_STAMPS)
	mkdir -p include/xen && \
          ln -sf $(wildcard $(XEN_ROOT)/xen/include/public/*.h) include/xen && \
          ln -sf $(addprefix $(XEN_ROOT)/xen/include/public/,arch-x86 hvm io xsm) include/xen && \
          ( [ -h include/xen/sys ] || ln -sf $(XEN_ROOT)/tools/include/xen-sys/MiniOS include/xen/sys ) && \
          ( [ -h include/xen/libelf ] || ln -sf $(XEN_ROOT)/tools/include/xen/libelf include/xen/libelf ) && \
	  mkdir -p include/xen-foreign && \
	  ln -sf $(wildcard $(XEN_ROOT)/tools/include/xen-foreign/*) include/xen-foreign/ && \
	  $(MAKE) DESTDIR= -C include/xen-foreign/ && \
	  ( [ -h include/xen/foreign ] || ln -sf ../xen-foreign include/xen/foreign )
	$(MAKE) DESTDIR= -C $(MINI_OS) links
	touch mk-headers-$(XEN_TARGET_ARCH)

TARGETS_MINIOS=$(addprefix mini-os-$(XEN_TARGET_ARCH)-,$(TARGETS))
$(TARGETS_MINIOS): mini-os-%:
	[ -d $@ ] || \
	for i in $$(cd $(MINI_OS) ; find . -type d) ; do \
                mkdir -p $@/$$i ; \
	done

#######
# ioemu
#######

ioemu-minios-config.mk: $(CURDIR)/ioemu-minios.cfg
	MINIOS_CONFIG="$<" CONFIG_FILE="$(CURDIR)/$@" $(MAKE) DESTDIR= -C $(MINI_OS) config

.PHONY: ioemu
ioemu: cross-zlib cross-libpci libxenguest ioemu-minios-config.mk
	[ -f ioemu/config-host.mak ] || \
	  ( $(buildmakevars2shellvars); \
	    cd ioemu ; \
	    LWIPDIR=$(CURDIR)/lwip-$(XEN_TARGET_ARCH) \
	    TARGET_CPPFLAGS="$(TARGET_CPPFLAGS) $(shell cat ioemu-minios-config.mk)" \
	    TARGET_CFLAGS="$(TARGET_CFLAGS)" \
	    TARGET_LDFLAGS="$(TARGET_LDFLAGS)" \
	    $(QEMU_ROOT)/xen-setup-stubdom )
	$(MAKE) DESTDIR= -C ioemu -f $(QEMU_ROOT)/Makefile

######
# caml
######

caml-minios-config.mk: $(CURDIR)/caml/minios.cfg
	MINIOS_CONFIG="$<" CONFIG_FILE="$(CURDIR)/$@" $(MAKE) DESTDIR= -C $(MINI_OS) config

.PHONY: caml
caml: $(CROSS_ROOT)
	CPPFLAGS="$(TARGET_CPPFLAGS) $(shell cat caml-minios-config.mk)" CFLAGS="$(TARGET_CFLAGS)" $(MAKE) DESTDIR= -C $@ LWIPDIR=$(CURDIR)/lwip-$(XEN_TARGET_ARCH) OCAMLC_CROSS_PREFIX=$(CROSS_PREFIX)/$(GNU_TARGET_ARCH)-xen-elf/bin/

###
# C
###

c-minios-config.mk: $(CURDIR)/c/minios.cfg
	MINIOS_CONFIG="$<" CONFIG_FILE="$(CURDIR)/$@" $(MAKE) DESTDIR= -C $(MINI_OS) config

.PHONY: c
c: $(CROSS_ROOT) c-minios-config.mk
	CPPFLAGS="$(TARGET_CPPFLAGS) $(shell cat c-minios-config.mk)" CFLAGS="$(TARGET_CFLAGS)" $(MAKE) DESTDIR= -C $@ LWIPDIR=$(CURDIR)/lwip-$(XEN_TARGET_ARCH) 

######
# VTPM
######

vtpm-minios-config.mk: $(CURDIR)/vtpm/minios.cfg
	MINIOS_CONFIG="$<" CONFIG_FILE="$(CURDIR)/$@" $(MAKE) DESTDIR= -C $(MINI_OS) config

.PHONY: vtpm
vtpm: cross-polarssl cross-tpmemu vtpm-minios-config.mk
	XEN_TARGET_ARCH="$(XEN_TARGET_ARCH)" CPPFLAGS="$(TARGET_CPPFLAGS) $(shell cat vtpm-minios-config.mk)" CFLAGS="$(TARGET_CFLAGS)" $(MAKE) -C $@

######
# VTPMMGR
######

vtpmmgr-minios-config.mk: $(CURDIR)/vtpmmgr/minios.cfg
	MINIOS_CONFIG="$<" CONFIG_FILE="$(CURDIR)/$@" $(MAKE) DESTDIR= -C $(MINI_OS) config

.PHONY: vtpmmgr
vtpmmgr: cross-polarssl vtpmmgr-minios-config.mk
	XEN_TARGET_ARCH="$(XEN_TARGET_ARCH)" CPPFLAGS="$(TARGET_CPPFLAGS) $(shell cat vtpmmgr-minios-config.mk)" CFLAGS="$(TARGET_CFLAGS)" $(MAKE) -C $@

######
# Grub
######

grub-$(GRUB_VERSION).tar.gz:
	$(FETCHER) $@ $(GRUB_URL)/$@

grub-upstream: grub-$(GRUB_VERSION).tar.gz
	tar xzf $<
	mv grub-$(GRUB_VERSION) $@
	for i in grub.patches/* ; do \
		patch -d $@ -p1 < $$i || exit 1; \
	done

grub-$(XEN_TARGET_ARCH)-minios-config.mk: $(CURDIR)/grub/minios.cfg
	MINIOS_CONFIG="$<" CONFIG_FILE="$(CURDIR)/$@" $(MAKE) DESTDIR= -C $(MINI_OS) config

.PHONY: grub
grub: cross-polarssl grub-upstream $(CROSS_ROOT) grub-$(XEN_TARGET_ARCH)-minios-config.mk
	mkdir -p grub-$(XEN_TARGET_ARCH)
	CPPFLAGS="$(TARGET_CPPFLAGS) $(shell cat grub-$(XEN_TARGET_ARCH)-minios-config.mk)" CFLAGS="$(TARGET_CFLAGS)" $(MAKE) DESTDIR= -C $@ OBJ_DIR=$(CURDIR)/grub-$(XEN_TARGET_ARCH)

##########
# xenstore
##########

xenstore-minios-config.mk: $(CURDIR)/xenstore-minios.cfg
	MINIOS_CONFIG="$<" CONFIG_FILE="$(CURDIR)/$@" $(MAKE) DESTDIR= -C $(MINI_OS) config

.PHONY: xenstore
xenstore: $(CROSS_ROOT) xenstore-minios-config.mk
	CPPFLAGS="$(TARGET_CPPFLAGS) $(shell cat xenstore-minios-config.mk)" CFLAGS="$(TARGET_CFLAGS)" $(MAKE) -f $(CURDIR)/xenlibs.mk -C $@ xenstored.a CONFIG_STUBDOM=y

#############
# xenstorepvh
#############

xenstorepvh-minios-config.mk: $(CURDIR)/xenstorepvh-minios.cfg
	MINIOS_CONFIG="$<" CONFIG_FILE="$(CURDIR)/$@" $(MAKE) DESTDIR= -C $(MINI_OS) config

.PHONY: xenstorepvh
xenstorepvh: $(CROSS_ROOT) xenstorepvh-minios-config.mk
	CPPFLAGS="$(TARGET_CPPFLAGS) $(shell cat xenstorepvh-minios-config.mk)" CFLAGS="$(TARGET_CFLAGS)" $(MAKE) -f $(CURDIR)/xenlibs.mk -C $@ xenstored.a CONFIG_STUBDOM=y

########
# minios
########

.PHONY: ioemu-stubdom
ioemu-stubdom: APP_OBJS=$(CURDIR)/ioemu/i386-stubdom/qemu.a $(CURDIR)/ioemu/i386-stubdom/libqemu.a $(CURDIR)/ioemu/libqemu_common.a
ioemu-stubdom: mini-os-$(XEN_TARGET_ARCH)-ioemu lwip-$(XEN_TARGET_ARCH) libxenguest ioemu
	DEF_CPPFLAGS="$(TARGET_CPPFLAGS)" DEF_CFLAGS="$(TARGET_CFLAGS)" DEF_LDFLAGS="$(TARGET_LDFLAGS)" MINIOS_CONFIG="$(CURDIR)/ioemu-minios.cfg" $(MAKE) DESTDIR= -C $(MINI_OS) OBJ_DIR=$(CURDIR)/$< LWIPDIR=$(CURDIR)/lwip-$(XEN_TARGET_ARCH) APP_OBJS="$(APP_OBJS)"

.PHONY: caml-stubdom
caml-stubdom: mini-os-$(XEN_TARGET_ARCH)-caml lwip-$(XEN_TARGET_ARCH) libxenguest cross-ocaml caml
	DEF_CPPFLAGS="$(TARGET_CPPFLAGS)" DEF_CFLAGS="$(TARGET_CFLAGS)" DEF_LDFLAGS="$(TARGET_LDFLAGS)" MINIOS_CONFIG="$(CURDIR)/caml/minios.cfg" $(MAKE) DESTDIR= -C $(MINI_OS) OBJ_DIR=$(CURDIR)/$< LWIPDIR=$(CURDIR)/lwip-$(XEN_TARGET_ARCH) APP_OBJS="$(CURDIR)/caml/main-caml.o $(CURDIR)/caml/caml.o $(CAMLLIB)/libasmrun.a"

.PHONY: c-stubdom
c-stubdom: mini-os-$(XEN_TARGET_ARCH)-c lwip-$(XEN_TARGET_ARCH) libxenguest c
	DEF_CPPFLAGS="$(TARGET_CPPFLAGS)" DEF_CFLAGS="$(TARGET_CFLAGS)" DEF_LDFLAGS="$(TARGET_LDFLAGS)" MINIOS_CONFIG="$(CURDIR)/c/minios.cfg" $(MAKE) DESTDIR= -C $(MINI_OS) OBJ_DIR=$(CURDIR)/$< LWIPDIR=$(CURDIR)/lwip-$(XEN_TARGET_ARCH) APP_OBJS=$(CURDIR)/c/main.a

.PHONY: vtpm-stubdom
vtpm-stubdom: mini-os-$(XEN_TARGET_ARCH)-vtpm vtpm
	DEF_CPPFLAGS="$(TARGET_CPPFLAGS)" DEF_CFLAGS="$(TARGET_CFLAGS)" DEF_LDFLAGS="$(TARGET_LDFLAGS)" MINIOS_CONFIG="$(CURDIR)/vtpm/minios.cfg" $(MAKE) -C $(MINI_OS) OBJ_DIR=$(CURDIR)/$< APP_OBJS="$(CURDIR)/vtpm/vtpm.a" APP_LDLIBS="-ltpm -ltpm_crypto -lgmp -lpolarssl"

.PHONY: vtpmmgr-stubdom
vtpmmgr-stubdom: mini-os-$(XEN_TARGET_ARCH)-vtpmmgr vtpmmgr
	DEF_CPPFLAGS="$(TARGET_CPPFLAGS)" DEF_CFLAGS="$(TARGET_CFLAGS)" DEF_LDFLAGS="$(TARGET_LDFLAGS)" MINIOS_CONFIG="$(CURDIR)/vtpmmgr/minios.cfg" $(MAKE) -C $(MINI_OS) OBJ_DIR=$(CURDIR)/$< APP_OBJS="$(CURDIR)/vtpmmgr/vtpmmgr.a" APP_LDLIBS="-lm -lpolarssl"

.PHONY: pv-grub
pv-grub: mini-os-$(XEN_TARGET_ARCH)-grub libxenguest grub
	DEF_CPPFLAGS="$(TARGET_CPPFLAGS)" DEF_CFLAGS="$(TARGET_CFLAGS)" DEF_LDFLAGS="$(TARGET_LDFLAGS)" MINIOS_CONFIG="$(CURDIR)/grub/minios.cfg" $(MAKE) DESTDIR= -C $(MINI_OS) OBJ_DIR=$(CURDIR)/$< APP_OBJS=$(CURDIR)/grub-$(XEN_TARGET_ARCH)/main.a

.PHONY: pv-grub-if-enabled
ifneq ($(filter grub,$(STUBDOM_TARGETS)),)
pv-grub-if-enabled: pv-grub
else
pv-grub-if-enabled:
endif

.PHONY: xenstore-stubdom
xenstore-stubdom: mini-os-$(XEN_TARGET_ARCH)-xenstore libxenguest xenstore
	DEF_CPPFLAGS="$(TARGET_CPPFLAGS)" DEF_CFLAGS="$(TARGET_CFLAGS)" DEF_LDFLAGS="$(TARGET_LDFLAGS)" MINIOS_CONFIG="$(CURDIR)/xenstore-minios.cfg" $(MAKE) DESTDIR= -C $(MINI_OS) OBJ_DIR=$(CURDIR)/$< APP_OBJS=$(CURDIR)/xenstore/xenstored.a

.PHONY: xenstorepvh-stubdom
xenstorepvh-stubdom: mini-os-$(XEN_TARGET_ARCH)-xenstorepvh libxenguest xenstorepvh
	DEF_CPPFLAGS="$(TARGET_CPPFLAGS)" DEF_CFLAGS="$(TARGET_CFLAGS)" DEF_LDFLAGS="$(TARGET_LDFLAGS)" MINIOS_CONFIG="$(CURDIR)/xenstorepvh-minios.cfg" $(MAKE) DESTDIR= -C $(MINI_OS) OBJ_DIR=$(CURDIR)/$< APP_OBJS=$(CURDIR)/xenstorepvh/xenstored.a

#########
# install
#########

ifeq ($(STUBDOM_SUPPORTED),1)
install: $(STUBDOMPATH) $(STUBDOM_INSTALL)
else
install: $(STUBDOMPATH)
endif

install-ioemu: ioemu-stubdom
	$(INSTALL_DIR) "$(DESTDIR)$(LIBEXEC_BIN)"
	$(INSTALL_PROG) stubdom-dm "$(DESTDIR)$(LIBEXEC_BIN)"
	$(INSTALL_DATA) stubdompath.sh "$(DESTDIR)$(LIBEXEC_BIN)"
	$(INSTALL_DIR) "$(DESTDIR)$(XENFIRMWAREDIR)"
	$(INSTALL_DATA) mini-os-$(XEN_TARGET_ARCH)-ioemu/mini-os.gz "$(DESTDIR)$(XENFIRMWAREDIR)/ioemu-stubdom.gz"

install-grub: pv-grub
	$(INSTALL_DIR) "$(DESTDIR)$(XENFIRMWAREDIR)"
	$(INSTALL_DATA) mini-os-$(XEN_TARGET_ARCH)-grub/mini-os.gz "$(DESTDIR)$(XENFIRMWAREDIR)/pv-grub-$(XEN_TARGET_ARCH).gz"

ifneq ($(filter grub,$(STUBDOM_TARGETS)),)
install-grub-if-enabled: install-grub
else
install-grub-if-enabled:
endif

install-c: c-stubdom

install-caml: caml-stubdom

install-xenstore: xenstore-stubdom
	$(INSTALL_DIR) "$(DESTDIR)$(XENFIRMWAREDIR)"
	$(INSTALL_DATA) mini-os-$(XEN_TARGET_ARCH)-xenstore/mini-os.gz "$(DESTDIR)$(XENFIRMWAREDIR)/xenstore-stubdom.gz"

install-xenstorepvh: xenstorepvh-stubdom
	$(INSTALL_DIR) "$(DESTDIR)$(XENFIRMWAREDIR)"
	$(INSTALL_DATA) mini-os-$(XEN_TARGET_ARCH)-xenstorepvh/mini-os.gz "$(DESTDIR)$(XENFIRMWAREDIR)/xenstorepvh-stubdom.gz"

install-vtpm: vtpm-stubdom
	$(INSTALL_DIR) "$(DESTDIR)$(XENFIRMWAREDIR)"
	$(INSTALL_DATA) mini-os-$(XEN_TARGET_ARCH)-vtpm/mini-os.gz "$(DESTDIR)$(XENFIRMWAREDIR)/vtpm-stubdom.gz"

install-vtpmmgr: vtpmmgr-stubdom
	$(INSTALL_DIR) "$(DESTDIR)$(XENFIRMWAREDIR)"
	$(INSTALL_DATA) mini-os-$(XEN_TARGET_ARCH)-vtpmmgr/mini-os.gz "$(DESTDIR)$(XENFIRMWAREDIR)/vtpmmgr-stubdom.gz"

###########
# uninstall
###########

ifeq (x$(STUBDOM_UNINSTALL),x)
STUBDOM_UNINSTALL=$(patsubst install-%,uninstall-%,$(STUBDOM_INSTALL))
endif

ifeq ($(STUBDOM_SUPPORTED),1)
uninstall: $(STUBDOM_UNINSTALL)
else
uninstall:
endif

uninstall-ioemu:
	rm -f $(DESTDIR)$(LIBEXEC_BIN)/stubdom-dm
	rm -f $(DESTDIR)$(LIBEXEC_BIN)/stubdompath.sh
	rm -f $(DESTDIR)$(XENFIRMWAREDIR)/ioemu-stubdom.gz

uninstall-grub:
	rm -f $(DESTDIR)$(XENFIRMWAREDIR)/pv-grub-$(XEN_TARGET_ARCH).gz

uninstall-xenstore:
	rm -f $(DESTDIR)$(XENFIRMWAREDIR)/xenstore-stubdom.gz

uninstall-xenstorepvh:
	rm -f $(DESTDIR)$(XENFIRMWAREDIR)/xenstorepvh-stubdom.gz

uninstall-vtpm:
	rm -f $(DESTDIR)$(XENFIRMWAREDIR)/vtpm-stubdom.gz

uninstall-vtpmmgr:
	rm -f $(DESTDIR)$(XENFIRMWAREDIR)/vtpmmgr-stubdom.gz

#######
# clean
#######

# Only clean the libxc/ioemu/mini-os part
.PHONY: clean
clean: $(foreach lib,$(STUB_LIBS),clean-libxen$(lib))
clean:
	rm -fr mini-os-$(XEN_TARGET_ARCH)-ioemu
	rm -fr mini-os-$(XEN_TARGET_ARCH)-c
	rm -fr mini-os-$(XEN_TARGET_ARCH)-caml
	rm -fr mini-os-$(XEN_TARGET_ARCH)-grub
	rm -fr mini-os-$(XEN_TARGET_ARCH)-xenstore
	rm -fr mini-os-$(XEN_TARGET_ARCH)-xenstorepvh
	rm -fr mini-os-$(XEN_TARGET_ARCH)-vtpm
	rm -fr mini-os-$(XEN_TARGET_ARCH)-vtpmmgr
	$(MAKE) DESTDIR= -C caml clean
	$(MAKE) DESTDIR= -C c clean
	$(MAKE) -C vtpm clean
	$(MAKE) -C vtpmmgr clean
	rm -fr grub-$(XEN_TARGET_ARCH)
	rm -f $(STUBDOMPATH)
	rm -f *-minios-config.mk
	rm -fr pkg-config
	-[ ! -d ioemu ] || $(MAKE) DESTDIR= -C ioemu clean
	-[ ! -d xenstore ] || $(MAKE) -f $(CURDIR)/xenlibs.mk -C xenstore clean
	-[ ! -d xenstorepvh ] || $(MAKE) -f $(CURDIR)/xenlibs.mk -C xenstorepvh clean

# clean the cross-compilation result
.PHONY: crossclean
crossclean: clean
	rm -fr $(CROSS_ROOT)
	rm -fr newlib-$(XEN_TARGET_ARCH)
	rm -fr zlib-$(XEN_TARGET_ARCH) pciutils-$(XEN_TARGET_ARCH)
	rm -fr libs-$(XEN_TARGET_ARCH)
	rm -fr ioemu xenstore xenstorepvh
	rm -fr gmp-$(XEN_TARGET_ARCH)
	rm -fr polarssl-$(XEN_TARGET_ARCH)
	rm -fr tpm_emulator-$(XEN_TARGET_ARCH)
	rm -f mk-headers-$(XEN_TARGET_ARCH)
	rm -fr ocaml-$(XEN_TARGET_ARCH)
	rm -fr include

# clean patched sources
.PHONY: patchclean
patchclean: crossclean
	rm -fr newlib-$(NEWLIB_VERSION)
	rm -fr gmp-$(XEN_TARGET_ARCH)
	rm -fr polarssl-$(XEN_TARGET_ARCH)
	rm -fr tpm_emulator-$(XEN_TARGET_ARCH)
	rm -fr lwip-$(XEN_TARGET_ARCH)
	rm -fr grub-upstream

# clean downloads
.PHONY: downloadclean
downloadclean: patchclean
	rm -f newlib-$(NEWLIB_VERSION).tar.gz
	rm -f zlib-$(ZLIB_VERSION).tar.gz
	rm -f gmp-$(GMP_VERSION).tar.bz2
	rm -f tpm_emulator-$(TPMEMU_VERSION).tar.gz
	rm -f pciutils-$(LIBPCI_VERSION).tar.bz2
	rm -f grub-$(GRUB_VERSION).tar.gz
	rm -f lwip-$(LWIP_VERSION).tar.gz
	rm -f ocaml-$(OCAML_VERSION).tar.gz
	rm -f polarssl-$(POLARSSL_VERSION)-gpl.tgz

.PHONY: distclean
distclean: downloadclean
	rm -rf config.log config.status config.cache autom4te.cache ../config/Stubdom.mk

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
$(XEN_ROOT)/config/Stubdom.mk:
	$(error You have to run ./configure before building or installing stubdom)
endif

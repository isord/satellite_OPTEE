global-incdirs-y += .
srcs-y += main.c
LIBNAME = kyber_test
LIBDIR = $(O)/examples/kyber_simple_test
include $(TA_DEV_KIT_DIR)/../host_lib/host_lib.mk

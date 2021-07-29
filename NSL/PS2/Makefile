# when called with no parameters, this makefile will make the NSLPS2 Lib file
# (nsl.lib).  It can also be used to make the nslTestApp by passing that
# as a parameter (make nslTestApp).  This makefile is very half-assed, so
# if you can't handle that, then make it fully assed and the world will be
# a better place.

SHELL       = /bin/sh

TOP         = /usr/local/sce/ee
LIBDIR      = $(TOP)/lib
INCDIR      = \
	$(TOP)/../common/include \
	-I. \
	-I.. \
	-I$(TOP)/include \
	-I/sm/sgi_stl \
	-I/usr/local/sce/common/include \
	-I$(TOP)/gcc/ee/include

TARGET      = nsl
OBJS        = gas_utility.o \
              nl_ps2.o \
			  ../common/nsl.o \
			  ../common/nsl_common.o \
              nsl_ps2.o \
              nsl_ps2_sound.o \
              nsl_ps2_source.o \
              nsl_ps2_emitter.o

TEST_APP_OBJS = nsl_testapp_sys_ps2.o \
              nsl_testapp.o \
              nsl_test_cases.o

TEST_SIMPLE_OBJS = nsl_testapp_sys_ps2.o \
              nsl_test_simple.o

LCFILE      = $(LIBDIR)/app.cmd
LIBS        = $(LIBDIR)/libcdvd.a
TEST_APP_LIBS = $(TOP)/gcc/ee/lib/libstdc++.a.2.10-ee-001003
# $(TOP)/gcc/lib/gcc-lib/ee/2.9-ee-991111/libstdc++.a

# sntty.lib
 
PREFIX      = ee
AS          = $(PREFIX)-gcc
AR          = $(PREFIX)-ar
RANLIB      = $(PREFIX)-ranlib
CC          = $(PREFIX)-gcc 
LD          = $(PREFIX)-gcc
DVPASM      = $(PREFIX)-dvp-as
OBJDUMP     = $(PREFIX)-objdump
RUN         = dsedb -r run
RM          = /bin/rm -f

CFLAGS      = -g -Wa,-al -fno-common -G0
CXXFLAGS    = -g -Wa,-al -fno-exceptions -fno-common
ASFLAGS     = -c -xassembler-with-cpp -Wa,-al
DVPASMFLAGS = -g 
LDFLAGS     = -Wl,-Map,$(TARGET).map -nostartfiles -L$(LIBDIR) -lpad -lsdr -lm 
TMPFLAGS    = -DTARGET_PS2

ARGS = host0:tr1l_pad.vb host0:tr1r_pad.vb

.SUFFIXES: .cpp .s .cc .dvpasm

all: $(TARGET).lib
	@echo This makefile does not respect header-dependencies.  Someone should add that.
	@echo .

$(TARGET).lib: $(OBJS)
	$(AR) cru $@ $^
	$(RANLIB) $@

nslTestApp: $(TEST_APP_OBJS) $(OBJS) 
	@echo This makefile does not respect header-dependencies.  Someone should add that.
	@echo .
	$(LD) -o $@.elf $(LINK_OPTIONS) -T $(LCFILE) crt0.o $(OBJS) $(TEST_APP_OBJS) $(LIBS) $(TEST_APP_LIBS) $(LDFLAGS)

nslTestSimple: $(TEST_SIMPLE_OBJS) $(OBJS) 
	@echo This makefile does not respect header-dependencies.  Someone should add that.
	@echo .
	$(LD) -o $@.elf $(LINK_OPTIONS) -T $(LCFILE) crt0.o $(OBJS) $(TEST_SIMPLE_OBJS) $(LIBS) $(TEST_APP_LIBS) $(LDFLAGS)

crt0.o: $(LIBDIR)/crt0.s
	$(AS) $(ASFLAGS) $(TMPFLAGS) -o $@ $< > $*.lst

.s.o:
	$(AS) $(ASFLAGS) $(TMPFLAGS) -I$(INCDIR) -o $@ $< > $*.lst

.dvpasm.o:
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $< > $*.lst

.cpp.o:
	$(CC) $(CFLAGS) $(TMPFLAGS) -I$(INCDIR) -c $< -o $*.o > $*.lst

.cc.o:
	$(CC) $(CXXFLAGS) $(TMPFLAGS) -I$(INCDIR) -c $< -o $*.o > $*.lst

run: $(TARGET).elf
	dsreset 0 0; $(RUN) $(TARGET).elf $(ARGS)

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf copy.c incdata

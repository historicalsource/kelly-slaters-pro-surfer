BUILD ?= debug
OBJ_DIR = $(BUILD)

TOP = /usr/local/sce/ee
LIBDIR = $(TOP)/lib
INCDIR = \
	-I$(TOP)/include \
	-I$(TOP)/gcc/ee/include \
	-I$(TOP)/gcc/include/g++-2 \
	-I/usr/local/sce/common/include \
	-I/sm/src/vsim \
	-I/sm/src

TARGET = ../bin/SpiderManPS2_$(BUILD)

CPP_OBJS = \
		hwosps2/krender.o \
		files_ai.o \
		files_hwosps2.o \
		files_anim.o \
		files_entity.o \
		files_misc1.o \
		files_misc2.o \
		files_misfits.o \
		files_script.o \
		files_spiderman.o \
		files_vsim.o \
		hwosps2/ps2_algebra.o \
		collide.o

ASM_OBJS = \
		hwosps2/krvu1.o 

CPP_SRC = \
		$(CPP_OBJS:.o=.cpp)

ASM_SRC = \
		$(ASM_OBJS:.o=.dsm)

OBJS = \
	$(ASM_OBJS) \
	$(CPP_OBJS)

SRC = \
	$(ASM_SRC) \
	$(CPP_SRC)

FULL_PATH_OBJS = $(foreach OBJ, $(OBJS), $(OBJ_DIR)/$(OBJ))

VPATH = %.o $(OBJ_DIR)

LCFILE = $(LIBDIR)/app.cmd
LIBS = \
	$(LIBDIR)/libvu0.a \
	$(LIBDIR)/libpc.a \
	$(LIBDIR)/libsn.a \
	$(LIBDIR)/libgraph.a \
        $(LIBDIR)/libdma.a \
        $(LIBDIR)/libdev.a \
        $(LIBDIR)/libpkt.a \
        $(LIBDIR)/libpad.a \
	$(LIBDIR)/libcdvd.a \
	$(LIBDIR)/libmpeg.a \
	$(LIBDIR)/libipu.a \
	hwosps2/dmadebug.a \
	$(TOP)/gcc/lib/gcc-lib/ee/2.9-ee-991111/libstdc++.a \
	sntty.lib

AS = ps2as
CC = ee-gcc2953
LD = ee-gcc
DVPASM = ps2dvpas
RM = del /f /q

WARNINGS = -Wall -Wno-unknown-pragmas -Wno-non-template-friend -Wpromote-doubles
OPTS = -mvu0-use-vf0-vf31 -fdefault-single-float -ffast-math -funroll-loops
CFLAGS = -G0 -D__PS2_EE__ -DARCH_ENGINE $(WARNINGS) $(OPTS) -fno-common
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions
EXTRA_OPTS = -finline-functions
ASFLAGS = -c -xassembler-with-cpp -Wa,-al
DVPASMFLAGS = -g
LDFLAGS = -L$(LIBDIR) -Wl,-Map,$(TARGET).map -nostartfiles -lm -lsdr
# -lgraph -ldma -ldev -lpkt -lvu0 -lpad  -lmpeg -lipu -lcdvd -lm

TMPFLAGS =

.SUFFIXES: .c .s .cc .dsm

#
# Various targets
#
all: debug

debug: COMPILE_OPTIONS = -g -DDEBUG -DBUILD_DEBUG
debug: $(TARGET).elf

fastdebug: COMPILE_OPTIONS = -g -DBUILD_FASTDEBUG -O2 -fno-strict-aliasing
fastdebug: LINK_OPTIONS = 
fastdebug: $(TARGET).elf

release: COMPILE_OPTIONS = -g -DBUILD_RELEASE -O2 -fno-strict-aliasing
release: LINK_OPTIONS = 
release: $(TARGET).elf

CRAZY_OPTS = -funroll-loops -finline-functions

bootable: COMPILE_OPTIONS = -DNDEBUG -DBUILD_BOOTABLE -O2 -fomit-frame-pointer $(CRAZY_OPTS) -fno-strict-aliasing
bootable: LINK_OPTIONS = -s
bootable: $(TARGET).elf

#
# Special build rules
#
users.h:
		-echo /\* insert custom defines here \*/ > users.h

hwosps2/algebra.o:	hwosps2/algebra.cpp
			ec $(CC) -c $< $(CXXFLAGS) $(COMPILE_OPTIONS) $(TMPFLAGS) $(INCDIR) -o $(OBJ_DIR)/$@

$(TARGET).elf:	users.h $(OBJS) $(LIBS)
		$(LD) -o $@ $(LINK_OPTIONS) -T $(LCFILE) crt0.o $(FULL_PATH_OBJS) $(LIBS) $(LDFLAGS)

#
# Custom generic rules
#
.s.o:
	$(AS) $(ASFLAGS) $(TMPFLAGS) $(INCDIR) -o $(OBJ_DIR)/$@ $< > $*.lst

.c.o:
	$(CC) -c $< $(CFLAGS) $(COMPILE_OPTIONS) $(TMPFLAGS) $(INCDIR) -o $(OBJ_DIR)/$*.o

.cpp.o:
	$(CC) -c $< $(CXXFLAGS) $(COMPILE_OPTIONS) $(TMPFLAGS) $(INCDIR) -o $(OBJ_DIR)/$*.o

#
# Misc
#
depend:
		$(CC) $(INCDIR) $(CXXFLAGS) -M $(CPP_SRC) > makefile.depend

clean:
		-$(RM) $(OBJ_DIR)\*.o
		-copy crt0.o $(OBJ_DIR)
		-$(RM) $(OBJ_DIR)\$(HWOS_DIR)\*.o
		-copy ramfont.o $(OBJ_DIR)\$(HWOS_DIR)
		-copy rfont.o $(OBJ_DIR)\$(HWOS_DIR)
		-$(RM) $(OBJ_DIR)\$(VSIM_DIR)\*.o
		-$(RM) $(TARGET).elf $(TARGET).map $(TARGET).lst $(TARGET)*.dis

-include makefile.depend

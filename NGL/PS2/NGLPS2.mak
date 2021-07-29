#
# NGL PS2 Makefile
#
.SILENT:

BUILD ?= debug

#
# Configurable settings
#
STL_INCDIR = /usr/local/sce/ee/gcc/include/g++-2
GCC_INCDIR = 2.9-ee-991111
GCC = ec ee-gcc2953

#
# Source files
#
SRC += ngl_ps2.cpp
SRC += ngl_dma.cpp
SRC += ngl_ate.cpp
SRC += ngl_instbank.cpp
SRC += ngl_ps2_asm.s
SRC += ngl_vu1.dsm
SRC += ngl_particle.dsm

OBJS = $(addsuffix .o,$(basename $(SRC)))
FULL_PATH_OBJS = $(addprefix $(BUILD)/,$(OBJS))

#
# Compiler settings
#
INCDIR += -I/usr/local/sce/ee/include 
INCDIR += -I/usr/local/sce/common/include 
INCDIR += -I/usr/local/sce/ee/gcc/ee/include 
INCDIR += -I/usr/local/sce/ee/gcc/lib/gcc-lib/ee/$(GCC_INCDIR)
INCDIR += -I/usr/local/sce/ee/gcc/lib/gcc-lib/ee/$(GCC_INCDIR)/include 
INCDIR += -I$(STL_INCDIR) 

ASM_INCDIR = /usr/local/sce/ee/include

#WARNINGS += -W -Wall -Wundef -Wshadow -Wlarger-than-32768 -Wpointer-arith -Wwrite-strings -Winline
#WARNINGS += -Wredundant-decls -Wpromote-doubles -Wsoftware-math-library -Wno-system-headers

WARNINGS = -Wall -Wno-deprecated -Wpointer-arith -Wwrite-strings -Wpromote-doubles 
CPPFLAGS = $(WARNINGS) -mvu0-use-vf0-vf31 -ffast-math -fno-common -fno-rtti -fno-exceptions -G0 -D__PS2_EE__ -DUSERNAME=\"$(USERNAME)\" 

ifeq ($(BUILD),debug)
CPPFLAGS += -g
TARGET = ngl_ps2d.a
endif

ifeq ($(BUILD),release)
CPPFLAGS += -g -O2 -finline-functions -fno-strict-aliasing -funroll-loops  
CPPFLAGS += -DNGL_FINAL
TARGET = ngl_ps2.a
endif				   

#
# Rules
#
$(TARGET): $(FULL_PATH_OBJS)
	echo Linking $(TARGET).
	ec ee-ar -ru $@ $(FULL_PATH_OBJS)

$(BUILD)/%.o : %.cpp
	echo Compiling $*.cpp.
	$(GCC) $(CPPFLAGS) $(INCDIR) -Wa,-ahlsd=$(BUILD)/$*.lst -o$@ -c $<

$(BUILD)/%.o : %.s
	echo Assembling $*.s.
	$(GCC) -xassembler-with-cpp -o$@ -c $<

# ee-gcc is just used for #include and #define, so it's not important what version is run.
$(BUILD)/%.o : %.dsm
	echo Assembling $*.dsm.
	ee-gcc -x c++ -E $< | ps2dvpas -sn -g -o$@ -I$(ASM_INCDIR) --

depend:
	$(GCC) $(INCDIR) -M $(SRC) > makefile.depend

clean:
	del /y ngl_ps2.a
	del /y $(BUILD)/*.*

-include makefile.depend


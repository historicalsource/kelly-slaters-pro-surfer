PROJECT ?= NO_PROJECT

include PathDefs
INCDIR = -Ic:/usr/local/sce/common/include -Ic:/usr/local/sce/iop/include -Ic:/usr/local/sce/iop/install/include
ILIBS	= -Lc:/usr/local/sce/iop/lib -L./ -Lc:/usr/local/sce/iop/install/lib -ilb=libsd.ilb

############## Warning warning, this makefile is half-assed ################
##############   It does not enforce header dependencies.   ################  
############## If you want to add that support, be my guest ################  

CFLAGS  = $(INCDIR) -I. -Wall -D$(PROJECT) -DTARGET_PS2
ASFLAGS = $(INCDIR)
RM      = del
CP      = copy


COMPILE.s = $(CC) -xassembler-with-cpp $(ASFLAGS) $(CPPFLAGS) $(TARGET_MACH) -c

#----------- customize section --------------
PROGNAME = gas

OBJS    = ../../common/nsl.cpp GasRpc.o GasRpcEntry.o GasRpcCommand.o GasRpcCommandSupport.o GasRpcSoundFileParser.o GasRpcUtility.o GasUpdate.o GasUpdateSupport.o iop_fifo_queue.o instr_fifo_queue.o
#ILIBS  = -ilb=$(TOP)lib/iop.ilb 

#----------- rules --------------
all:    debug

debug:  CFLAGS+=-G0 -g -DGAS_MODULE -DDEBUG
debug:  ASFLAGS+=-G0
debug:	$(PROGNAME).irx

chatty:  CFLAGS+=-g -DGAS_MODULE -DDEBUG -DDEBUG_OUTPUT
chatty:  ASFLAGS+=-G0
chatty:	 $(PROGNAME).irx

#
# the -O2 optimizations are untested for the GAS module, 
# so if you build a release IRX, you're treading new ground.
#
release: CFLAGS+=-G0 -O2 -fomit-frame-pointer -DGAS_MODULE
release: ASFLAGS+=-G0
release: $(PROGNAME).irx

install:
	$(CP) $(PROGNAME).irx ..\\..\\..\\data\\
	$(CP) $(PROGNAME).irx ..\\..\\..\\cd\\
	
clean:
	$(RM) *.o $(PROGNAME).irx *.obj *.map

$(PROGNAME).irx: $(OBJS) ../gas.h gasSystem.h iop_fifo_queue.h
	$(LINK.o)  -v -o $@ \
                $(OBJS) $(ILIBS) -ilb=cdvdman.ilb

$(OBJS): ../gas.h

include PathDefs
INCDIR = -Ic:/usr/local/sce/common/include -Ic:/usr/local/sce/iop/include -Ic:/usr/local/sce/iop/install/include
ILIBS	= -Lc:/usr/local/sce/iop/lib -L./ -Lc:/usr/local/sce/iop/install/lib -ilb=libsd.ilb

CFLAGS  = $(INCDIR) -I. -Wall -G0 -g
ASFLAGS = $(INCDIR) -G0
RM      = /bin/rm -f

COMPILE.s = $(CC) -xassembler-with-cpp $(ASFLAGS) $(CPPFLAGS) $(TARGET_MACH) -c

#----------- customize section --------------
PROGNAME = gas

OBJS    = GasRpc.o GasRpcEntry.o GasRpcCommand.o GasRpcCommandSupport.o GasRpcSoundFileParser.o GasRpcUtility.o GasUpdate.o GasUpdateSupport.o iop_fifo_queue.o
#ILIBS  = -ilb=$(TOP)lib/iop.ilb 

#----------- rules --------------
all:	$(PROGNAME).irx

clean:
	rm -f *.o $(PROGNAME).irx *.obj *.map

$(PROGNAME).irx: $(OBJS) ../gas.h gasSystem.h iop_fifo_queue.h
	$(LINK.o)  -o $@ \
                $(OBJS) $(ILIBS) -ilb=cdvdman.ilb

$(OBJS): ../gas.h
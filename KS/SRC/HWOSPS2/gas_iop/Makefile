include PathDefs
INCDIR = -Ic:/usr/local/sce/common/include -Ic:/usr/local/sce/iop/include -Ic:/usr/local/sce/iop/install/include
ILIBS	= -Lc:/usr/local/sce/iop/lib -L./ -Lc:/usr/local/sce/iop/install/lib -ilb=libsd.ilb

CFLAGS  = $(INCDIR) -I. -Wall -G0 -g
ASFLAGS = $(INCDIR) -G0
RM          = /bin/rm -f

COMPILE.s = $(CC) -xassembler-with-cpp $(ASFLAGS) $(CPPFLAGS) $(TARGET_MACH) -c

#----------- customize section --------------
PROGNAME = gas_iop

OBJS     = GasIopDecode.o GasIopSourceSpu.o GasIopSourceIop.o GasIopSourceCd.o GasIopEntry.o GasIopSystem.o GasIopSoundFileParser.o iop_fifo_queue.o
#ILIBS  = -ilb=$(TOP)lib/iop.ilb 

#----------- rules --------------
all:	$(PROGNAME).irx

clean:
	rm -f *.o $(PROGNAME).irx *.obj *.map

$(PROGNAME).irx: $(OBJS) ../gas_iop.h gasIopSystem.h iop_fifo_queue.h
	$(LINK.o)  -o $@ \
                $(OBJS) $(ILIBS) -ilb=cdvdman.ilb

$(OBJS): ../gas_iop.h
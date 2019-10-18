CC=g++

OBJS1=snmprun.o
TARGETS= snmprun

CFLAGS=-I. `net-snmp-config --cflags`
BUILDLIBS=`net-snmp-config --libs`
BUILDAGENTLIBS=`net-snmp-config --agent-libs`

# shared library flags (assumes gcc)
DLFLAGS=-fPIC -shared

all: $(TARGETS)

snmprun: $(OBJS1)
	$(CC) -g -o snmprun $(OBJS1) $(BUILDLIBS)

clean:
	rm -rf $(OBJS1) $(TARGETS)
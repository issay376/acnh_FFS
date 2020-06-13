# Test Code Makefile Template for Open Web Middleware
# 	K.Saitoh, 12/08/2004

include makefile.inc

all : bn bn_lite gr grx ph layout1 layout2 reprod
misc : anm mum tlp glr layout0

bn : bn.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o bn -I${PCTLINC} ${CFLAGS} bn.cpp

bn_lite : bn_lite.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o bn_lite -I${PCTLINC} ${CFLAGS} bn_lite.cpp

gr : gr.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o gr -I${PCTLINC} ${CFLAGS} gr.cpp

grx : grx.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o grx -I${PCTLINC} ${CFLAGS} grx.cpp

ph : ph.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o ph -I${PCTLINC} ${CFLAGS} ph.cpp

layout1 : layout1.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o layout1 -I${PCTLINC} ${CFLAGS} layout1.cpp

layout2 : layout2.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o layout2 -I${PCTLINC} ${CFLAGS} layout2.cpp

reprod : reprod.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o reprod -I${PCTLINC} ${CFLAGS} reprod.cpp

layout0 : misc/layout0.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o layout0 -I${PCTLINC} ${CFLAGS} misc/layout0.cpp

anm : misc/anm.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o anm -I${PCTLINC} ${CFLAGS} misc/anm.cpp

mum : misc/mum.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o mum -I${PCTLINC} ${CFLAGS} misc/mum.cpp

tlp : misc/tlp.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o tlp -I${PCTLINC} ${CFLAGS} misc/tlp.cpp

glr : misc/glr.cpp\
	${PCTLINC}/acnh.h\
	${PCTLINC}/pctl/pcontainer.h\
	${PCTLINC}/pctl/plist.h
	${CCPP} -o glr -I${PCTLINC} ${CFLAGS} misc/glr.cpp

test : bn 
	./bn

clean :
	${RM} -f *.o
	${RM} -f bn
	${RM} -f bn_lite
	${RM} -f gr
	${RM} -f grx
	${RM} -f ph 
	${RM} -f layout1
	${RM} -f layout2
	${RM} -f reprod 
	${RM} -f anm
	${RM} -f mum
	${RM} -f tlp
	${RM} -f glr
	${RM} -f layout0


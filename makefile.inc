# Makefile definitions
#
# --- Environment Depends -->
PCTLDIR = .
#INCDIR = /usr/local/include
#LIBDIR = /usr/local/lib
# <-------------------------

PCTLSRC = ${PCTLDIR}/src
PCTLINC = ${PCTLDIR}/include
PCTLLIB = ${PCTLDIR}/lib
PCTLBIN = ${PCTLDIR}/bin

# CCPP = g++
CCPP = clang++

CFLAGS = -ggdb -D_DEBUG -Wno-parentheses -std=c++14 -D__cpp_lib_transformation_trait_aliases

CP = cp
RM = rm -f
AR = ar -cr

.cpp.o :
	${CCPP} ${CFLAGS} -I${PCTLINC} -c $<


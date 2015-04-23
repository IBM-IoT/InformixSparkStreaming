# This Makefile builds the boundingbox DataBlade.


# This make file assumes a directory structure  that is  similar to
# the directory structure in which the source files were originally
# generated by BladeSmith.  This is:
#
#                              src  <- the makefile goes here
#                              /\
#                       ActiveX  c
#
# Because the directory structure is the same,  files can be copied
# from NT to UNIX (and back) by first NFS  mounting  your UNIX file
# system and then using Windows NT Explorer to copy the files.

# =================================================================


# TARGET must be set to the location/filename
# of the platform-specific make include file.
# export TARGET=$INFORMIXDIR/incl/dbdk/makeinc.<platform>
include $(TARGET)

#OS_NAME  = linux
#PLATFORM = x86
#OBJSUFF = o
#BLDLIB_SUFF = bld
#SHLIBLOD = ld
#SYMFLAG =
#SHLIBLFLAG = -shared -m32
#CC_PIC = -fPIC


# This is the project title.
PROJECT_TITLE = boundingbox

# The linked DataBlade module is placed here.
BINDIR = $(OS_NAME)-$(PLATFORM)
BINDIR = output

# Platform independent code goes here.
# The following code was generated by BladeSmith.

CC        = gcc
MI_INCL   = $(INFORMIXDIR)/incl
CFLAGS    = -DMI_SERVBUILD $(CC_PIC) -I$(MI_INCL)/public -I$(MI_INCL)/esql -I$(MI_INCL) $(COPTS)
LINKFLAGS = $(SHLIBLFLAG) $(SYMFLAG)
LIBS      =
# Add in -m32 in CFLAGS and LINKFLAGS if necessary
CFLAGS    = -DMI_SERVBUILD -fPIC -I/opt/ifmx/incl/public -I/opt/ifmx/incl/esql -I/opt/ifmx/incl -g
LINKFLAGS = -shared -Bsymbolic

# This is a list of the C object files.
PROJECTC_OBJS = \
	$(BINDIR)/support.$(OBJSUFF) \
	$(BINDIR)/udr.$(OBJSUFF) \
	$(BINDIR)/boundingbox.$(OBJSUFF)

# This is a list of the ActiveX server object files.
PROJECTX_OBJS = \

PROJECT_LIBS = $(BINDIR)/$(PROJECT_TITLE).$(BLDLIB_SUFF)

all : $(BINDIR)
	$(MAKE) $(MAKEFLAGS) -f $(PROJECT_TITLE)U.mak server $(BUILD_TARGET)

# Construct each object file.

$(BINDIR)/support.$(OBJSUFF) : c/support.c
	$(CC) $(CFLAGS) -o $@ -c $?

$(BINDIR)/udr.$(OBJSUFF) : c/udr.c
	$(CC) $(CFLAGS) -o $@ -c $?

$(BINDIR)/boundingbox.$(OBJSUFF) : c/boundingbox.c
	$(CC) $(CFLAGS) -o $@ -c $?

c/udr.c :  c/$(PROJECT_TITLE).h

c/boundingbox.c :  c/$(PROJECT_TITLE).h


# Construct the shared library.
# Do *NOT* link with client side  libraries.  You  will  see  many
# undefined symbols during linking.  This  is  normal  since those
# symbols are resolved when the server loads  your  shared object.
#
# ATTENTION:
# The  ld  "Symbol referencing errors"  warning  is  normal. These
# unresolved symbols are resolved when the server loads the shared
# object.  This list should be examined, however, for symbol names
# that may have been inadvertently  misspelled.  Misspelled symbol
# names will not be resolved here or at  load  time.  If a version
# 9.20 Informix Server is installed, these  symbols  are  filtered
# by the filtersym.sh script.
#
$(PROJECT_LIBS) : $(PROJECTC_OBJS) $(PROJECTX_OBJS)
	$(SHLIBLOD) $(LINKFLAGS) -o $(PROJECT_LIBS)		\
	$(PROJECTC_OBJS) $(PROJECTX_OBJS) $(LIBS)   		\
	$(DATABLADE_LIBS) 2> link.errs
	if test -x $(INFORMIXDIR)/bin/filtersym.sh ;		\
	then $(INFORMIXDIR)/bin/filtersym.sh link.errs ;	\
	else cat link.errs ;                        		\
	fi


server :	$(PROJECT_LIBS)


clean :
	$(RM) $(RMFLAGS) $(PROJECT_LIBS) $(PROJECTC_OBJS) $(PROJECTX_OBJS)


$(BINDIR) :
	-mkdir $(BINDIR)

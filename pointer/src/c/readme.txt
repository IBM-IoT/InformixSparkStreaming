========================================================================
                     DBDK DataBlade Project boundingbox
========================================================================


BladeSmith Version 4.00.TC1B has created the boundingbox
DataBlade module for you. This application not only demonstrates the
basics of using an Informix DataBlade but is also a starting point for
writing your DataBlade module application.

This file contains a summary of what you will find in each of the files
that make up your boundingbox DataBlade module.

boundingbox.h

	This include file contains various definitions along with the
	'C' definitions of all your opaque types.

boundingbox.c

	This file contains the interface functions of the
	boundingbox DataBlade module for the opaque
	type BoundingBox.

udr.c

	This file contains all the User Defined Routines,
	Iterators and Aggregates of the boundingbox DataBlade module.

support.c

	This file contains all the support functions that are required
	for the boundingbox DataBlade module.

readme.txt

	This file.

========================================================================

Building your DataBlade module
==============================

To build your DataBlade module on UNIX, first transport the generated
files to your UNIX machine.  Be sure to copy in ASCII mode when using
ftp.  On your UNIX machine, the TARGET environment variable must be set
to the location/filename of the platform-specific make include file. For
Solaris, this is "$INFORMIXDIR/incl/dbdk/makeinc.solaris".

boundingboxU.mak is the makefile used to build the DataBlade
module under UNIX.  To build the DataBlade module, type:

               make -f boundingbox.mak

Documentation
=============

The following is a list of documents containing additional information
about DataBlade module programming.

	for procedures on creating DataBlade modules:

		DataBlade Developers Kit User's Guide

	for C coding:

		DataBlade API Programmer's Manual

	for using BladeManager:

		DataBlade Module Installation and Registration Guide

	for understanding opaque data types:

		Extending INFORMIX-Universal Server: Data Types

	for understanding User-Defined Routines:

		Extending INFORMIX-Universal Server: User-Defined Routines

	for SQL use:
		Informix Guide to SQL: Syntax
		Informix Guide to SQL: Tutorial
		Informix Guide to SQL: Reference


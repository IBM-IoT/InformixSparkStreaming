#!/bin/sh

DB="bbox_testdb"
BLADEFILE="boundingbox"
BLADE="boundingbox.0.0"
BLADEDIR="$INFORMIXDIR/extend/$BLADE"
TBL="bbox_tbl"
IDX="bbox_idx"
OUTDIR="src/output"
export TARGET=/opt/ifmx/incl/dbdk/makeinc.linux86_64

# Check if script is being run as 'informix' user
if [ "$USER" != "informix" ]
then
 echo "must be run as user 'informix'"
 exit
fi 

# Create the output folder for the build if it does not exist
if [ -d "$OUTDIR" ]
then
  echo "$OUTDIR exists"
else
  echo "Creating $OUTDIR"
  mkdir -p $OUTDIR
fi 

# Check if the blade has already been built
if [ -e "$OUTDIR/$BLADEFILE.bld" ]
then
  echo "$OUTDIR/$BLADEFILE.bld exists"
else
  echo "$OUTDIR/$BLADEFILE.bld does not exist."
  echo "Building $BLADEFILE"
  cd 'src'
  make -f boundingboxU.mak
  cd ..
fi

# Check to see if the blade install directory exists
if [ -d "$INFORMIXDIR/extend/$BLADE" ]
then
  echo "$INFORMIXDIR/extend/$BLADE exists"
else
  echo "Creating blade install directory '$INFORMIXDIR/extend/$BLADE'"
  mkdir -p $BLADEDIR
fi

# Check to see if the blade object has been copied in BLADEDIR
if [ -e "$BLADEDIR/$BLADEFILE.bld" ]
then
  echo "$BLADEDIR/$BLADEFILE.bld exists"
else
  echo "Copying blade object code to to: '$BLADEDIR/$BLADEFILE.bld'"
  cp $OUTDIR/$BLADEFILE.bld $BLADEDIR
fi

# Check to see if the objects script has been installed in BLADEDIR
if [ -e "$BLADEDIR/objects.sql" ]
then
  echo "scripts exist in $BLADEDIR"
else
  echo "Copying scripts to $BLADEDIR"
  cp scripts/* $BLADEDIR
fi


# Drop and recreate the database 'bbox_testdb'
echo -e "DROP DATABASE $DB;" | dbaccess -e - 
echo -e "CREATE DATABASE $DB with log;" | dbaccess -e -
# Register the datablade
blademgr <<!
list $DB
register $BLADE $DB
Y
1
list $DB
quit
!
echo -e "list $DB" | blademgr

# Define the trace class 'BoundingBox'
dbaccess -e $DB <<!
INSERT INTO informix.systraceclasses(name) VALUES ('BoundingBox');
CREATE PROCEDURE traceset(LVARCHAR, INTEGER) EXTERNAL NAME '$INFORMIXDIR/extend/$BLADE/boundingbox.bld(BoundingBoxTraceSet)' LANGUAGE C;
!
# Enable tracing
echo -e "EXECUTE PROCEDURE TRACESET('/tmp/boundingbox.trc', 40);" | dbaccess -e $DB

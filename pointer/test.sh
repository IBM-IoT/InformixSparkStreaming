#!/bin/sh

DB="bbox_testdb"
TBL="bbox_tbl"
IDX="bbox_idx"

# Check if script is being run as 'informix' user
if [ "$USER" != "informix" ]
then
 echo "must be run as user 'informix'"
 exit
fi 

# Create a new table with a BoundingBox UDT column 'bbox'
echo -e "create table $TBL (id serial, bbox BoundingBox);" | dbaccess -e $DB
# Create an index on the BoundingBox UDT column
echo -e "create index $IDX on $TBL (bbox BoundingBox_ops) using rtree;" | dbaccess -e $DB
# Update statistics on an empty table succeeds
echo -e "update statistics for table $TBL;" | dbaccess -e $DB

# Insert one row into the table 
echo -e "insert into $TBL values (0, '(0.0 0.0 0.0 1.0 1.0 1.0)')" | dbaccess -e $DB
# Update statistics on a table with one row succeeds
echo -e "update statistics for table $TBL;" | dbaccess -e $DB
#
# Insert three more rows
# This forms a "box of unit boxes"
# (-1,1)     (1,1)
#    *----*----*
#    |    |    |
#    |    |    |
#    *----*----*
#    |    |    |
#    |    |    |
#    *----*----*
# (-1,-1)    (1,-1)
#
dbaccess -e $DB <<!
insert into $TBL values (0, '(0.0 0.0 0.0 -1.0  1.0 1.0)');
insert into $TBL values (0, '(0.0 0.0 0.0 -1.0 -1.0 1.0)');
insert into $TBL values (0, '(0.0 0.0 0.0  1.0 -1.0 1.0)');
SELECT * FROM $TBL;
select * from bbox_tbl where overlap(bbox, '(-.1 -.1 .1 .1 .1 .1)');
!
# Update statistics on a table with more than one row fails 
echo -e "update statistics for table $TBL;" | dbaccess -e $DB

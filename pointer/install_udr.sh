DB="bbox_testdb"
OUTDIR="$PWD/src/output"

dbaccess <<!
database bbox_testdb;

create opclass BoundingBox_ops for rtree
  strategies(Overlap,Equal,Contains,Within)
  support(Union,Size,Inter);

create opaque type BoundingBox(internallength = 48, alignment = 8);
grant usage on type BoundingBox to public;

create function BoundingBoxInput(lvarchar)
    returns BoundingBox
    external name '$OUTDIR/boundingbox.bld(BoundingBoxInput)'
    language C not variant;

create function BoundingBoxOutput(BoundingBox)
    returns lvarchar
    external name '$OUTDIR/boundingbox.bld(BoundingBoxOutput)'
    language C not variant;

create implicit cast (lvarchar as BoundingBox with BoundingBoxInput);
create explicit cast (BoundingBox as lvarchar with BoundingBoxOutput);

create function overlap(BoundingBox, BoundingBox)
    returns boolean
    external name '$OUTDIR/boundingbox.bld(BoundingBoxOverlap)'
    language C not variant;

create function equal(BoundingBox, BoundingBox)
    returns boolean
    external name '$OUTDIR/boundingbox.bld(BoundingBoxEqual)'
    language C not variant;

create function contains(BoundingBox, BoundingBox)
    returns boolean
    with (commutator = within)
    external name '$OUTDIR/boundingbox.bld(BoundingBoxContains)'
    language C not variant;

create function within(BoundingBox, BoundingBox)
    returns boolean
    with (commutator = contains)
    external name '$OUTDIR/boundingbox.bld(BoundingBoxWithin)'
    language C not variant;

create function union(BoundingBox, BoundingBox, BoundingBox)
    returns integer
    external name '$OUTDIR/boundingbox.bld(BoundingBoxUnion)'
    language C not variant;

create function size(BoundingBox, float)
    returns integer
    external name '$OUTDIR/boundingbox.bld(BoundingBoxSize)'
    language C not variant;

create function inter(BoundingBox, BoundingBox, BoundingBox)
    returns integer
    external name '$OUTDIR/boundingbox.bld(BoundingBoxInter)'
    language C not variant;
!

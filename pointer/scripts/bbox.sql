database bbox_testdb;

create opclass BoundingBox_ops for rtree
  strategies(Overlap,Equal,Contains,Within)
  support(Union,Size,Inter);

create opaque type BoundingBox(internallength = 48, alignment = 8);
grant usage on type BoundingBox to public;

create function BoundingBoxInput(lvarchar)
    returns BoundingBox
    external name '/opt/ifmx/extend/boundingbox.0.0/boundingbox.bld(BoundingBoxInput)'
    language C not variant;

create function BoundingBoxOutput(BoundingBox)
    returns lvarchar
    external name '/opt/ifmx/extend/boundingbox.0.0/boundingbox.bld(BoundingBoxOutput)'
    language C not variant;

create implicit cast (lvarchar as BoundingBox with BoundingBoxInput);
create explicit cast (BoundingBox as lvarchar with BoundingBoxOutput);

create function overlap(BoundingBox, BoundingBox)
    returns boolean
    external name '/opt/ifmx/extend/boundingbox.0.0/boundingbox.bld(BoundingBoxOverlap)'
    language C not variant;

create function equal(BoundingBox, BoundingBox)
    returns boolean
    external name '/opt/ifmx/extend/boundingbox.0.0/boundingbox.bld(BoundingBoxEqual)'
    language C not variant;

create function contains(BoundingBox, BoundingBox)
    returns boolean
    with (commutator = within)
    external name '/opt/ifmx/extend/boundingbox.0.0/boundingbox.bld(BoundingBoxContains)'
    language C not variant;

create function within(BoundingBox, BoundingBox)
    returns boolean
    with (commutator = contains)
    external name '/opt/ifmx/extend/boundingbox.0.0/boundingbox.bld(BoundingBoxWithin)'
    language C not variant;

create function union(BoundingBox, BoundingBox, BoundingBox)
    returns integer
    external name '/opt/ifmx/extend/boundingbox.0.0/boundingbox.bld(BoundingBoxUnion)'
    language C not variant;

create function size(BoundingBox, float)
    returns integer
    external name '/opt/ifmx/extend/boundingbox.0.0/boundingbox.bld(BoundingBoxSize)'
    language C not variant;

create function inter(BoundingBox, BoundingBox, BoundingBox)
    returns integer
    external name '/opt/ifmx/extend/boundingbox.0.0/boundingbox.bld(BoundingBoxInter)'
    language C not variant;



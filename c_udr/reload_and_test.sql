drop database if exists test42;
create database test42;

DROP INDEX IF EXISTS ix1;
DROP TABLE IF EXISTS vii;
DROP ACCESS_METHOD if exists vii_am4 restrict;
DROP OPCLASS if exists abs_btree_ops3 restrict;

drop cast if exists (pointer as sendrecv);
drop cast if exists (lvarchar as pointer);
drop cast if exists (pointer as lvarchar);
drop function if exists PointerSend(pointer);
drop function if exists PointerInput(lvarchar);
drop function if exists PointerOutput(pointer);
drop function if exists tachyonCreate(pointer);
drop function if exists tachyonOpen(pointer);
drop function if exists tachyonClose(pointer);
drop function if exists tachyonDrop(pointer);
drop function if exists tachyonGetById(pointer);
drop function if exists tachyonGetNext(pointer,pointer,pointer);
drop function if exists tachyonInsert(pointer,pointer,pointer);
drop function if exists tachyonUpdate(pointer,pointer,pointer,pointer,pointer);
drop function if exists tachyonDelete(pointer,pointer,pointer);
drop function if exists tachyonEndscan(pointer);
drop function if exists tachyonBeginscan(pointer);

create function tachyonCreate(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonCreate)'
    language C;

create function tachyonDrop(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonDrop)'
    language C;

create function tachyonOpen(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonOpen)'
    language C;

create function tachyonClose(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonClose)'
    language C;

create function tachyonGetById(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonGetById)'
    language C;

create function tachyonGetNext(pointer,pointer,pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonGetNext)'
    language C;

create function tachyonInsert(pointer,pointer,pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonInsert)'
    language C;

create function tachyonBeginscan(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonBeginscan)'
    language C;

create function tachyonEndscan(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonEndscan)'
    language C;

create function tachyonDelete(pointer,pointer,pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonDelete)'
    language C;

create function tachyonUpdate(pointer,pointer,pointer,pointer,pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/c_udr.so(tachyonUpdate)'
    language C;

create function PointerSend(pointer)
    returns sendrecv
    external name '/opt/informix/extend/c_udr.so(PointerSend)'
    language C not variant;


create function PointerInput(lvarchar)
    returns Pointer
    external name '/opt/informix/extend/c_udr.so(PointerInput)'
    language C not variant;

create function PointerOutput(pointer)
    returns lvarchar
    external name '/opt/informix/extend/c_udr.so(PointerOutput)'
    language C not variant;

CREATE SECONDARY ACCESS_METHOD vii_am4
(AM_OPEN = tachyonOpen,
AM_CLOSE = tachyonClose,
AM_CREATE = tachyonCreate,
AM_DROP = tachyonDrop,
AM_BEGINSCAN = tachyonBeginscan,
AM_GETNEXT = tachyonGetNext,
AM_ENDSCAN = tachyonEndscan,
AM_INSERT = tachyonInsert,
AM_DELETE = tachyonDelete,
AM_UPDATE = tachyonUpdate,
AM_SPTYPE = 'A');

CREATE OPCLASS abs_btree_ops3 FOR vii_am4
   STRATEGIES (abs_lt, abs_lte, abs_eq, abs_gte, abs_gt)
   SUPPORT (abs_cmp);

ALTER ACCESS_METHOD vii_am4
ADD AM_DEFOPCLASS = abs_btree_ops3;

create table vii(col1 INTEGER,col2 INTEGER);
create index ix1 on vii(col1, col2) USING vii_am4;

insert into vii(col1, col2) values (12456756, 435345);
--insert into vii(col1, col2) values (784566, 8768);

UPDATE vii SET col1 = 99999, col2 = 11111 WHERE col1 = 12456756;

select * from vii;

close database;
drop database test42;


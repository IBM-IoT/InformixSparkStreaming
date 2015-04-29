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

DROP TABLE IF EXISTS vt;
DROP ACCESS_METHOD if exists vtam2 restrict;



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

create function PointerSend(pointer)
    returns sendrecv
    external name '/opt/informix/extend/c_udr.so(PointerSend)'
    language C not variant;
create explicit cast (pointer as sendrecv with PointerSend);

create function PointerInput(lvarchar)
    returns Pointer
    external name '/opt/informix/extend/c_udr.so(PointerInput)'
    language C not variant;

create function PointerOutput(pointer)
    returns lvarchar
    external name '/opt/informix/extend/c_udr.so(PointerOutput)'
    language C not variant;

create implicit cast (lvarchar as pointer with PointerInput);
create explicit cast (pointer as lvarchar with PointerOutput);

CREATE PRIMARY ACCESS_METHOD vtam2
(AM_OPEN = tachyonOpen,
AM_CLOSE = tachyonClose,
AM_CREATE = tachyonCreate,
AM_DROP = tachyonDrop,
AM_GETNEXT = tachyonGetNext,
AM_INSERT = tachyonInsert,
AM_GETBYID = tachyonGetByid,
AM_READWRITE,
AM_ROWIDS,
AM_SPTYPE = 'X',
AM_CLUSTER);

create table vt(col1 INTEGER,
col2 INTEGER) using vtam2;

execute function pointerinput('0x00001234');
execute function pointerinput('0x00001234');
execute function pointerinput('0x00001234');
execute function pointerinput('0x1234abcd');

execute function tachyonInsert('0x1234abcd', '0x1234abcd', '0x1234abcd');


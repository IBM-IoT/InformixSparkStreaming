-- Create the UDRs that we will use to create the seconary access method.
-- TODO Change names to something other than tachyon, that is left over from our Tachyon experimentation
-- TODO Change "/opt/informix" to an environment variable that points to the Informix base directory instead

create function tachyonCreate(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonCreate)'
    language C;

create function tachyonDrop(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonDrop)'
    language C;

create function tachyonOpen(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonOpen)'
    language C;

create function tachyonClose(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonClose)'
    language C;

create function tachyonGetById(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonGetById)'
    language C;

create function tachyonGetNext(pointer,pointer,pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonGetNext)'
    language C;

create function tachyonInsert(pointer,pointer,pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonInsert)'
    language C;

create function tachyonBeginscan(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonBeginscan)'
    language C;

create function tachyonEndscan(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonEndscan)'
    language C;

create function tachyonDelete(pointer,pointer,pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonDelete)'
    language C;

create function tachyonUpdate(pointer,pointer,pointer,pointer,pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(tachyonUpdate)'
    language C;

create function PointerSend(pointer)
    returns sendrecv
    external name '/opt/informix/extend/informix_socket_streaming.so(PointerSend)'
    language C not variant;


create function PointerInput(lvarchar)
    returns Pointer
    external name '/opt/informix/extend/informix_socket_streaming.so(PointerInput)'
    language C not variant;

create function PointerOutput(pointer)
    returns lvarchar
    external name '/opt/informix/extend/informix_socket_streaming.so(PointerOutput)'
    language C not variant;

-- Now create the secondary access method (the virtual index)

CREATE SECONDARY ACCESS_METHOD informix_socket_streaming
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

-- TODO I don't recall that the stuff below does. Is it needed?

CREATE OPCLASS abs_btree_ops3 FOR vii_am4
   STRATEGIES (abs_lt, abs_lte, abs_eq, abs_gte, abs_gt)
   SUPPORT (abs_cmp);

ALTER ACCESS_METHOD vii_am4
ADD AM_DEFOPCLASS = abs_btree_ops3;

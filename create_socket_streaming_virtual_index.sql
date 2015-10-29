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


-- We must define a default operator class for our custom access method. Here it is simply given a junk one.
CREATE OPCLASS junk_operator_class FOR informix_socket_streaming
   STRATEGIES (abs_lt, abs_lte, abs_eq, abs_gte, abs_gt)
   SUPPORT (abs_cmp);

ALTER ACCESS_METHOD informix_socket_streaming
ADD AM_DEFOPCLASS = junk_operator_class;
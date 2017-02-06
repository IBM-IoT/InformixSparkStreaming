-- This file defines the Virtual Index Interface and all its functions in a database
-- To use it you will need to create a database, or use an existing one, then run the following command:
-- dbaccess <db_name> setup.sql

-- First create all the functions the Virtual Index Interface will use, referencing the external shared library

create function iss_am_create(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_create)'
    language C;

create function iss_am_drop(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_drop)'
    language C;

create function iss_am_open(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_open)'
    language C;

create function iss_am_close(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_close)'
    language C;

create function iss_am_getnext(pointer, pointer, pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_getnext)'
    language C;

create function iss_am_insert(pointer,pointer, pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_insert)'
    language C;

create function iss_am_beginscan(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_beginscan)'
    language C;

create function iss_am_endscan(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_endscan)'
    language C;

create function iss_am_delete(pointer, pointer, pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_delete)'
    language C;

create function iss_am_update(pointer, pointer, pointer, pointer, pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_update)'
    language C;

create function iss_am_scancost(pointer, pointer)
    RETURNING pointer
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_scancost)'
    language C;

create function iss_am_truncate(pointer)
    RETURNING pointer
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_truncate)'
    language C;


-- Next, create the Secondary Access Method (the Virtual Index Interface)

CREATE SECONDARY ACCESS_METHOD informix_socket_streaming
(AM_OPEN = iss_am_open,
AM_CLOSE = iss_am_close,
AM_CREATE = iss_am_create,
AM_DROP = iss_am_drop,
AM_BEGINSCAN = iss_am_beginscan,
AM_GETNEXT = iss_am_getnext,
AM_ENDSCAN = iss_am_endscan,
AM_INSERT = iss_am_insert,
AM_DELETE = iss_am_delete,
AM_UPDATE = iss_am_update,
AM_SCANCOST = iss_am_scancost,
AM_TRUNCATE = iss_am_truncate,
AM_SPTYPE = 'A');

-- Finally, create a dummy operator class and set it as the Virtual Index Interface's default.
-- The operator class will never be used, but it is required to have a default one

CREATE OPCLASS iss_dummy_operator_class FOR informix_socket_streaming
   STRATEGIES (abs_lt, abs_lte, abs_eq, abs_gte, abs_gt)
   SUPPORT (abs_cmp);

ALTER ACCESS_METHOD informix_socket_streaming
ADD AM_DEFOPCLASS = iss_dummy_operator_class;

-- The Virtual Index Interface should now be ready to be used

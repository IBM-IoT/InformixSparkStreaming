-- Create the UDRs that we will use to create the secondary access method.
-- TODO Change names to something other than tachyon, that is left over from our Tachyon experimentation
-- TODO Change "/opt/informix" to an environment variable that points to the Informix base directory instead

create function am_create(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '$INFORMIXDIR/extend/informix_socket_streaming.so(am_create)'
    language C;

create function am_drop(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(am_drop)'
    language C;

create function am_open(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(am_open)'
    language C;

create function am_close(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(am_close)'
    language C;

create function am_getnext(pointer, pointer, pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(am_getnext)'
    language C;

create function am_insert(pointer,pointer, pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(am_insert)'
    language C;

create function am_beginscan(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(am_beginscan)'
    language C;

create function am_endscan(pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(am_endscan)'
    language C;

create function am_delete(pointer, pointer, pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(am_delete)'
    language C;

create function am_update(pointer, pointer, pointer, pointer, pointer)
    RETURNING INTEGER
    with (not variant, parallelizable)
    external name '/opt/informix/extend/informix_socket_streaming.so(am_update)'
    language C;


-- Now create the secondary access method (the virtual index)

CREATE SECONDARY ACCESS_METHOD informix_socket_streaming
(AM_OPEN = am_open,
AM_CLOSE = am_close,
AM_CREATE = am_create,
AM_DROP = am_drop,
AM_BEGINSCAN = am_beginscan,
AM_GETNEXT = am_getnext,
AM_ENDSCAN = am_endscan,
AM_INSERT = am_insert,
AM_DELETE = am_delete,
AM_UPDATE = am_update,
AM_SPTYPE = 'A');


-- We must define a default operator class for our custom access method. Here it is simply given a junk one.
CREATE OPCLASS junk_operator_class FOR informix_socket_streaming
   STRATEGIES (abs_lt, abs_lte, abs_eq, abs_gte, abs_gt)
   SUPPORT (abs_cmp);

ALTER ACCESS_METHOD informix_socket_streaming
ADD AM_DEFOPCLASS = junk_operator_class;
-- Remove all access methods and UDRs used by the Informix Socket Streamer.

DROP ACCESS_METHOD if exists informix_socket_streaming restrict;
DROP OPCLASS if exists junk_operator_class restrict;

drop function if exists am_create(pointer);
drop function if exists am_drop(pointer);
drop function if exists am_close(pointer);
drop function if exists am_drop(pointer);
drop function if exists am_getnext(pointer,pointer,pointer);
drop function if exists am_insert(pointer,pointer,pointer);
drop function if exists am_update(pointer,pointer,pointer,pointer,pointer);
drop function if exists am_delete(pointer,pointer,pointer);
drop function if exists am_endscan(pointer);
drop function if exists am_beginscan(pointer);

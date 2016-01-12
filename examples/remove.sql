-- To remove the Virtual Index Interface from a database, first remove any indexes created
-- then remove the access method, operator class, and all the relevant functions

DROP ACCESS_METHOD if exists informix_socket_streaming restrict;
DROP OPCLASS if exists iss_dummy_operator_class restrict;

drop function if exists iss_am_create(pointer);
drop function if exists iss_am_drop(pointer);
drop function if exists iss_am_open(pointer);
drop function if exists iss_am_close(pointer);
drop function if exists iss_am_getnext(pointer,pointer,pointer);
drop function if exists iss_am_insert(pointer,pointer,pointer);
drop function if exists iss_am_update(pointer,pointer,pointer,pointer,pointer);
drop function if exists iss_am_delete(pointer,pointer,pointer);
drop function if exists iss_am_endscan(pointer);
drop function if exists iss_am_beginscan(pointer);

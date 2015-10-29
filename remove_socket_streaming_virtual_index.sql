-- Remove all access methods and UDRs used by the Informix Socket Streamer.

DROP ACCESS_METHOD if exists informix_socket_streaming restrict;
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

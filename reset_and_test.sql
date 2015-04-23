DROP TABLE IF EXISTS vt;
DROP ACCESS_METHOD if exists vtam restrict;
DROP FUNCTION disconnectFromTachyon(pointer);
DROP FUNCTION connectToTachyon(pointer);
DROP FUNCTION createTableInTachyon(pointer);
DROP FUNCTION deleteTableInTachyon(pointer);
DROP FUNCTION beginScanTachyon(pointer);
DROP FUNCTION getNextTachyon(pointer, pointer, pointer);
DROP FUNCTION endScanTachyon(pointer);
DROP FUNCTION insertTachyon(pointer, pointer, pointer);
DROP FUNCTION getByte(pointer);

EXECUTE PROCEDURE sqlj.remove_jar("informix_tachyon");

EXECUTE PROCEDURE sqlj.install_jar(
"file:$INFORMIXDIR/tmp/tachyon_vti_interface.jar","informix_tachyon", 0);

CREATE FUNCTION disconnectFromTachyon(pointer)
RETURNING integer
 WITH (class='jvp', NOT VARIANT, PARALLELIZABLE)
EXTERNAL NAME 'informix_tachyon:com.ibm.TachyonVtiInterface.disconnectFromTachyon(com.ibm.Pointer)'
LANGUAGE java;

CREATE FUNCTION connectToTachyon(pointer)
RETURNING integer
 WITH (class='jvp', NOT VARIANT, PARALLELIZABLE)
EXTERNAL NAME 'informix_tachyon:com.ibm.TachyonVtiInterface.connectToTachyon(com.ibm.Pointer)'
LANGUAGE java;

CREATE FUNCTION createTableInTachyon(pointer)
RETURNING integer
 WITH (class='jvp', NOT VARIANT, PARALLELIZABLE)
EXTERNAL NAME 'informix_tachyon:com.ibm.TachyonVtiInterface.createTableInTachyon(com.ibm.Pointer)'
LANGUAGE java;

CREATE FUNCTION deleteTableInTachyon(pointer)
RETURNING integer
 WITH (class='jvp', NOT VARIANT, PARALLELIZABLE)
EXTERNAL NAME 'informix_tachyon:com.ibm.TachyonVtiInterface.deleteTableInTachyon(com.ibm.Pointer)'
LANGUAGE java;

CREATE FUNCTION beginScanTachyon(pointer)
RETURNING integer
 WITH (class='jvp', NOT VARIANT, PARALLELIZABLE)
EXTERNAL NAME 'informix_tachyon:com.ibm.TachyonVtiInterface.beginScanTachyon(com.ibm.Pointer)'
LANGUAGE java;

CREATE FUNCTION getNextTachyon(pointer, pointer, pointer)
RETURNING integer
 WITH (class='jvp', NOT VARIANT, PARALLELIZABLE)
EXTERNAL NAME 'informix_tachyon:com.ibm.TachyonVtiInterface.getNextTachyon(com.ibm.Pointer, com.ibm.Pointer, com.ibm.Pointer)'
LANGUAGE java;

CREATE FUNCTION endScanTachyon(pointer)
RETURNING integer
 WITH (class='jvp', NOT VARIANT, PARALLELIZABLE)
EXTERNAL NAME 'informix_tachyon:com.ibm.TachyonVtiInterface.endScanTachyon(com.ibm.Pointer)'
LANGUAGE java;

CREATE FUNCTION insertTachyon(pointer, pointer, pointer)
RETURNING integer
 WITH (class='jvp', NOT VARIANT, PARALLELIZABLE)
EXTERNAL NAME 'informix_tachyon:com.ibm.TachyonVtiInterface.insertTachyon(com.ibm.Pointer, com.ibm.Pointer, com.ibm.Pointer)'
LANGUAGE java;

CREATE FUNCTION getByte(pointer)
RETURNING integer
 WITH (class='jvp', NOT VARIANT, PARALLELIZABLE)
EXTERNAL NAME 'informix_tachyon:com.ibm.TachyonVtiInterface.getByte(com.ibm.Pointer)'
LANGUAGE java;


CREATE PRIMARY ACCESS_METHOD vtam
(
--AM_CREATE = createTableInTachyon,
--AM_OPEN = connectToTachyon,
--AM_CLOSE = disconnectFromTachyon,
--AM_DROP = deleteTableInTachyon,
--AM_GETNEXT = getNextTachyon,
--AM_INSERT = insertTachyon,
--AM_GETBYID = getByte,
AM_CREATE = tachyonCreate,
AM_OPEN = tachyonOpen,
AM_CLOSE = tachyonClose,
AM_DROP = tachyonDrop,
AM_GETNEXT = tachyonGetNext,
AM_INSERT = tachyonInsert,
AM_GETBYID = tachyonGetById,
AM_READWRITE,
AM_ROWIDS,
AM_SPTYPE = 'X',
AM_CLUSTER);

CREATE TABLE vt(
col1 INTEGER,
col2 INTEGER)
IN tachyon
USING vtam;

-- Load the BSON Utilities JAR
EXECUTE PROCEDURE sqlj.install_jar(
"file:$INFORMIXDIR/jars/tachyon_vti_interface.jar",
 "informix_tachyon", 1);

CREATE FUNCTION disconnectFromTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix.tachyon_vti_interface:com.ibm.TachyonVtiInterface.disconnectFromTachyon(java.lang.String)'
LANGUAGE java;

CREATE FUNCTION connect_to_tachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix.tachyon_vti_interface:com.ibm.TachyonVtiInterface.connect_to_tachyon(java.lang.String)'
LANGUAGE java;

CREATE FUNCTION createTableInTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix.tachyon_vti_interface:com.ibm.TachyonVtiInterface.createTableInTachyon(java.lang.String)'
LANGUAGE java;

CREATE FUNCTION deleteTableInTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix.tachyon_vti_interface:com.ibm.TachyonVtiInterface.deleteTableInTachyon(java.lang.String)'
LANGUAGE java;

CREATE FUNCTION beginScanTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix.tachyon_vti_interface:com.ibm.TachyonVtiInterface.beginScanTachyon(java.lang.String)'
LANGUAGE java;

CREATE FUNCTION getNextTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix.tachyon_vti_interface:com.ibm.TachyonVtiInterface.getNextTachyon()'
LANGUAGE java;

CREATE FUNCTION endScanTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix.tachyon_vti_interface:com.ibm.TachyonVtiInterface.endScanTachyon(java.lang.String)'
LANGUAGE java;

CREATE FUNCTION insertTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix.tachyon_vti_interface:com.ibm.TachyonVtiInterface.insertTachyon(java.lang.String, java.lang.String, int)'
LANGUAGE java;

CREATE FUNCTION getByte(pointer)
RETURNING integer
EXTERNAL NAME 'informix.tachyon_vti_interface:com.ibm.TachyonVtiInterface.getByte(java.lang.String)'
LANGUAGE java;

CREATE PRIMARY ACCESS_METHOD tachyon_vti
(AM_OPEN = connect_to_tachyon,
AM_CLOSE = disconnectFromTachyon,
AM_CREATE = createTableInTachyon,
AM_DROP = deleteTableInTachyon,
AM_BEGINSCAN = beginScanTachyon,
AM_GETNEXT = getNextTachyon,
AM_ENDSCAN = endScanTachyon,
AM_INSERT = insertTachyon,
AM_GETBYID = getByte,
AM_READWRITE,
AM_ROWIDS,
AM_SPTYPE = 'X',
AM_CLUSTER)


DROP FUNCTION disconnectFromTachyon(pointer);
DROP FUNCTION connect_to_tachyon(pointer);
DROP FUNCTION createTableInTachyon(pointer);
DROP FUNCTION deleteTableInTachyon(pointer);
DROP FUNCTION beginScanTachyon(pointer);
DROP FUNCTION getNextTachyon(pointer);
DROP FUNCTION endScanTachyon(pointer);
DROP FUNCTION insertTachyon(pointer);
DROP FUNCTION getByte(pointer);
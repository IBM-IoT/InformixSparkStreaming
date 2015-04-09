-- Load the BSON Utilities JAR
EXECUTE PROCEDURE sqlj.install_jar(
"file:$INFORMIXDIR/jars/tachyon_vti_interface.jar",
 "informix_tachyon", 1);
-- Bind to the server UDT for the BSON type:
EXECUTE PROCEDURE sqlj.setUDTExtName('TachyonVtiInterface', 'com.ibm.informix.TachyonVtiInterface');

CREATE PROCEDURE connect_to_tachyon()
   WITH (class='jvp')
   EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.connect_to_tachyon()'
   LANGUAGE java;

CREATE FUNCTION disconnectFromTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.disconnectFromTachyon()'
LANGUAGE java;

CREATE FUNCTION connect_to_tachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.connect_to_tachyon()'
LANGUAGE java;


CREATE FUNCTION createTableInTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.createTableInTachyon()'
LANGUAGE java;

CREATE FUNCTION deleteTableInTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.deleteTableInTachyon()'
LANGUAGE java;

CREATE FUNCTION beginScanTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.beginScanTachyon()'
LANGUAGE java;

CREATE FUNCTION getNextTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.getNextTachyon()'
LANGUAGE java;

CREATE FUNCTION endScanTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.endScanTachyon()'
LANGUAGE java;

CREATE FUNCTION insertTachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.insertTachyon()'
LANGUAGE java;

CREATE FUNCTION getByte(pointer)
RETURNING integer
EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.getByte()'
LANGUAGE java;

CREATE PRIMARY ACCESS_METHOD my_virtual
(AM_OPEN = connect_to_tachyon,
AM_CLOSE = disconnectFromTachyon,
AM_CREATE = createTableInTachyon,
AM_DROP = deleteTableInTachyon,
AM_BEGINSCAN =beginScanTachyon,
AM_GETNEXT = getNextTachyon,
AM_ENDSCAN = endScanTachyon,
AM_INSERT = insertTachyon,
AM_GETBYID = getByte,
AM_READWRITE,
AM_ROWIDS,
AM_SPTYPE = 'X',
AM_CLUSTER)

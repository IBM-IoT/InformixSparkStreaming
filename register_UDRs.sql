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

CREATE FUNCTION connect_to_tachyon(pointer)
RETURNING integer
EXTERNAL NAME 'informix_tachyon:TachyonVtiInterface.connect_to_tachyon()'
LANGUAGE java;
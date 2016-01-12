
-- If you haven't already, look at examples/setup.sql first!

-- Once the Virtual Index Interface is set up, to use it create an 'informix_socket_streaming' index on the table/columns you want to watch
-- This index will never be used to optimize database operations. It is safe to use it with other indexes.
-- When creating the index, the columns specified in the statement will be the only ones streamed out of the database.
-- Also, the index currently supports and requires two parameters: the host and port of the target MQTT server
-- The topic being published to will be the name of the table

-- In this example, a table with 3 columns is created first...
create table test(col1 MONEY, col2 INTEGER, col3 VARCHAR(100));

-- ...then, the index is created only on the 1st and 3rd column, publishing to localhost:1883
create index socket_stream on test(col1,col3) USING informix_socket_streaming(host='localhost',port='1883');
-- This assumes there's an MQTT Server running locally. See http://mosquitto.org for an easy-to-setup MQTT Server.

-- Now, let's insert, update, and delete some rows
insert into test values (1.99,100,"1st row");
insert into test values (2.99,200,"2nd row");
insert into test values (3.99,300,"3rd row");
update test set col3 = "It works!" where col1 = 2.99;
delete from test where col1 > 2.50;
-- These changes should be published under the topic "test"

-- Finally, clean up...
DROP INDEX IF EXISTS socket_stream;
DROP TABLE IF EXISTS test;

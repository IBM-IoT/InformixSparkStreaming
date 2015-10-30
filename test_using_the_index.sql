-- Very simple test of the streaming functionality. Create a table, add the socket streaming index type and insert
-- some data.

create table test(col1 INTEGER, col2 INTEGER);

create index socket_stream on test(col1) USING informix_socket_streaming;

insert into test(col1, col2) values (12456756, 435345);
insert into test(col1, col2) values (784566, 8768);
insert into test(col1, col2) values (784566, 8768);

select * from test;

DROP INDEX IF EXISTS socket_stream;
DROP TABLE IF EXISTS test;
## USAGE

Create an index on an existing table using the "socket_streaming" access method.

```create index demo_index on demo(id, message, temperature) USING socket_streaming;```

Then ```INSERT``` and ```UPDATE``` into the database as usual.

The added data will be instantly streamed to Apache Spark.
# Some misc. useful commands to use for development/debugging

Find what applications using a specific port. This is useful for debugging if a socket isn't connecting as expected.

$ sudo netstat -anp | grep <port number>

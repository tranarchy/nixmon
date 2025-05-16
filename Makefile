all:
	cc -Wall *.c */*.c -o nixmon

install:
	cp -f nixmon /usr/local/bin
CFLAGS = -std=c11 -Wall -Wextra -ggdb
TARGET = text2web

$(TARGET): main.c lexer.c
	gcc *.c -o $(TARGET) $(CFLAGS)

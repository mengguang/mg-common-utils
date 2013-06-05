CFLAGS += $(shell pkg-config --cflags json)
LDFLAGS += $(shell pkg-config --libs json)
all: json_encode
json_encode: json_encode.c 
	gcc  -g ${CFLAGS} ${LDFLAGS} -std=c99 -o json_encode json_encode.c
clean:
	rm -f json_encode *.o 

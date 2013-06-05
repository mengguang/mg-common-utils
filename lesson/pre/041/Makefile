CFLAGS += $(shell pkg-config --cflags json)
LDFLAGS += $(shell pkg-config --libs json)
all: json_decode
json_decode: json_decode.c 
	gcc  -g ${CFLAGS} ${LDFLAGS} -std=c99 -o json_decode json_decode.c
clean:
	rm -f json_decode *.o 

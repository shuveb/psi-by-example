monitor: monitor.c
		gcc -o $@ $<

create_load: create_load.c
		gcc -o $@ $< -lpthread

all: monitor create_load

.PHONY: clean

clean:
	rm -f monitor create_load


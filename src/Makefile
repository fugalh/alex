CXXFLAGS=-g -Wall
CFLAGS=${CXXFLAGS}
all: alex

alex: audio.o jack.o jack_client.o gsm.o iax.o main.o 
	g++ $(CXXFLAGS) -o $@ $^ -lgsm -liax -ljack -lm -lsamplerate

clean: 
	rm -f *.o alex core.*

.PHONY: clean all

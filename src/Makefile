CXXFLAGS=-g
alex: audio.o jack.o coder.o gsm.o iax.o main.o 
	g++ $(CXXFLAGS) -o $@ $^ -lgsm -liax -ljack -lm -lsamplerate

clean: 
	rm -f *.o alex core.*

.PHONY: clean

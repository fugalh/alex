CXXFLAGS=-g
#alex: audio.o jack.o coder.o gsm.o iax.o main.o 
alex: audio.o jack.o main.o 
	g++ $(CXXFLAGS) -o $@ $^ -lgsm -liax -ljack

clean: 
	rm -f *.o alex core.*

.PHONY: clean

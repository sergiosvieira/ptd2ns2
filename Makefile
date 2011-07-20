all:
	g++ -g -Drng_stand_alone -lm ptd2ns2.cc random.cc ranvar.cc rng.cc -o ptd2ns2

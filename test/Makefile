CC=gcc
CFLAGS=-Wall -ggdb
INCLUDE=-I ../include/
LIBS=-L../ -lmpl

DIV_OBJECTS=div_test.o
DIVTEST=divtest

MOD_EXP_OBJECTS=mod_exp_simple_test.o
MOD_EXP_TEST=modexptest

RANDOM_OBJECTS=random_test.o
RANDOM_TEST=randomtest

all: $(DIVTEST) $(MOD_EXP_TEST) $(RANDOM_TEST)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(DIVTEST): $(DIV_OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(MOD_EXP_TEST): $(MOD_EXP_OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(RANDOM_TEST): $(RANDOM_OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

clean:
	rm -rf *~ *.o $(DIVTEST) $(MOD_EXP_TEST)

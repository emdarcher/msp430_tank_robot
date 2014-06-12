#change to correct file
#OBJECTS = main.o 
OBJECTS = light_seeker_basic.c
 
CC = msp430-gcc
#change to correct ic here
CFLAGS =-Os -Wall -g -mmcu=msp430g2452
 
all : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o main.elf
 
%.o : %.c 
	$(CC) $(CFLAGS) -c $<

 
clean:
	rm -fr $(OBJECTS) main.elf
 
erase:
	mspdebug rf2500 "erase"
 
upload:
	mspdebug rf2500 "prog main.elf"

tilib-upload:

	mspdebug-new tilib "prog main.elf"

size:

	msp430-size main.elf

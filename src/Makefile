# Variable:	DEPENDENCIES
# Content:	List with all the *.c source file 
#			with their extension replaced with *.o, 
#			used with the final rule to build them recursivelly.
#
# Example:	We have these files:
#
#				library.c header.c etc.c
#
#			Now we get the list
#
#				library.o header.o etc.o
#
DEPENDENCIES=$(patsubst %.c, %.o, $(wildcard *.c))

# Rule:			all
#
# Objective:	Build source files recursivelly.
#
all: $(DEPENDENCIES)

# Rule:			%.o: %.c
#
# Objective:	Build source files base in its object name.
#
%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

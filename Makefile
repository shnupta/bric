# GLOBAL VARIABLES
export CC=gcc
export CFLAGS=-Wall -g -W -pedantic -std=gnu99
export LDFLAGS=-lm

export BRIC_EXECUTABLE=bric
export BRIC_DIRECTORY=/usr/local/bin/

# LOCAL VARIABLES
SOURCE_DIRECTORY=src

# Rule:			all
#
# Objective:	Build the $(BRIC_EXECUTABLE).
#
all: src
	$(CC) $(SOURCE_DIRECTORY)/*.o $(CFLAGS) $(LDFLAGS) -o $(BRIC_EXECUTABLE)

# Rule:			install
#
# Objective:	Install the $(BRIC_EXECUTABLE) on the system.
#
install: all
	mkdir -p $(BRIC_DIRECTORY)
	chmod 755 $(BRIC_EXECUTABLE)
	cp -a $(BRIC_EXECUTABLE) $(BRIC_DIRECTORY)

# Rule:			clean
#
# Objective:	Clean all the objects inside 
#				the $(SOURCE_DIRECTORY) directory 
#				and the $(BRIC_EXECUTABLE) file.
#
clean:
	rm -f bric
	find $(SOURCE_DIRECTORY) -type f -iname "*.o" -exec rm {} +

# Rule:			src
#
# Objective:	Build the source files inside 
#				the $(SOURCE_DIRECTORY) directory.
#
.PHONY: $(SOURCE_DIRECTORY)
$(SOURCE_DIRECTORY):
	$(MAKE) --directory=$(SOURCE_DIRECTORY)

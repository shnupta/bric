# bric
bric is a text editor based on kilo.

bric does not depend on any library (not even curses). It uses fairly standard VT100 (and similar terminals) escape sequences to write and read to and from the terminal.


### Installation:
```
git clone https://github.com/shnupta/bric
cd bric
make
make install
```

### Usage:
If you have performed a `make install` then just `bric <filename>`. It can be a new or existing filename. 

Shortcuts:
```
Ctrl-Q - Quit 
Ctrl-S - Save 
Ctrl-F - Find 
Ctrl-G - Goto
Arrow Keys - Move
```

### Todo:
- Git integration
- More syntax keyword colours
- More language syntax highlighting
- More shortcuts

### Contribution:
I'm completely open to anyone forking and helping build features of the editor so go ahead and make a PR!

### Bugs and feedback:
To submit any bugs or give feedback please add an issue on this repo.

### References:
- kilo - https://github.com/antirez/kilo


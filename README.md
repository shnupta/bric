# bric
bric is a text editor based on kilo.

bric does not depend on any library (not even curses). It uses fairly standard VT100 (and similar terminals) escape sequences to write and read to and from the terminal.

[My blog post](https://shnupta.github.io/blog/17/04/bric.html)

### Here is a screencast of bric on macOS:
![Screencast](https://github.com/shnupta/bric/blob/master/screencast_low.gif)

### Installation:
```
git clone https://github.com/shnupta/bric
cd bric
make install
```

### Usage:
If you have performed a `make install` then just `bric <filename>`. It can be a new or existing filename. 

Config file:
bric is customizable from a configuration file - `~/.bricrc`

Here is an example config file:
```
set linenumbers true
set indent true

set hl_comment_colour 33
set hl_mlcomment_colour 33
set hl_keyword_cond_colour 36
set hl_keyword_type_colour 32
set hl_keyword_pp_colour 34
set hl_keyword_return_colour 35
set hl_keyword_adapter_colour 94
set hl_keyword_loop_colour 36
set hl_string_colour 31
set hl_number_colour 34
set hl_match_colour 101
set hl_background_colour 49
set hl_default_colour 37

set tab_length 8
```
Note: 
Not all options have to be set. The ones shown above are the colours (however indent maintaining and linenumbers are not)


### Contribution:
I'm completely open to anyone forking and helping build features of the editor so go ahead and make a PR! A contribution guide will be written shortly.

### Bugs and feedback:
To submit any bugs or give feedback please add an issue on this repo.

### References:
- kilo - https://github.com/antirez/kilo


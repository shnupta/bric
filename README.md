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

Currently bric only supports a few vim commands (with similar names) but plans are in place to implement more to make editing much easier and more efficient.

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

### Brief tutorial:
Here are a brief list of commands to start use the editor:

Insert Mode: We have some different ways to that:
- Press 'i' to active this mode at the current cursor position
- Press 'I' to active this mode at the beginning of line where the cursor is
- Press 'o' to create a new line and active this mode
- Press 'O' to create a new line at the current cursor position (shift all the lines below)
- Press 'a' to active this mode one character forward from the current cursor position
- Press 'A' to active this mode at the end from the current line

Normal Mode:
- Press 'ESC' to active this mode

Other basic commands:
- Press 'CTRL+Q' used to exit without save (you'll receive a message to "confirm")
- Press 'g' jump the cursor to the first line
- Press 'G' jump the cursor to the last line
- Press '$' move the cursor to the last character of the current line
- Press '0' move the cursor to the first character of the current line
- Press 'HOME_KEY' move the cursor to the first character of the current line
- Press 'END_KEY' move the cursor to the last character of the current line

### Contribution:
I'm completely open to anyone forking and helping build features of the editor so go ahead and make a PR! A contribution guide will be written shortly.

The basic workflow as of November 2017 is:
- Development and pul requests are made on the development branch.
- These PRs will be reviewed and if accepted merged into the development branch.
- Every (x time period) a test version of bric will be merged into the testing branch for confirmation that new features are working as expected.
- Once tested, approved features will be merged into the master branch for the next release.

### Bugs and feedback:
To submit any bugs or give feedback please add an issue on this repo or email me or one of the contributors.

### References:
- kilo - https://github.com/antirez/kilo
- My site - https://shnupta.github.com

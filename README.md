[![Build Status](https://travis-ci.com/shnupta/bric.svg?branch=development)](https://travis-ci.com/shnupta/bric)

[![Waffle.io - Columns and their card count](https://badge.waffle.io/shnupta/bric.svg?columns=all)](https://waffle.io/shnupta/bric)

[Join our slack team!](https://join.slack.com/t/bric-editor/shared_invite/enQtNDIyNjg5NzY2MTQ1LTk4OTE4ZjdiMGFmMDlhNWM2ZWJkMmM0MGQxMjNhODJlOWY1MjQzMmQ5MDEzOGM3YjM0YTJiZTc3MWY5MGNmZjI)

# bric
bric is a text editor based on kilo.

bric does not depend on any library (not even curses). It uses fairly standard VT100 (and similar terminals) escape sequences to write and read to and from the terminal.

[My blog post](https://shnupta.github.io/blog/17/04/bric.html)

### Here is a screencast of bric on macOS:
![Screencast](https://github.com/shnupta/bric/blob/master/screencast_low.gif)

### Getting the code

The links to the git repo or source archives can be found at
https://github.com/shnupta/bric

### Building and Installation:

From the top level of the source directory, run:

    ./configure (use `./configure --help` to set extra options)

By default, `make install` will install all the files in
*/usr/local/bin*, */usr/local/lib* etc.  You can specify
an installation prefix other than */usr/local* using `./configure --prefix`,
for instance *--prefix=$HOME* or *--prefix=$PWD/install_test*.

    make (Before install, the resulting `bric` binary will be located in src/)

If you have write privileges to *prefix*:

    make install (otherwise, use `sudo make install`)

*Note:* You will never need root privileges to `make` bric, and it's
good practice only to use super-user privileges when absolutely required.

Those instructions will be adequate for most users, but more details
can be found in the [INSTALL](INSTALL) document.

#### Removing files:

    make clean
    make distclean

#### Uninstalling

    make uninstall (must be run before `make distclean`)

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


### Contribution:
I'm completely open to anyone forking and helping build features of the editor so go ahead and make a PR! A contribution guide will be written shortly.

The basic workflow as of November 2017 is:
- Development and pull requests are made on the development branch.
- These PRs will be reviewed and if accepted merged into the development branch.
- Every (x time period) a test version of bric will be merged into the testing branch for confirmation that new features are working as expected.
- Once tested, approved features will be merged into the master branch for the next release.

### Bugs and feedback:
To submit any bugs or give feedback please add an issue on this repo or email me or one of the contributors.

### References:
- kilo - https://github.com/antirez/kilo
- My site - https://shnupta.github.io

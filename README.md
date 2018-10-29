[![Build
Status](https://travis-ci.com/shnupta/bric.svg?branch=master)](https://travis-ci.com/shnupta/bric)

[![Waffle.io - Columns and their card
count](https://badge.waffle.io/shnupta/bric.svg?columns=all)](https://waffle.io/shnupta/bric)

[Join our Matrix
chat room!](https://matrix.to/#/!hhzVjHGoQleZmFfyrP:matrix.org)

# bric
bric is a text editor based on [kilo](https://github.com/antirez/kilo).

bric does not depend on any library (not even curses). It uses fairly
standard VT100 (and similar terminals) escape sequences to write and
read to and from the terminal.

A blog post by [Casey](https://github.com/shnupta), bric's first maintainer

[Casey's blog post](https://shnupta.github.io/blog/17/04/bric.html)

### Screencast of bric on macOS:
[View Screencast](https://andy5995.github.io/screencast_low.gif)

### Getting the code

The code on the master branch is more likely to be unstable. See the
[Releases](https://github.com/shnupta/bric/releases) section for
downloads that have been more thoroughly tested.

The links to the git repo or source archives can be found at
https://github.com/shnupta/bric

### Building and Installation:

From the top level of the source directory, run:

    mkdir build
    cd build
    ../configure (use `../configure --help` to set extra options)

By default, `make install` will install all the files in
*/usr/local/bin*, */usr/local/lib* etc.  You can specify
an installation prefix other than */usr/local* using `../configure --prefix`,
for instance *--prefix=$HOME* or *--prefix=$PWD/install_test*.

    make (Before install, the resulting `bric` binary will be located in src/)

If you have write privileges to *prefix*:

    make install (otherwise, use `sudo make install`)

*Note:* You will never need root privileges to `make` bric, and it's
good practice only to use super-user privileges when absolutely required.

Those instructions will be adequate for most users, but more details
can be found in the [INSTALL](INSTALL) document.

#### Removing files:

    make clean (or remove the build directory you created)
    make distclean

#### Uninstalling

    make uninstall (must be run before `make distclean`)

### Usage:

If you have performed a `make install` then just `bric <filename>`. It
can be a new or existing filename.

Currently bric only supports a few vim commands (with similar names)
but plans are in place to implement more to make editing much easier
and more efficient.

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

### Translating

If you are interested in translating bric, please see the [TRANSLATE document](TRANSLATE.md).

### Contribution:

I'm completely open to anyone forking and helping build features of the
editor so go ahead and make a PR!

Please review the [CONTRIBUTING
guidelines](https://github.com/shnupta/bric/blob/master/CONTRIBUTING.md).

The basic workflow as of November 2017 is:
- Development and pull requests are made on the master branch.
- These PRs will be reviewed and if accepted merged into the master branch.

### Bugs and feedback:

To submit any bugs or give feedback please add an issue on this repo or
[join the bric chat room](https://matrix.to/#/!hhzVjHGoQleZmFfyrP:matrix.org).

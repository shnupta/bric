# Usage

This document is intended to be a guide and index of the commands and functions available within the editor. If you find any problems please submit an issue.

## Contents:

- [Normal mode](#normal-mode)
	- [Movement](#movement)
	- [Edit](#edit)
	- [Misc](#misc)

- [Insert mode](#insert-mode) 
	- [Movement](#movement-1)
	- [Edit](#edit-1)
	- [Misc](#misc-1)

- [Selection mode](#selection-mode)
	- [Movement](#movement-2)
	- [Misc](#misc-2)

- [Command mode](#command-mode)
	- [Movement](#movement-3)
	- [Misc](#misc-3)

## Normal mode

1. ### Movement
>	- __h__ - Move cursor left.
>	- __j__ - Move cursor down.
>	- __k__ - Move cursor up.
>	- __l__ - Move cursor right.
>	- __Left arrow__ - Move cursor left.
>	- __Down arrow__ - Move cursor down.
>	- __Up arrow__ - Move cursor up.
>	- __Right arrow__ - Move cursor right.
>	- __G__ - Move cursor to the last row.
>	- __g__ - Move cursor to the first row.
>	- __$__ - Move cursor to the end of the current row.
>	- __0__ - Move cursor to the start of the current row.
>	- __a__ - Move one step to the right and enter insert mode.
>	- __A__ - Move cursor to the end of the current row and enter insert mode .
>	- __HOME__ - Move cursor to the start of the current row.
>	- __END__ - Move cursor to the end of the current row.
>	- __PAGE UP__ - Move cursor to the first row.
>	- __PAGE DOWN__ - Move cursor to the last line or move cursor to the last visual line (??).
>	- __ctrl + m__ - Jump to symbol definition .
>	- __ctrl + n__ - Jump back to previous position after __ctrl + m__ .

2. ### Edit
>	- __cr__ - Copy current row.
>	- __yr__ - Cut current row.
>	- __pr__ - Paste row from clipboard.
>	- __dr__ - Delete current row.
>	- __cp__ - Paste from clipboard.

3. ### Misc
>	- __cc__ - Clears the clipboard  
>	- __:__ - Enter command mode.
>	- __i__ - Enter insert mode.
>	- __I__ - Enter insert mode.
>	- __o__ - Enter insert mode on a new row under the current row.
>	- __O__ - Enter insert mode on a new row over the current row.

## Insert mode

1. ### Movement
>	- __PAGE UP__ - Move cursor to the first row.
>	- __PAGE DOWN__ - Move cursor to the last line or move cursor to the last visual line (??).
>	- __Left arrow__ - Move cursor left.
>	- __Down arrow__ - Move cursor down.
>	- __Up arrow__ - Move cursor up.
>	- __Right arrow__ - Move cursor right.
>	- __HOME__ - Move cursor to the start of the current row.
>	- __END__ - Move cursor to the end of the current row.

2. ### Edit
>	- __ctrl + y__ - Cut current row.
>	- __ctrl + p__ - Paste row from clipboard.
>	- __ctrl + h__ - Delete character under cursor
>	- __ctrl + v__ - Paste from clipboard.

3. ### Misc
>	- __ctrl + q__ - Quit.
>	- __ctrl + s__ - Save changes.
>	- __ctrl + f__ - Find.
>	- __ctrl + r__ - Find and replace.
>	- __ctrl + d__ - Enter selection mode.
>	- __ESC__ - Enter normal mode.

## Selection mode

1. ### Movement
>	- __h__ - Move cursor left.
>	- __j__ - Move cursor down.
>	- __k__ - Move cursor up.
>	- __l__ - Move cursor right.
>	- __Left arrow__ - Move cursor left.
>	- __Down arrow__ - Move cursor down.
>	- __Up arrow__ - Move cursor up.
>	- __Right arrow__ - Move cursor right.

2. ### Misc
>	- __ESC__ - Enter Normal mode.
>	- __ctrl + c__ - Copy to clipboard.

## Command mode

1. ### Movement
>	- __*[0-9]__ - Move cursor to specified line number.

2. ### Misc
>	- __q!__ - Force quit without saving changes.
>	- __w__ - Save changes.
>	- __wq__ - Save changes and quit.
>	- __q__ - Quit.
>	- __f__ - Find.
>	- __fr__ - Find and replace. 
>	- __sm__ - Enter selection mode.
>	- __sp__ - ???
>	- __up__ - ???

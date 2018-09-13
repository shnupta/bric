# Translating

Translators are encouraged to join the [chat
room](https://join.slack.com/t/bric-editor/shared_invite/enQtNDIyNjg5NzY2MTQ1LTk4OTE4ZjdiMGFmMDlhNWM2ZWJkMmM0MGQxMjNhODJlOWY1MjQzMmQ5MDEzOGM3YjM0YTJiZTc3MWY5MGNmZjI).
We will try to help and answer any questions you may have about the
translation process.

There are some [po editing
utilities](https://www.gnu.org/software/trans-coord/manual/web-trans/html_node/PO-Editors.html)
that can be helpful (but not required. .po files can be edited with any text editor).

If you have experience translating files in a [po format](https://www.gnu.org/software/gettext/manual/html_node/PO-Files.html), then you will be able to skip most of the instructions.

A sample po file (from the GNU coreutils `cp` command)
```
msgid "write error"
msgstr "Schreibfehler"

#: lib/copy-acl.c:54 src/copy.c:1387 src/copy.c:2863
#, c-format
msgid "preserving permissions for %s"
msgstr "Erhalten der Zugriffsrechte für %s"

#: lib/error.c:191
msgid "Unknown system error"
msgstr "Unbekannter Systemfehler"

#: lib/file-type.c:40
msgid "regular empty file"
msgstr "reguläre leere Datei"

#: lib/file-type.c:40
msgid "regular file"
msgstr "reguläre Datei"
```

* You will need the `gettext` package from your OS distribution.
* [Fork the repo](https://github.com/shnupta/bric#fork-destination-box)
* After you clone your fork, go into the bric source po/ directory

* run `msginit -i bric.pot -o lc_CC.po -l lc_CC` (Where lc is your
 [iso-631-1 language code](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)) and CC is your country code

* At this point, you may open your new .po file and begin adding translations
* When done, make a [pull request](CONTRIBUTING.md)
* Thank you!

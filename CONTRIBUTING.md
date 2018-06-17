# Contributing

Welcome to bric. If you are reading this you are most likely wanting to write some code and help out with the development of bric, so get reading and find out how.

## Pull Requests ##
1. Fork the repo
2. Clone it to your computer
3. When you're ready to work on an issue, be sure you're on the **master** branch. From there, [create a separate branch](https://github.com/Kunena/Kunena-Forum/wiki/Create-a-new-branch-with-git-and-manage-branches) (e.g. issue_32)
4. Make your changes.
5. Commit your changes. [git-cola](https://git-cola.github.io/) is a nice GUI front-end for adding files and entering commit messages (git-cola is probably available from your OS repository).
6. Push the working branch (e.g. issue_32) to your remote fork.
7. Make the pull request (on the [upstream **development** branch](https://github.com/shnupta/bric/tree/development))
    * Do not merge it with the master branch on your fork. That would result in multiple, or unrelated patches being included in a single PR.

## Syncing ##
Periodically, you'll need the sync your repo with mine (the upstream). GitHub has instructions for doing this

* [Configuring a remote for a fork](https://help.github.com/articles/configuring-a-remote-for-a-fork/)
  * For step 3 on that page, use https://github.com/shnupta/bric for the URL.
* [Syncing a Fork](https://help.github.com/articles/syncing-a-fork/)
  * On that page, it shows how to merge the **master** branch (steps 4 & 5).

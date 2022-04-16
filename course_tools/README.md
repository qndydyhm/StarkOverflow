# CSE 320 Fall 2020 Course Tools


Run the script to install the 320 course tools and packages.
```
$ bash vm-setup.sh
```

The script will ask for sudo privileges to install packages.

**NOTE THIS TOOL RUNS APT UPGRADE**

Once the script has been run, read the comments at the end of the script
in order to install packages into the Sublime Text editor.  The commented
commands in the script are an old version of an automatic installation
procedure, which doesn't work any more.  What you should do instead is to
first manually install "Package Control", then "SublimeLinter", "SublimeLinter-gcc",
and "TrailingSpaces".  Then, copy file `SublimeLinter.sublime-settings` to
directory `~/.config/sublime-text-3/Packages/User` as described in the
comments.

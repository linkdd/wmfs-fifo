# Installation

```
$ make
# sudo make install
or
# sudo make install PREFIX=/usr/local
or
# sudo make install DESTDIR=~/.local/
```

# How to use

Be sure to run ``wmfs-fifo`` *before* any program which use it.

```
$ wmfs-fifo &
$ echo "spawn urxvtc" > /tmp/wmfs-$DISPLAY.fifo
```


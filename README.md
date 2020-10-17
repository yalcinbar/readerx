# readerx
Simple X11 based PDF reader with Vim-like keybindings

### Dependencies
You need to have **x11**, **cairo**, **poppler-glib** and **glib-2.0** installed.

### Installation
Run <code>make</code> and <code>sudo make install</code>.

### Usage
You can run readerx with <code>readerx FILE</code>. Scroll up/down/left/right support repetition (e.g. 10j means scrolling down 10 times).

### Keybindings
Scroll up: k, ↑, Mouse wheel

Scroll down: j, ↓, Mouse wheel

Scroll left: h, ←

Scroll right: l, →

Go to page number x: xG

Next page: Ctrl + f, PgDn

Previous page: Ctrl + b, PgUp

Go to first page: gg, Home

Go to last page: G, End

Continuous/non-continuous mode: c

Fit page/width: f

Zoom in: +

Zoom out: -

Repeat last action: .

Quit: Alt + F4

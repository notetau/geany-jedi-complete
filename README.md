geany-jedi-complete
====================

Geany plugin to provide code completion (Python) using jedi autocompletion library

## requirements

1.

`libcurl-dev` virtual package or `libcurl4-openssl-dev`

`libgtk2.0-dev`

install from repos on Ubuntu:

`sudo apt-get install libgtk2.0-dev libcurl4-openssl-dev`

2.

`jedi`

install using pip:

`sudo pip install jedi`

## install plugin

`sudo make install`

or

`sudo make install PREFIX=<INSTALL DIR>`

then you can enable plugin in Plugin manager from editor

also you can reinstall jedi server script from settings

## screenshot

![screenshot](https://github.com/notetau/geany-complete-core/wiki/image/geany-cc_sc2.png)

## uninstall plugin

````
rm /usr/lib/i386-linux-gnu/geany/geanyjedicomplete.so
rm -rf $HOME/.config/geany/plugins/jedi-complete
````

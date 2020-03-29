geany-jedi-complete
====================

A Geany plugin to provide code completion (Python) using the jedi autocompletion library

## Requirements

1.

`libcurl-dev` virtual package or `libcurl4-openssl-dev`

`libgtk2.0-dev`

Install from repositories on Ubuntu:

`sudo apt-get install libgtk2.0-dev libcurl4-openssl-dev`

2.

`jedi`

Install using pip:

`sudo pip install jedi`

## Install plugin

`sudo make install`

or

`sudo make install PREFIX=<INSTALL DIR>`

Then you can enable the plugin in the Plugin manager from the editor.

Also, you can reinstall the jedi server script from settings.

## Screenshot

![Screenshot](https://github.com/notetau/geany-complete-core/wiki/image/geany-cc_sc2.png)

## Uninstall plugin

Remove the installed library. E.g.,

````
rm /usr/lib/i386-linux-gnu/geany/geanyjedicomplete.so
````

Remove the configuration files. e.g.

````
rm -rf $HOME/.config/geany/plugins/jedi-complete
````

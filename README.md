crlserver
=========

crlserver is a roguelike server in the spirit of dgamelaunch, but it
manages users logins and scores for far more than just nethack.

Feature:

 * It's free, no cost and GPL'ed.
 * Multiple games
 * Simple configuration
 * SQL abstraction
 * Simple and clean UI

TODO:

 * Support of ttyrec for games record.
 * Support of ttyplay for real time games watching.
 * Configurable menus via the configuration file, like dgamelaunch does.

Contribution
------------
There is an important thing to understand if you want to contribute :<br />
I **only** accept ISC licensed patches. No GPL !

Remember that your patches are your own intellectual properties.

How to setup ?
--------------

Fetch the code and use BSDmakefile or GNUmakefile, depending on your
working environment.

You have to edit pathnames.h if you want to change path for files or folders.

Add a user 'crls' and give it crlserver as login shell.
Now, setup a ssh server, and *tada*, it works.

If you have an error or even a little warning, mail me (or fork and patch it).

crlserver is actualy known to run and work fine on Ubuntu and OpenBSD,
help us to run everywhere :)

Why X, my favorite roguelike, is not supported ?
------------------------------------------------

Hey, just add the configuration file for it.
It's simple.

This is the rogue one :

    name=rogue
    longname=rogue-clone
    version=III
    description=Written by Tim Stoehr. This game is the origin.
    key=r
    path=/usr/games/rogue
    params=
    env=ROGUEOPTS=name=%user%

The params and env keywords are optionals.

I'm on Windows and I want to run crlserver.
-------------------------------------------
Windows is not supported and not a targeted platform.<br />
It's designed to work on Unix-like systems.

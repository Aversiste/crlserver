crlserver
=========

crlserver is roguelike server in the spirit of dgamelaunch, but it
manages users logins and scores for far more than just nethack.

It uses a little configuration file to launch the games, so you can
extend it without any recompilation, contrary to dgamelaunch.

Feature:

 * It's free, no cost and GPL'ed.
 * Multiple games
 * Simple configuration
 * SQL abstraction
 * Simple and clean UI

TODO:

 * Use of yacc/lex for configuration file parsing.
 * Support of ttyrec for games record.
 * Support of ttyplay for real time games watching.

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

So, it won't work on my VMS?
----------------------------
Yes. But if I have access to a (Open)VMS box, I can try to officially
port crlserver.
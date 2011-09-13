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
    [xin@ronnie:crlserver][14:19][18]cat /var/games/crlsdir/games/rogue 
    name=rogue
    longname=rogue-clone
    version=III
    description=Written by Tim Stoehr. This game is the origin.
    key=r
    path=/usr/games/rogue
    params=
    env=ROGUEOPTS=name=%user%

The params and env keywords are optionals.

petbuilds
=========

recipes for pet package building

see http://www.murga-linux.com/puppy/viewtopic.php?t=96027 for more info
and the files `new2dir` and `dir2pet` which are required for this system to
operate. They will be included in puppy soon after some more testing.

usage
-----
-it is suggested if you cloned this repo to copy it somewhere else so 
you can pull or fetch without issue
-just cd into `build` and run `./build_all.sh`. Hopefully all packages
will download and build successfully. The idea is "code goes in, package
comes out".
-if you want to build individual packages just cd into the package dir
and run its build recipe.EXAMPLE `sh rox_filer.petbuild`

recipes
-------
Just look at some of the examples. They aren't difficult. There is no 
stringent requirement to use `new2dir`, as you can use DESTDIR or even paco,
but if you do use paco can you make a recipe first please?:P
Also, for weight requirements, split (DEV,DOC,NLS separated packages) are highly
desirable.

Anyone with write access to the puppylinux git repo is encouraged to add recipes.
Forks are needed too so we can consider your pull requests. All updated packages
will be added to the ibiblio/puppylinux repository.

BUGS
----
There will be BUGS! Raise an issue here or report on the murga thread above.

ACK
---
amigo, jamesbond, Iguleder, Tman

### Compiling common32/64 pets ###

To use petbuilds-run_woof.sh you need run_woof, if you do not already have
run_woof but have git installed, the following command will download it.
(you must be in a linux type filesystem)

```
git clone https://github.com/puppylinux-woof-CE/run_woof.git
```

You will also need to download petbuilds and checkout the 'generic' branch.

```
git clone https://github.com/puppylinux-woof-CE/petbuilds.git
cd petbuilds
git checkout generic
```

The 'ORDER-common' list in the basic/pkgs directory lists all the packages that
compile successfully as 'common32/64' pets, but you can also make your own list
as long as it's name starts with 'ORDER-' (it must be in the basic/pkgs
directory)

To compile all the packages in the 'ORDER-common' list, run the following
command,

```
./petbuilds-run_woof.sh ORDER-common
```

This will download two sets of devx/ISOs to local-repositories/petbuilds (about
160M) and the source code for the packages listed in 'ORDER-common' (about 20M).

The pets will be put in petbuilds-out/puppylinux and the log files will be in
petbuilds-out/0logs.

The check-status.sh script can be run from the petbuilds-out directory (there is
a symlink) to check the git hash listed in the log files against the git hash of
the last commit to touch each directory in
petbuilds/basic/pkgs.

```
cd ../petbuilds-out
./check-status.sh ORDER-common
```

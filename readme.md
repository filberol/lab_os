### How to build and use kernel modules

First install devtools
```
sudo apt get update && sudo apt get upgrade
sudo apt-get install build-essential kmod flex bison bc
```
You will also need the linux headers
```
apt-cache search linux-headers-`uname -r` // didnt work for me
uname -r // get your kernel version and instal generic headers
sudo apt-get install linux-headers-x.x.x-xx-generic
```
### Description
This is the sample `top` utility, written with kernel module Api.

Kernel module consists of ``init_module()`` and ``cleanup_module()``

These are the functions that wil be invoked under inserting and deleting our module.
There we create a debugfs file to which we can write and read from client program.
This is the communication channel. Thus, we have ``request`` and ``response(process_info)``
structures, which will contain all the info/

In ``debugfs_op`` we declare our own functions that will be invoked upon read and
write call to debugfs.

We also use `si_meminfo(struct)` and `for_each_process` function and macro, that are
internal kernel interfaces to get certain structures, in this case `struct sysinfo`
and `struct task_struct`

### Build and run
make command builds the module, using makefile in `/usr/lib/headers...` directory
with `modules` target. Then the modules are inserted with `insmod` command.
You can also check current modules with `lsmod`. Finally, module is unloaded
with `rmmod`
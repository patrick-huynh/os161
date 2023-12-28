# OS161 | System/161


## About System/161
System/161 is a machine simulator that provides a simplified but still realistic environment for OS hacking. It is a 32-bit MIPS system supporting up to 32 processors, with up to 31 hardware slots each holding a single simple device (disk, console, network, etc.) It was designed to support OS/161, with a balance of simplicity and realism chosen to make it maximally useful for teaching. However, it also has proven useful as a platform for rapid development of research kernel projects.

System/161 supports fully transparent debugging, via remote gdb into the simulator. It also provides transparent kernel profiling, statistical monitoring, event tracing (down to the level of individual machine instructions) and one can connect multiple running System/161 instances together into a network using a "hub" program.

Multiprocessor support is introduced with System/161 2.x, which will run both multiprocessor OS/161 2.x and uniprocessor OS/161 1.x. The System/161 1.x branch is now discontinued. Everyone should move to 2.x.

The primary things that are not in System/161:

- There is no hardware floating point support.
- There is no cache timing model.
- There is no enforcement of cache flushing, memory barrier, or pipeline hazard requirements.
- The hardware devices do not support DMA but require the processor to do bulk copies out of transfer buffers.
- The processor is still an r3000 at heart rather than a MIPS32. This is intentional: we want the simpler r3000 MMU and feel the paired-pages model of the later MIPS MMUs is unnecessarily complicated.


## Installation Guide

### Step 0: Please be aware of the FAQ
If things go wrong please have a look at the [FAQ](UW: OS/161 and tools FAQ) for answers and solutions. In particular most systems require the installation of additional software and/or libraries. This will vary by Linux distro and/or Mac OS X version and we'll try to collect information in the FAQ.

### Step 1: Download
Start by downloading the necessary files, which are described in the following table:
What	Download Link	Approx. Download Size (MB)	Approx. Unpacked Size (MB)
Binutils for MIPS	os161-binutils.tar.gz	18	113
GCC MIPS Cross-Compiler	os161-gcc.tar.gz	23	95
GDB for Use with OS/161	os161-gdb.tar.gz	20	116
bmake for use with OS/161	os161-bmake.tar.gz	1	2
mk for use with OS/161	os161-mk.tar.gz	1	1
sys/161	sys161.tar.gz	1	2
OS/161	os161.tar.gz	1	3


NOTE: It may take some time to download some of these files and if you try to proceed (e.g., run tar before all of the file has been downloaded it will fail). Please check that your browser has finished downloading the file you are going to work with before working with it.

A Virtual Machine May Help
If you are running Windows or macOS you may have a difficult time getting OS/161 installed natively. One option is to install a Virtual Machine. Michael Honke has made available a Virtual Box VM (it runs Linux) for you and has OS/161 installed already. It can be downloaded here. The VM's OS is very light, you should not need 3D acceleration or more than 1024MB of RAM. However, you will need to enable virtualization on your CPU (Intel VT or AMD-V). This is available on most current CPUs and is a setting in your BIOS (typically). The login is: cs350 and the password is: os161

### Step 2: Build and Install the Binary Utilities (Binutils)
Unpack the binutils archive:
tar -xzf os161-binutils.tar.gz
Move into the newly-created directory:
cd binutils-2.17+os161-2.0.1
Configure binutils:
./configure --nfp --disable-werror --target=mips-harvard-os161 --prefix=$HOME/sys161/tools
Make binutils:
make
If the make command fails because of a problem related to makeinfo, try running the following command:
find . -name '*.info' | xargs touch
and then re-run make.
Finally, once make has succeeded, install the binutils into their final location:
make install
This will create the directory $HOME/sys161/tools/ and populate it.

### Step 3: Adjust Your Shell's Command Path
First, make the directory in which your shell will eventually find the toolchain binaries:
mkdir $HOME/sys161/bin
Next, add two directories ($HOME/sys161/bin and $HOME/sys161/tools/bin) to your shell's search path. Users of bash should do something like this in your .bash_profile and .bashrc files in your home directory:
export PATH=$HOME/sys161/bin:$HOME/sys161/tools/bin:$PATH
while users of csh or tcsh would typically use .profile and .cshrc (in your home directory):
setenv PATH $HOME/sys161/bin:$HOME/sys161/tools/bin:${PATH}

Note that setting these variables in .bashrc and .cshrc only works in the shell or window in which you issue the above commands in, and will most likely NOT work for logging in through SSH. You will need to ensure that the proper path is set and used for future logins and for other newly created shells.
A bit more information about setting up your shell.
Note that you may need to log out and log back in again so that this PATH change will take effect. You can check the current setting of the PATH environment variable using the command

printenv PATH


### Step 4: Install the GCC MIPS Cross-Compiler
Unpack the gcc archive:
tar -xzf os161-gcc.tar.gz
Move into the newly-created directory:
cd gcc-4.1.2+os161-2.0
Configure gcc
./configure -nfp --disable-shared --disable-threads --disable-libmudflap --disable-libssp --target=mips-harvard-os161 --prefix=$HOME/sys161/tools
Make it and install it:
make
make install
Step 5: Install GDB
Unpack the gdb archive:
tar -xzf os161-gdb.tar.gz
Move into the newly-created directory:
cd gdb-6.6+os161-2.0
Configure gdb
./configure --target=mips-harvard-os161 --prefix=$HOME/sys161/tools --disable-werror
Make it and install it:
make
make install
Note: Compiling this version of GDB may fail when newer versions of the texinfo package are installed (version 5.2-1, for instance). If you encounter an issue with this step, you can either include MAKEINFO=missing when you run the make command, or you can install an older version of texinfo (such as 4.13-4) and try again. You may also need to install the libncurses-devel library.

### Step 6: Install bmake
Unpack the bmake archive:
tar -xzf os161-bmake.tar.gz
Move into the newly-created directory:
cd bmake
Unpack mk within the bmake directory:
tar -xzf ../os161-mk.tar.gz
Run the bmake bootstrap script
./boot-strap --prefix=$HOME/sys161/tools
As the boot-strap script finishes, it should print a list of commands that you can run to install bmake under $HOME/sys161/tools. The list should look something like this:
mkdir -p /home/kmsalem/sys161/tools/bin
cp /home/kmsalem/bmake/Linux/bmake /home/kmsalem/sys161/tools/bin/bmake-20101215
rm -f /home/kmsalem/sys161/tools/bin/bmake
ln -s bmake-20101215 /home/kmsalem/sys161/tools/bin/bmake
mkdir -p /home/kmsalem/sys161/tools/share/man/cat1
cp /home/kmsalem/bmake/bmake.cat1 /home/kmsalem/sys161/tools/share/man/cat1/bmake.1
sh /home/kmsalem/bmake/mk/install-mk /home/kmsalem/sys161/tools/share/mk
Of course, your output will refer to your directories, not to /home/kmsalem.
Run the commands printed by boot-strap in the order in which they are listed.

### Step 7: Set Up Links for Toolchain Binaries
mkdir $HOME/sys161/bin
cd $HOME/sys161/tools/bin
sh -c 'for i in mips-*; do ln -s $HOME/sys161/tools/bin/$i $HOME/sys161/bin/cs350-`echo $i | cut -d- -f4-`; done'
ln -s $HOME/sys161/tools/bin/bmake $HOME/sys161/bin/bmake
Cygwin Users: Shells under Cygwin sometimes have difficulty handling shell commands in a string input as given above. If you find yourself unable to get the second command to work, copy and paste the entire second line into a shell script in $HOME/sys161/tools/bin and run it from there. You will likely need to change the permissions on the shell script to allow it to run, as follows:

chmod 755 myscript.sh
When you are finished with these steps, a listing of the directory $HOME/sys161/bin should look similar to this:

bmake@            cs350-gcc@        cs350-ld@       cs350-run@
cs350-addr2line@  cs350-gcc-4.1.2@  cs350-nm@       cs350-size@
cs350-ar@         cs350-gccbug@     cs350-objcopy@  cs350-strings@
cs350-as@         cs350-gcov@       cs350-objdump@  cs350-strip@
cs350-c++filt@    cs350-gdb@        cs350-ranlib@
cs350-cpp@        cs350-gdbtui@     cs350-readelf@

### Step 8: Build and Install the sys161 Simulator
Unpack the sys161 archive:
tar -xzf sys161.tar.gz
Move into the newly-created directory:
cd sys161-1.99.06
Next, configure sys161:
./configure --prefix=$HOME/sys161 mipseb
Build sys161 and install it:
make
make install
Finally, set up a link to a sample sys161 configuration file
cd $HOME/sys161
ln -s share/examples/sys161/sys161.conf.sample sys161.conf

### Step 9: Install OS/161
First, create a directory to hold the OS/161 source code, your compiled OS/161 kernels, and related test programs.
cd $HOME
mkdir cs350-os161
Next, move the OS/161 archive into your new directory and unpack it:
mv os161.tar.gz cs350-os161
cd cs350-os161
tar -xzf os161.tar.gz
This will create a directory called os161-1.99 (under cs350-os161) containing the OS/161 source code. You should now be able build, install, and run an OS/161 kernel and related application and test programs by following steps similar to those used to install OS/161 in the student.cs computing environment, starting with the step "Configure and Build OS/161".

### Step 10: Cleanup (optional)
Once you have completed the above steps, your OS/161-related development tools (binutils, gcc, gdb) and the sys/161 simulator will be installed under $HOME/sys161, and OS/161 itself will be installed under $HOME/cs350-os161. The archive files (filenames ending in .tar.gz) that you downloaded in Step 1 are no longer needed, so feel free to remove them if you want to save space or reduce clutter. You can also remove the directories in which you unpacked the archives and built the software: binutils-2.17+os161-2.0.1, gcc-4.1.2+os161-2.0, gdb-6.6+os161-2.0, bmake and sys161-1.99.06. However, do not delete the OS/161 source code (under $HOME/cs350-os161/os161-1.99), since you will be making use of it for your assignments.
Optionally, you may also remove the directory $HOME/sys161/tools/bin from your shell command path, as it is needed only during the process of building the toolchain. However, you be sure to leave $HOME/sys161/bin in your path, as that is the home of the toolchain binaries that you will be using as you work with OS/161.

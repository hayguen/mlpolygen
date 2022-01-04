
===============================================
mlpolygen - Maximal Length polynomial generator
===============================================

MLPolyGen is a command line tool for generating maximal length (ML)
polynomials for a linear feedback shift register (LFSR_).
MLPolyGen can generate ML polynomials of a specified order,
or test whether a specified polynomial is maximal length.
MLPolyGen can use the GNU Multiple Precision Arithmetic Library (GMP_)
to work with very large numbers. The larger the numbers, the more time
it will take to compute.

The home page for this package is https://github.com/hayguen/mlpolygen

Earlier, it was on https://bitbucket.org/gallen/mlpolygen/ , which no
longer exists. A backup can be found here: 
http://web.archive.org/web/20180820105651/https://bitbucket.org/gallen/mlpolygen


Building and Installing
-----------------------

This program is built with CMake_, the cross-platform, open-source build system.
Typically the program is not built directly inside the source directory.
My preference is to build the program in a directory named ``build``, located
in the root of the source directory.

.. _CMake: http://www.cmake.org/
.. _LFSR: http://en.wikipedia.org/wiki/Linear_feedback_shift_register
.. _GMP: http://gmplib.org/

On Ubuntu Linux, you can install GMP with::

 $ sudo apt-get install -y libgmp-dev

Get the sources::

 $ git clone --recursive https://github.com/hayguen/mlpolygen.git

In case you forgot the `--recursive` flag::

  $ cd mlpolygen
  $ git submodule init
  $ git submodule update
  $ cd ..

And build/install, following this approach::

 $ cmake -S mlpolygen -B build_mlpolygen -DCMAKE_BUILD_TYPE=Release
 $ cmake --build build_mlpolygen
 $ sudo cmake --build build_mlpolygen --target install

Normally, MLPolyGen requires GMP_ to build so that it can support numbers larger than 64 bits.
If GMP_ is present on your system, CMake will automatically detect and use it.
On Ubuntu 20.04.2 LTS, the required package is ``libgmp-dev``.
If you would like to disable the dependency on GMP_,
run CMake using the WITHOUT_GMP option as follows::

 $ cmake -S mlpolygen -B build_mlpolygen -DCMAKE_BUILD_TYPE=Release -DWITHOUT_GMP=1


For building with Visual Studio on Windows and without GMP you need some more modification, e.g.::

 $ cmake -G "Visual Studio 16 2019" -A x64 -S mlpolygen -B build_mlpolygen -DWITHOUT_GMP=1
 $ cmake --build build_mlpolygen --config Release

A pre-built executable for Windows should be available with github Actions.


Usage examples
--------------

This section contains some examples for some common use cases.
All of the options that are built in can shown by requesting help::

 $ mlpolygen -?


To generate all of the maximal length polynomials of a particular order (e.g. 16)::

 $ mlpolygen 16
 8016
 801c
 ...
 fff5
 fff6

These can also be generated using the symmetric pairs method.
This is about twice as fast as the linear method (because it only has to search
half the space), but it generates an unsorted output::

 $ mlpolygen -p 16
 8016
 b400
 ...
 fdbf
 fedf

To generate only few polynomials of a particular order::

 $ mlpolygen -n 4 16
 8016
 801c
 801f
 8029

To generate ML polynomials from a particular starting point::

 $ mlpolygen -s 0xff00
 ff12
 ff18
 ff24
 ff41
 ...

To generate a few *random* ML polynomials of a particular order::

 $ mlpolygen -r -n 4 16
 b354
 b5ab
 cca0
 8299

To test whether specified polynomials are maximal length::

 $ mlpolygen -t 0xb354 -t 0xb355
 0xb354 is maximal length for order 16
 0xb355 is NOT maximal length for order 16

Testing
-------

MLPolyGen includes code for testing its results.
The testing is currently implemented using a simple makefile along
with the stardard tools provided on a Unix system.
To run the test, build according to the above instructions and then::

 $ cd mlpolygen-1.x.x # where you put the source code
 $ cd test
 $ make

Refer to ``test/Makefile`` to see the tests performed, or increase the
order for which the tests are performed. Note that larger orders could
take hours (days, weeks) to complete.

To do
-----

- add a CLI switch to specify a stop polynomial value (so it could compute subsections in parallel)

- make sure it works on multiple platforms

- do some profiling to see if we can speed it up

- improve PrimeFactorizer to choose better prime candidates

- increase my CMake knowledge (I'm a noob)

- use CMake for testing (instead of the current Makefile)

Acknowledgements and Background (from Gregory Allen)
----------------------------------------------------

- Thank you to Philip Koopman for providing his page on ML LFSR polynomials: http://www.ece.cmu.edu/~koopman/lfsr/index.html

  - I've used his ML polynomials as reference material for a number of years

  - The mlpolygen tester uses his polynomials for verification

  - His page pointed me to ``lfsr_s.c``

- Thank you to the author of ``lfsr_s.c``; I believe it was authored by Scott Nelson

  - ``lfsr_s.c`` was once located at ``ftp://helsbreth.org/pub/helsbret/random/lfsr_s.c``

  - It contained no license when I downloaded it, and I can no longer find it on the internet

  - I've included an unmodified copy of ``lfsr_s.c`` in ``mlpolygen/src``

- mlpolygen is based on the algorithm described in ``lfsr_s.c``

- I wrote mlpolygen while examining ``lfsr_s.c``, so portions of mlpolygen may be very loosely based on ``lfsr_s.c``


Background and history (from Hayati Ayguen)
-------------------------------------------

- I took the backup/.zip from http://web.archive.org/ and put it online here,
  cause https://bitbucket.org/gallen/mlpolygen  was no longer available

- Thanks to all authors and supporters, especially Gregory Allen, Scott Nelson and Philip Koopman

- some of my additions:

  - get to compile with Visual Studio (without GMP) and the GitHub Actions infrastructure

  - 64 bit support for lfsr_s

  - replaced getopt by platform independent cargs, see https://github.com/likle/cargs

  - added some options: '-2', '-u shiftUp' and '-f' bruteForce - see usage


License
-------

MLPolyGen is released under the GNU_ General Public License (GPL_) version 3.
See the file ``COPYING`` for the full license.

.. _GNU: http://www.gnu.org/
.. _GPL: http://www.gnu.org/licenses/gpl.html


This file is part of MLPolyGen, a maximal-length polynomial generator
for linear feedback shift registers.

Copyright (C) 2012  Gregory E. Allen

Copyright (C) 2021  Hayati Ayguen


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

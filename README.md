  # CODERDBC 
  
  ## What is it

  CLI utilty for generating C code from dbc (CAN matrix) files

  ## Build and run

  This manual works on Ubuntu 20.04 and Windows 10. You need to ensure that your system has
  C++ compile and builing toolchain (**c++17**)

  To build coderdbc you need to make next steps:
  
  1 install cmake
  
  2 download source code:

  `git clone https://github.com/astand/c-coderdbc.git coderdbc`
  
  3 goto source code directory:

  `cd coderdbc`

  4 run cmake configuration to 'build' directory:

  `cmake -S src -B build`

  5 run cmake build:

  `cmake --build build --config release`

  6 goto to build directory and run:

  `cd build`

  `./coderdbc`

  Call without argument will print instruction how to pass DBC file for generation

  ## Driver functionality description

  The full pack of source code (both generated and manually edited) will be looked this
  (presuming that the dbc driver name is "ecudb"):
      
     ecudb.c / ecudb.h                             (1) RO / lib

    Main driver which has all dbc frames structs / pack functions / unpack functions these 
    source files preferably to place in lib level directory because they are RO using model
    and can be 
    shared among few separated projects.

      ecudb-fmon.h                                 (2) RO / lib

    Contains monitoring functions signatures which can be optionally called from unpack frame. 
    Best option to place file beside Main driver files (1).

      ecudb-fmon.c                                 (3) app

    User defined functions with diagnostic purpose. DLC, rolling, checksum errors can be handled 
    automatically if specific configuration enabled. This file is user level source code.

      ecudb-config.h                               (4) app / inc*

    This is application specific configuration file. If you have a few projects (applications) 
    which referenced on single main driver (1,2) then each project have to have own copy of this 
    configuration. Source code (1,2) includes this configuration. If a few dbc matrix is in use 
    in your application then for each of (1,2) specific configuration file must be presented.

      dbccodeconf.h                                (5) app / inc

    This is application specific configuration file. This file may include "CanFrame" definition,
    sigfloat_t typedef and binutil macros which enables rx and tx structures allocation inside 
    ecudb-binutil.c. This file must be single for application (see template dbccodeconf.h), source
    code (4,6) includes this configuration.

      ecudb-binutil.c / ecudb-binutil.h            (6) RO / app

    Basically this is application specific file. This driver has one function which intakes CAN
    message data and calls dedicated unpacking function. Function selection based on binary search. 
    
      canmonitorutil.h                             (7) lib

    This is lib level source code. This file is basic for all automatic monitoring functions. 
    This configuration file location have to be added to project include path.
    
    -----------------------------------------------------------------------------------------------

    *inc - file location have to be added to project include path.

  ## "-nodeutils" option

  If your matrix has strict routing setup, where each CAN device (node) has defined collection 
  of TX frames as well as defined collection of RX frames the "-nodeutils" option may be used.
  In this mode all the nodes defined in CAN matrix will be handled as specific ECU and 
  for each of these specific ECUs dedicated "###-binutil.c/h" pair of source code will be generated.
  
  See help output using details.

# CODERDBC 
  
Coderdbc is a CLI utility for generating C code from DBC CAN matrix files

### Features
- ***Pack*** and ***Unpack*** functions for conversion signals to CAN payload raw data and vice verse
- ***Node based*** Receive function _(each node (ECU) has its own ***Receive*** function according to its DBC configuration)_
- Automation on monitoring functions: CRC, counter and missing tests
- Optional source code generation _(the generation of readonly and configuration files can be avoided)_
- Flexible setup via driver configuration _(see comments in source code for details)_

## Build and run

For building project you need to have cmake and c++ development toolkit in your system
1 download source code:
```sh
git clone https://github.com/astand/c-coderdbc.git coderdbc
```
Go to the source code directory:
```sh
cd coderdbc
```
Run cmake configuration to 'build' directory:
```sh
cmake -S src -B build
```
Run cmake build:
```sh
cmake --build build --config release
```
Go to the build directory and run:
```sh
cd build
./coderdbc --help
```

Help information with main instructions about using the tool will be printed

## Driver functionality description

    The source code package includes the following source files (presuming that the dbc driver name is "ecudb"):
      
      ecudb.c / ecudb.h                            (1) RO / lib

    Pair of the main driver which contains all dbc frames structs / pack functions / unpack functions declarations. These source files preferably to place in the share/library directory. This part of the package is non-changable and has no any data, so can be used across multi projects.
    
      ecudb-fmon.h                                 (2) RO / lib

    Fmon header is a readonly part of monitoring part of the package. It contains the list of functions for CAN message validation. Those functions should be defined in the scope of user code and can be optionally used in unpack messages. This file is preferably to place in the share/library directory next to the main driver source files.

      ecudb-fmon.c                                 (3) app

    User specific part of monitoring functionality. If monitoring is fully enabled user code must define all the monitoring functions. This file is a part of the scope of user code.

      ecudb-config.h                               (4) app / inc*

    An application specific configuration file for enabling features in the main driver. If there are a few projects (applications) which include a single main driver (1,2) then each project has to have its own copy of this configuration. Source code (1,2) includes this configuration. If a few dbc matrix is in use in your application then for each of (1,2) specific configuration file must be presented.

      dbccodeconf.h                                (5) app / inc

    Application specific configuration file. This file might include "CanFrame" definition, sigfloat_t typedef and binutil macros which enables rx and tx structures allocation inside ecudb-binutil.c. Each project has to have its own copy of this configuration (see template dbccodeconf.h). Source code (4,6) includes this configuration.

      ecudb-binutil.c / ecudb-binutil.h            (6) RO / app

    The part which is used for generalization CAN frame flow receiving and unpacking. It also optionally can allocate CAN frame tx/rx structs. 
    
      canmonitorutil.h                             (7) lib

    General definitions for monitoring feature. The source file can be place to the share/library directory.
    
    -----------------------------------------------------------------------------------------------

    *inc - file location have to be added to project include path.

## generation options

  There are several available generation option, use '-help' option for details

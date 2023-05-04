  # CODERDBC 
  
  ## What is it

  CLI utilty for generating C code from dbc (CAN matrix) files

  ## Build and run

  Below setup works on Ubuntu 20.04 and Windows 10. You need to ensure that your system has
  C++ compile and builing toolchain (in my case for windows - VS Studio 2019 Community)

  To build coderdbc you need to make next steps:
  
  1 install cmake
  
  2 download source code:

  `git clone https://github.com/astand/c-coderdbc.git coderdbc`
  
  3 goto source code directory:

  `cd coderdbc`

  4 run cmake configuration:

  `cmake -S src -B build`

  5 run cmake build:

  `cmake --build build --config release`

  6 goto to build directory and run:

  `./coderdbc`

  Call without argument will print instruction how to pass DBC file for generation


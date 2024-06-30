# C-Coderdbc

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),

## [Unreleased]


## [v3.1] - 2024-06-30

### Added

- CAN FD frames with max DLC (64 bytes) are supported
- Minor type cast improvements in the sign extension function (\_\_ext_sig\_\_)
- (INFR) Test generation script takes one argument: release/debug to specify particular binary call
- istream handling improved in dbcscanner
- The head information with: coderdbc version, dbc file name ([issue #28](https://github.com/astand/c-coderdbc/issues/28))
- General source file (not driver related) have only coderdbc version and date if enabled (see next chages)
- The generation date can be added to head information with '-gendate' argument
- The final output directory path is extended by the driver name when '-driverdir' argument is passed 

### Fixed
- Head part info conversion to source code comments in mon-generator module
- FindVersion function improved
- Missing comments from dbc for signals based on real data types (floating-point variables, denoted by '_ro' and '_phys').

## [v3.0] - 2023-10-09

### Added

- Added explicit type casting on:
  * Writing data bytes, IDE, DLC and MsgId on pack step
  * Explicit unsigned nature ('U') to message CANID macro value
- ```-Wpedantic``` doesn't complain anymore on generated code (tested on test.dbc only)
- Added 'define' with value for filling initial value in frame's bytes before packing signals.
  The value can be set in the user scope configuration file (driver-config)
- Added enhanced DLC handling:
  * Added max detected DLC value (as a macro in main driver)
  * User can specify its own max DLC value if necessary (by setting corresponding macro in driver-config file)
  * Functions use new 'VALIDATE_DLC' test when checks the limit of frame data bytes count

### Changed

- Include files in quotes instead of angle brackets
- This changelog file format has been changed
- Added style option file
- Changed git flow
- README.md changed

### Fixed

- No more struct, pack and unpack for frames with no signals (empty frames). IDs and frame related info is left
- Fixed an issue in 'findversion' function
- Fixed issue on appending new string which is empty

## [v2.9] - 2023-01-27 

### Changed

- prt_double refactored

## [v2.8] - 2023-01-27

### Fixed

- Floating factor/offset values handling (incorrect rounding in some cases).

## [v2.7] - 2023-01-15

### Fixed

- Removed signal conversion macroses duplication ([issue #11](https://github.com/astand/c-coderdbc/issues/18))
- More precise floating factor/offset values handling ([issue #10](https://github.com/astand/c-coderdbc/issues/20))

### Added

- Better print for floating factor/offset values (removed tailing zeros)
- Signals with too low factor/offset values (less than 0.000000001) are considered as plain integer signals, it is up to client to handle them

## [v2.6] - 2022-09-29

### Fixed

- [issue #9](https://github.com/astand/c-coderdbc/issues/16) found by [@DPOH357](https://github.com/DPOH357)

## [v2.5] - 2022-07-26

### Changed

- GetSystemTick and GetFrameHash functions became macroses. It makes coupling lighter
and adds more flexibility to configure driver
- bitext_t and ubitext_t defined by default in dbccodeconf.h file
- App version is printed when generator runs (not only in help print case)

## [v2.4] - 2022-07-24

### Added

- Three new CLI keys to optionally disable some source code generation (see help)
- Added more tests

## [v2.3] - 2022-07-19

### Changed
- The gGeneration and the file configurations are splitted to different structs
- Specific generators moved to separated files (fmon, config)
- FileWriter API more simple

### Added
- FMon driver can be configured for using MONO function approach or
dedicated function (how it was before) by setting _USE_MONO_FMON macro

## [v2.2] - 2022-05-07

### Fixed
- "C valid name" check for main driver name and value table descriptions
- Fixed some minor printing artefacts (extra whitespaces, extra lines)
- Very strange issue with wrong naming through all bunch of __***-binutil__ drivers (WTF?)


### Added
- Values from value tables and it's descpriptions are printed as macroses
  ([issue#9](https://github.com/astand/c-coderdbc/issues/9) from [@delipl](https://github.com/delipl))
- Added more information about __\_\_ext_sig\_\___ function
- Added strong check of matching of the versions of secondary dbc source 
  files to main dbc source file (drvname.h)
- Sources which are presumed to be located in __include__ directory are in square brakets

## [v2.1] - 2022-02-19

### Fixed
- Some times incorrect _ro and _phys type deduction

### Added 
- Sign extension for signal which have signed type ([@2Black2](https://github.com/2Black2))

## [v2.0] - 2022-02-02

### Added
- '-rw' and '-nodeutils' (renamed '--node-utils') options

### Changed
- Argument passing way to view: 'key' - 'value'

### Fixed
- Minor warnings
- Bad source files placement for -rw key

## [v1.10] - 2021-01-24

### Changed
- Source files placed to separated directories
### Fixed
- Incorrect comments for valuetable's values
- Minor changes

## [v1.9] - 2021-11-09

### Fixed
- Closing last comment section in -config.h ([@SamFisher940425](https://github.com/SamFisher940425))
- A few minor style changes in generated code
- All sources of repo processed by code style formatting tool

## [v1.8] - 2021-11-01

### Fixed
- Issue #6. Incorrect checksum signal assigning.

## [v1.7] - 2021-10-10

### Fixed
- Potential issue when node is only Receiver (presumably will be skipped in node-util struct)

### Added
- Support multiple transmitters on single frame (for --node-utils generation variant)

## [v1.6] - 2021-09-09

### Added
- 4th CLI param '--node-utils' for generation pairs of binutil for each 
network node defined in DBC

### Fixed
- Bad *_Receive() function body when there is only 1 frame in RX struct

## [v1.5] - 2021-08-26

### Fixed
- Fixed 'atoi' with Extended ID on Windows OS ([@shevu](https://github.com/shevu))
- Fixed issue with parsing value table values ([@shevu](https://github.com/shevu))
- Fixed issue with frames where signals goes out of DLC range

## [v1.4] - 2021-07-13

### Fixed
- Removed default into "this is test"
- Edited README.md

## [v1.3] - 2021-07-11

### Added
- Printing template canmonitorutil.h
- Printing template dbccodeconf.h
- Updated driver-config comment text

## [v1.2] - 2021-07-11

### Added
- Option for rewriting generated files
- Added README.md (base build and run instruction)
- Added LICENSE.md

### Changed
- Lib has become CLI utility "coderdbc"
- Added help to CLI 
- Refactored project settings (cmake) to make easy way to build on windows PC

### Fixed
- Fixed some warnings

## [v1.0] - 2021-05-15

### Added
- Added DBC file version ("VERSION "x.x"") tag parsing
- Added dbc version info in: fmon, main and util drivers
- Added codegen lib version file

### Changed
- Generate interface takes DbcMessageList instance which has version info

### Fixed
- Fixed some warnings


[Unreleased]: https://github.com/astand/c-coderdbc/compare/v3.1...HEAD
[v3.1]: https://github.com/astand/c-coderdbc/compare/v3.0...v3.1
[v3.0]: https://github.com/astand/c-coderdbc/compare/v2.9...v3.0
[v2.9]: https://github.com/astand/c-coderdbc/compare/v2.8...v2.9
[v2.8]: https://github.com/astand/c-coderdbc/compare/v2.7...v2.8
[v2.7]: https://github.com/astand/c-coderdbc/compare/v2.6...v2.7
[v2.6]: https://github.com/astand/c-coderdbc/compare/v2.5...v2.6
[v2.5]: https://github.com/astand/c-coderdbc/compare/v2.4...v2.5
[v2.4]: https://github.com/astand/c-coderdbc/compare/v2.3...v2.4
[v2.3]: https://github.com/astand/c-coderdbc/compare/v2.2...v2.3
[v2.2]: https://github.com/astand/c-coderdbc/compare/v2.1...v2.2
[v2.1]: https://github.com/astand/c-coderdbc/compare/v2.0...v2.1
[v2.0]: https://github.com/astand/c-coderdbc/compare/v1.10...v2.0
[v1.10]: https://github.com/astand/c-coderdbc/compare/v1.9...v1.10
[v1.9]: https://github.com/astand/c-coderdbc/compare/v1.8...v1.9
[v1.8]: https://github.com/astand/c-coderdbc/compare/v1.7...v1.8
[v1.7]: https://github.com/astand/c-coderdbc/compare/v1.6...v1.7
[v1.6]: https://github.com/astand/c-coderdbc/compare/v1.5...v1.6
[v1.5]: https://github.com/astand/c-coderdbc/compare/v1.4...v1.5
[v1.4]: https://github.com/astand/c-coderdbc/compare/v1.3...v1.4
[v1.3]: https://github.com/astand/c-coderdbc/compare/v1.2...v1.3
[v1.2]: https://github.com/astand/c-coderdbc/compare/v1.0...v1.2
[v1.0]: https://github.com/astand/c-coderdbc/releases/tag/v1.0
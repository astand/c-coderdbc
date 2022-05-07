# Changelog

## v2.2 2022-05-07

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

---

## v2.1 2022-02-19

### Fixed
- Some times incorrect _ro and _phys type deduction

### Added 
- Sign extension for signal which have signed type ([@2Black2](https://github.com/2Black2))

---

## v2.0 2022-02-02

### Added
- '-rw' and '-nodeutils' (renamed '--node-utils') options

### Changed
- Argument passing way to view: 'key' - 'value'

### Fixed
- Minor warnings
- Bad source files placement for -rw key
***

## v1.10 2021-01-24

### Changed
- Source files placed to separated directories
### Fixed
- Incorrect comments for valuetable's values
- Minor changes
***

## v1.9 2021-11-09
### Fixed
- Closing last comment section in -config.h ([@SamFisher940425](https://github.com/SamFisher940425))
- A few minor style changes in generated code
- All sources of repo processed by code style formatting tool
***

## v1.8 2021-11-01
### Fixed
- Issue #6. Incorrect checksum signal assigning.
***

## v1.7 2021-10-10
### Fixed
- Potential issue when node is only Receiver (presumably will be skipped in node-util struct)

### Added
- Support multiple transmitters on single frame (for --node-utils generation variant)
***

## v1.6 2021-09-09
### Added
- 4th CLI param '--node-utils' for generation pairs of binutil for each 
network node defined in DBC

### Fixed
- Bad *_Receive() function body when there is only 1 frame in RX struct
***

## v1.5 - 2021-08-26
### Fixed
- Fixed 'atoi' with Extended ID on Windows OS ([@shevu](https://github.com/shevu))
- Fixed issue with parsing value table values ([@shevu](https://github.com/shevu))
- Fixed issue with frames where signals goes out of DLC range
***

## v1.4 - 2021-07-13
### Fixed
- Removed default into "this is test"
- Edited README.md
***

## v1.3 - 2021-07-11
### Added
- Printing template canmonitorutil.h
- Printing template dbccodeconf.h
- Updated driver-config comment text
***

## v1.2 - 2021-07-11
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
***

## v1.0 - 2021-05-15
### Added
- Added DBC file version ("VERSION "x.x"") tag parsing
- Added dbc version info in: fmon, main and util drivers
- Added codegen lib version file

### Changed
- Generate interface takes DbcMessageList instance which has version info

### Fixed
- Fixed some warnings

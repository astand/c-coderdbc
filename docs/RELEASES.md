# Changelog
All notable changes to this project will be documented in this file.

## v1.5 - 2021-08-26
### Fixed
- Fixed 'atoi' with Extended ID on Windows OS ([@shevu](https://github.com/shevu))
- Fixed issue with parsing value table values ([@shevu](https://github.com/shevu))
- Fixed issue with frames where signals goes out of DLC range


## v1.4 - 2021-07-13
### Fixed
- Removed default into "this is test"
- Edited README.md


## v1.3 - 2021-07-11
### Added
- Printing template canmonitorutil.h
- Printing template dbccodeconf.h
- Updated driver-config comment text


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


## v1.0 - 2021-05-15
### Added
- Added DBC file version ("VERSION "x.x"") tag parsing
- Added dbc version info in: fmon, main and util drivers
- Added codegen lib version file

### Changed
- Generate interface takes DbcMessageList instance which has version info

### Fixed
- Fixed some warnings

# Changelog
All notable changes to this project will be documented in this file.


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

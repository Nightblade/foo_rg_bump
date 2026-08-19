# Changelog

All notable changes to this project will be documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [1.1] - 2026-08-19

### Added

- `src/version.h` as the single source of truth for version, filename, and copyright strings.
- Clamping of adjusted ReplayGain values to the valid range (-51.0 dB to +51.0 dB).

### Changed

- `DECLARE_COMPONENT_VERSION` description now prepends `foo_rg_bump 1.1` above the copyright line.

## [1.0] - 2026-08-19

### Added

- Initial release.
- Keyboard-shortcut commands to increase or decrease ReplayGain track gain and/or album gain by a configurable step size.
- Advanced Preferences entry under Tools > RG Bump for step size and target tag.
- CI via GitHub Actions: build on every push, release on `v*` tags.

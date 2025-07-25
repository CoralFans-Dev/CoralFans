# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.5.0] - 2025-7-20

### Added

+ Adapted to 1.21.93

## [2.4.0] - 2025-7-20

### Added

+ Adapted to 1.21.80

### Changed

+ Noclip is independent command now, and it can keep working when player rejoin the world again.

## [2.3.1] - 2025-7-16

### Fixed

+ fix two bug of prof normal

## [2.3.0] - 2025-6-7

### Added

+ Adapted to 1.21.70
+ Now freeCamera will keep player's skin
+ Added new command cfhud removeall

### Fixed

+ add a feature that shortcut which include useless commonds will not be registed

## [2.2.1] - 2025-4-22

### Added

+ Add command FreeCamera (Thanks for GroupMountain)
+ Add command func/self PortalDisabled

### Fixed

+ Fix a bug about the permission level of commmand minerule

## [2.2.0] - 2025-3-7

### Added

+ Adapted to 1.21.60

### Fixed

+ Fix a bug that the village function may crash BDS when village is removed

### Removed

+ Remove command tick query

## [2.1.1] - 2025-4-20

### Added

+ Add command FreeCamera (Thanks for GroupMountain)
+ Add command func/self PortalDisabled

### Fixed

+ Fix a bug that the village function may crash BDS when village is removed
+ Fix a bug about the permission level of commmand minerule

## [2.1.0] - 2025-2-25

### Added

+ Adapted to 1.21.50
+ Added minerule command.
+ Added calculate command.
+ Added log rpt command.

### Changed

+ Thanks for Mojang, now hsa command only show the top side of Hsa.

### Fixed

+ Fix a bug that may cause lse not work correct.
+ Fix a bug when set tick rate repeatly.

## [2.0.0] - 2024-10-12

### Added

+ Added nopickup (tewakers)
+ Added container info in cfhud

### Changed

+ Config version changed to `3`

### Removed

+ Removed SimPlayer System (now is [CFSP](https://github.com/CoralFans-Dev/CFSP))

## [1.0.2] - 2024-10-11

### Fixed

+ Fixed SimPlayer System (#26)

## [1.0.1] - 2024-10-11

### Fixed

+ Fixed SimPlayer System (#24, #25)

## [1.0.0] - 2024-10-10

### Added

+ Added `coralfans version` command
+ Added data command (#6, #13, #19)
+ Added cfhud command (#8, #20)
+ Added autototem (tweakers) (#15)
+ Added autoitem (tweakers) (#15, #17)
+ Added fastdrop (tweakers) (#22)
+ Added containerreader (tweakers) (#10)
+ Added SimPlayer System (#5)
+ Added log command (#9)

### Changed

+ Config version changed to `2`
+ Added `try-catch` when loading config.json

### Fixed

+ Fixed BlockRotate (#11, #23)
+ Fixed position get (#18)
+ Fixed HitResult (#21)

## [0.0.1] - 2024-09-13

### Added

+ Added tick command
+ Added hsa command
+ Added hopper-counter
+ Added container-reader
+ Added auto-tool
+ Added village helper
+ Added noclip
+ Added slime-chunk helper
+ Added profiler helper (has bugs)
+ Added block-rotate
+ Added shortcuts
+ Added force-open
+ Added force-place
+ Added dropper-no-cost (has bugs)
+ Added safe-explode

[2.0.0]: https://github.com/CoralFans-Dev/CoralFans/compare/v1.0.2...v2.0.0
[1.0.2]: https://github.com/CoralFans-Dev/CoralFans/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/CoralFans-Dev/CoralFans/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/CoralFans-Dev/CoralFans/compare/v0.0.1...v1.0.0
[0.0.1]: https://github.com/CoralFans-Dev/CoralFans/releases/tag/v0.0.1

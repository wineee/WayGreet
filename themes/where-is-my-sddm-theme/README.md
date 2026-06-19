# Where is my SDDM theme? (WayGreet Port)

This theme is adapted for WayGreet from the original SDDM theme:
[where-is-my-sddm-theme](https://github.com/stepanzubkov/where-is-my-sddm-theme) by stepanzubkov.

### Changes for WayGreet:
- Replaced `SddmComponents` with `WayGreet` native APIs.
- Migrated `sddm` specific model calls to `Helper.userModel` and `Helper.sessionModel`.
- Migrated power actions to `PowerManager`.
- Inlined the configuration defaults natively in QML so it works out of the box without `theme.conf`.
- Removed SDDM specific assets (`metadata.desktop`, `theme.conf`, `example_configs`).

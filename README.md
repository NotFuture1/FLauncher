<p align="center">
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="/program_info/org.flauncher.FLauncher.logo-darkmode.svg">
  <source media="(prefers-color-scheme: light)" srcset="/program_info/org.flauncher.FLauncher.logo.svg">
  <img alt="FLauncher" src="/program_info/org.flauncher.FLauncher.logo.svg" width="40%">
</picture>
</p>

# FLauncher

FLauncher is a custom Minecraft launcher that lets you manage multiple installations, accounts, and modpacks at once — install modpacks and mods from Modrinth and CurseForge, keep every instance sandboxed in its own folder, and pick a Java version per instance.

It is a fork of [Prism Launcher](https://prismlauncher.org) (itself descended from PolyMC and MultiMC), rebranded and extended with our own changes.

## Building

FLauncher is C++ / Qt 6, built with CMake. Upstream's build documentation applies unchanged: <https://prismlauncher.org/wiki/development/build-instructions/>

### macOS quick start

```bash
brew install cmake ninja qt extra-cmake-modules
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
cmake --install build   # produces install/FLauncher.app
```

## License & credits

FLauncher is free software licensed under the [GPL-3.0-only](LICENSE) license, as a derivative of:

- [Prism Launcher](https://github.com/PrismLauncher/PrismLauncher) — © 2022-2026 Prism Launcher Contributors
- PolyMC — © 2021-2022 PolyMC Contributors
- [MultiMC](https://github.com/MultiMC/Launcher) — © 2012-2021 MultiMC Contributors

All upstream copyright notices are preserved in the source. This project is not affiliated with or endorsed by Prism Launcher, PolyMC, MultiMC, or Mojang/Microsoft.

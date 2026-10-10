# OPTO-1B v2 (AU / VST3)

Controls: Output, Ratio, Threshold, Attack, Release, Parallel,
Att/Rel (fixed/manual), Generation (vintage/modern), SC Low Cut (off/90 Hz),
Meter (input / compression / output). Lamp lights while compressing.

Build (macOS):
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
    cmake --build build --config Release -j
Plugins are copied to ~/Library/Audio/Plug-Ins/{Components,VST3}.

## Installer (.pkg)
After building:   ./package.sh      ->  dist/OPTO-1B-1.0.0.pkg
Or push this folder to GitHub: Actions tab -> "Build macOS installer" -> Run workflow
-> download the .pkg from the run's Artifacts.

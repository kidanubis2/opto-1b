#!/bin/bash
# Builds a macOS installer (.pkg) containing AU + VST3. Run AFTER building (see README).
set -e
cd "$(dirname "$0")"

VERSION="1.0.0"
ID="com.yourname.opto1b"
ART="build/OptoOne_artefacts/Release"

rm -rf pkgroot dist
mkdir -p pkgroot/Components pkgroot/VST3 dist

cp -R "$ART/AU/OPTO-1B.component" pkgroot/Components/
cp -R "$ART/VST3/OPTO-1B.vst3"    pkgroot/VST3/

# ad-hoc signature (no Apple Developer account needed)
codesign --force --deep --sign - pkgroot/Components/OPTO-1B.component
codesign --force --deep --sign - pkgroot/VST3/OPTO-1B.vst3

pkgbuild --root pkgroot/Components --identifier "$ID.au" --version "$VERSION" \
         --install-location "/Library/Audio/Plug-Ins/Components" dist/au.pkg
pkgbuild --root pkgroot/VST3 --identifier "$ID.vst3" --version "$VERSION" \
         --install-location "/Library/Audio/Plug-Ins/VST3" dist/vst3.pkg

productbuild --package dist/au.pkg --package dist/vst3.pkg "dist/OPTO-1B-$VERSION.pkg"
rm dist/au.pkg dist/vst3.pkg

echo "Done: dist/OPTO-1B-$VERSION.pkg"

# RosettaPrompter (JUCE VST3)

A JUCE-based VST3 teleprompter plugin for macOS Apple Silicon (arm64). It is a stereo in/out pass-through plugin with a lyrics editor, line highlighting, and tempo-synced auto-scroll based on Start/End bar calibration.

## JUCE submodule

JUCE is included as a git submodule at `./JUCE`.

Commands:

```
git submodule add https://github.com/juce-framework/JUCE.git JUCE
git submodule update --init --recursive
```

If you are cloning fresh:

```
git clone --recurse-submodules <repo-url>
```

## Local build (macOS arm64)

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release
```

The built plugin will be located at:

```
build/RosettaPrompter_artefacts/Release/VST3/RosettaPrompter.vst3
```

## Install the VST3

```
mkdir -p ~/Library/Audio/Plug-Ins/VST3
cp -R build/RosettaPrompter_artefacts/Release/VST3/RosettaPrompter.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

## Trigger CI build (GitHub Actions)

Tag a release and push the tag:

```
git tag v0.1.0
git push origin v0.1.0
```

The workflow triggers on tags matching `v*` and uploads a zipped VST3 artifact.

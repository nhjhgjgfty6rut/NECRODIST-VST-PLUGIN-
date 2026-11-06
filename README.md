# NECRODIST-VST-PLUGIN-
# NECRODIST

Distortion plugin (VST3) for the necrotrap / underground scene.
Made by VIOLENCE COLLECTIVE.

## Features

- 5 distortion modes: Soft, Hard, Fold, Asym, Fuzz
- Drive, Tone, Mix, Output
- ON/OFF button
- 4x oversampling (less aliasing)
- Smoothed parameters (no clicks on automation)
- DC blocker
- Full automation in FL Studio (right click on a knob)

## Download

Get the latest `NECRODIST.vst3` from the [Releases](../../releases) page.

## Installation (Windows)

1. Unzip the archive.
2. Copy `NECRODIST.vst3` to `C:\Program Files\Common Files\VST3`.
3. In FL Studio: Options -> Manage plugins -> Find installed plugins.
4. Add NECRODIST to a mixer insert slot.

Windows 64-bit, VST3 hosts only. Tested in FL Studio.
If the plugin does not load, install the latest Microsoft Visual C++ Redistributable (x64).

## Building from source

1. Install [JUCE](https://juce.com/) (built with 9.0.5) and Visual Studio 2019 or newer with the C++ desktop workload.
2. Open `NECRODIST.jucer` in Projucer.
3. Set the global JUCE path (File -> Global Paths).
4. Save the project and open it in your IDE.
5. Build the `NECRODIST_VST3` target in Release / x64.

## License

Licensed under the GNU Affero General Public License v3.0 (AGPLv3). See [LICENSE](LICENSE).

Built with [JUCE](https://juce.com/).

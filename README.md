# foo_rg_bump

A foobar2000 component that adjusts the `REPLAYGAIN_TRACK_GAIN` tag of the currently playing track by ±0.5 dB, triggered by user-assigned keyboard shortcuts.

## Usage

1. Install the component (drag `.fb2k-component` onto foobar2000, or use Preferences > Components > Install).
2. Open **Preferences > Keyboard Shortcuts**.
3. Click **Add New** and search for:
   - `RG Adjust / Track Gain +0.5 dB`
   - `RG Adjust / Track Gain -0.5 dB`
4. Assign your preferred keys and click Apply.

The commands operate on the **currently playing track**. If nothing is playing the keypress is a no-op. The tag is written to the physical file on disk.

## Building

The SDK is included as a git submodule. Clone with:

```
git clone --recurse-submodules https://github.com/YOUR_USERNAME/foo_rg_bump.git
```

Then open `foo_rg_bump.sln` in Visual Studio 2022 and build the Release|x64 configuration. The resulting DLL is at `x64\Release\foo_rg_bump.dll`.

### CI

Every push triggers a GitHub Actions build (`.github/workflows/build.yml`) that produces:
- `foo_rg_bump-x64` artifact (x64 DLL)
- `foo_rg_bump-x86` artifact (x86 DLL)
- `foo_rg_bump-component` artifact (`.fb2k-component` package containing both)

Download the `.fb2k-component` from the Actions run's artifacts to install.

## Notes

- foobar2000 2.0+ is x64. The x86 build is for legacy installs.
- The tag format written is `+N.NN dB` / `-N.NN dB`, matching the ReplayGain convention.
- If the tag is absent when you press the key, the component treats the current value as `0.00 dB` and writes `+0.50 dB` or `-0.50 dB`.
- Writing tags to the currently playing file works in practice but fb2k may briefly delay the write until playback of that file ends in some edge cases.

## License

BSD 3-Clause (same as the foobar2000 SDK).

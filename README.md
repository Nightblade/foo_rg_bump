# foo_rg_bump

A foobar2000 component that adjusts ReplayGain tags of the currently playing track or focused playlist item, triggered by user-assigned keyboard shortcuts.

## Usage

1. Install the component (drag `.fb2k-component` onto foobar2000, or use Preferences > Components > Install).
2. Open **Preferences > Keyboard Shortcuts**.
3. Click **Add New** and search for:
   - `RG Adjust / Track Gain +delta dB`
   - `RG Adjust / Track Gain -delta dB`
4. Assign your preferred keys and click Apply.

The component operates on all **selected playlist items** and the **currently playing track** if any.

## Configuration

Settings are changed via **Preferences > Advanced > Tools > RG Bump**.

- **Step size:** adjustment in tenths of a dB (e.g. 5 = 0.5 dB). Default: 5. Range: 1 to 200.
- **Target tag:** 0 = track gain only, 1 = album gain only, 2 = both. Default: 0.

Settings are saved persistently in foobar2000's configuration.

## Building

The SDK is included as a git submodule. Clone with:

```
git clone --recurse-submodules https://github.com/Nightblade/foo_rg_bump.git
```

Then open `foo_rg_bump.sln` in Visual Studio 2022 and build the Release|x64 configuration. The resulting DLL is at `x64\Release\foo_rg_bump.dll`.

### CI

Every push triggers a GitHub Actions build (`.github/workflows/build.yml`) that produces:
- `foo_rg_bump-x64` artifact (x64 DLL)
- `foo_rg_bump-x86` artifact (x86 DLL)
- `foo_rg_bump-component` artifact (`.fb2k-component` package containing both)

Download the `.fb2k-component` from the Actions run's artifacts to install.

## Download

The latest release is available on the [GitHub Releases page](https://github.com/Nightblade/foo_rg_bump/releases/latest). Download `foo_rg_bump-<version>.fb2k-component` and drag it onto foobar2000 to install.

## Notes

- foobar2000 2.0+ is x64. The x86 build is for legacy installs.
- If a ReplayGain tag is absent, the component treats the current value as 0.00 dB.

## Acknowledgements / Credits

- Peter Pawlowski for the [foobar2000](https://www.foobar2000.org/) audio player. ![foobar2000](https://www.foobar2000.org/button-small.png)
- Workflow and project structure inspired by [PEERSOFTdev/foo_musical_key](https://github.com/PEERSOFTdev/foo_musical_key).
- CI build approach informed by [jecassis/foo_vis_milk2](https://github.com/jecassis/foo_vis_milk2).
- Developed with assistance from Claude (Sonnet 4.6 Medium)

## License

BSD 3-Clause (same as the foobar2000 SDK).

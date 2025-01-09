# BB Studio Drag-n-Drop Game Creator for NES/Famicom

![BB Studio](sampleproject_nes_photo.jpg)

## What is this?

"BB Studio" is a heavily-hacked version of the awesome GB Studio by Chris Maltby. 

It replaces the Game Boy target with the NES (the Game Boy's "Big Brother").

You are strongly advised to already be familiar with the original GB Studio, before trying out BB Studio.

If you are looking for the original GB Studio, the friendly downloadables can be found [HERE](https://www.gbstudio.dev) and the source code can be found [HERE](https://github.com/chrismaltby/gb-studio).

## End User License Agreement

The current version is early-alpha software with all the bugs and warts that entails.

It is targeted at early adopters who like to tinker with experimental software.

It may crash-and-burn on you - and you shall remain patient and courteous.

Other than that, the MIT license [MIT license](https://opensource.org/licenses/MIT) applies.

## Where to get it

Download the latest alpha version from the [Release Page](https://github.com/michel-iwaniec/bb-studio/releases) for your operating system.

Load it up, and create a new project from the Sample Project template.

This Sample Project has been slightly tweaked for NES screen size / color restrictions. And the music re-created with FamiTracker + FamiStudio.

## Pre-requisite software to install

Both Mesen and FamiStudio depend on .NET runtime which will need installing.

You can download this from [HERE](https://dotnet.microsoft.com/en-us/download/dotnet/8.0)

On Linux you can use the package manager for a simpler installation process:

```
sudo apt install dotnet8
```

## How to use it

To run the game using the included Mesen fork, choose the "Game" -> "Run" menu option. After a bit of a wait, Mesen should start with the .nes ROM loaded.

To export a .nes ROM back to the project directory, choose the "Game" -> "Export As" -> "Export ROM". After a bit of a wait, your .nes ROM will appear in the build/rom sub-folder of your project.

To adapt your GBC game to the NES, you may need to do some changes.

There are a few key things to consider to make your GBC game run well on the NES.

* You have more screen space: 256x240 instead of 160x144. This can change the look and feel of your GBC game quite a bit, for better or worse.

* The edges of the screen will be partially cropped on a real TV - this varies depending on TV model.

* You can only use half the number of background and sprite palettes in a scene. (for now)

* NES background palettes also require the first color to be the same across all 4 background palettes.
- Typically black is a good choice for the shared background color - but your mileage may vary.
- The exception is palette 7. This is only used for UI, and has 4 unique colors.

* The NES has a limited number of total colors, so the palette editing has a dedicated color picker 
- If you load an existing project, an approximate RGB -> NES PPU color conversion will take place.
- From then on the project will always use NES colors.
- For best results always pick the colors carefully, and be aware they can vary across TVs
- Do not rely on perceptual brightness difference between hues on the same row. Shade using different columns.

* The NES graphics chip allows much fewer sprites / scanline than the GB/GBC. So you need to be much more frugal with placing sprites on the same horizontal line.

* The 6502 CPU may struggle to keep up with the double-clocked GBC CPU. If your game already tends to lag when running in monochrome DMG mode, it will probably struggle on the NES as well.
- More optimizations are planned for the next version.

## Running your built NES game

Your NES game can be run on an Everdrive N8 Pro, by placing your built .nes file in a subdirectory on the sdcard along with the provided bbstudio.rbf. This contains an Everdrive N8 Pro implementation of the custom mapper BB Studio requires.

For running on a PC you can instead use a customized version of the Mesen emulator [downloadable here](https://github.com/michel-iwaniec/Mesen2/releases/tag/Mesen2-with-bbstudio-mapper-v4).

This emulator is also currently bundled with BB Studio to make launching via "Game" -> "Run" easier.

### Using music in your project

BB Studio uses the FamiStudio sound engine, so any songs in your game need to be re-created with FamiStudio.

BB Studio currently uses FamiStudio 4.3.0, so please use that version to play it safe.

A future version may support auto-converting Game Boy songs to FamiStudio format. But re-making your song in FamiStudio will always be the recommended option for best results.

To make a FamiStudio .fms file replace a GB Studio file, simply put it into the assets/music directory of your project, with an identical name as the .uge file apart from the .uge extension being replaced with .fms instead.
The build process will then pick up your FamiStudio song in place of the .uge one.

FamiStudio also requires you to set the tempo mode to either "FamiStudio tempo" or "FamiTracker tempo". This is a global driver setting decided at build time.

The current setting is FamiTracker tempo, as all music files were originally in tracker format.

If you wish to use FamiStudio tempo instead, you can change this under "Music" -> "Music Tempo Mode" on the "Settings" page.

Make sure to check that *all* your songs use the same mode, or the sound engine will crash without mercy.

#### Sound effects / DPCM

SFX is not supported in this alpha version, but will be added soon.

DPCM is also not support in SFX nor music tracks yet.

## Putting your NES game on a stand-alone cartridge for sale

BB Studio uses a custom NES mapper to enable some GBC-like features such as 8x8 color attributes in an affordable way. 

Because the mapper is not 100% finished yet, .nes files built by BB Studio currently use a temporary mapper number of 248, to indicate the mapper is defined by the Everdrive N8 Pro .RBF file.

Sharing those files is not really recommended, as they may not work with the final version of the mapper.

Prototypes of the mapper have already been built and tested. One vintage-like discrete board using only simple TTL chips. 

And a more optimized board with all logic squeezed into a 32-macrocell CPLD.

The specifications for this mapper should be finalized in early 2025, with boards being available later this year.

## Something isn't working! How do I report it?

Please use the issue tracker on this github page... but only if the bug is specific to BB Studio rather than GB Studio :)

## How do I support this development?

Like GB Studio, BB Studio will always be free and open-source. But donations are much appreciated.

If you have money to spare, you can support Chris Maltby, the original creator of GB Studio on his Patreon page at https://www.patreon.com/c/gbstudiodev/

If you have even more money to spare after that, I also have a Patreon page at https://www.patreon.com/c/RetroCoding

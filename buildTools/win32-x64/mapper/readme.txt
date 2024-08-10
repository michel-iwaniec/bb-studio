The bbstudio.rbf file in this directory is an FPGA bitfile containing an early prototype version of a custom NES mapper.

It is provided to make running a .nes ROM from the drag-n-drop game creator BB Studio on an Everdrive N8 Pro more convenient.
If the .nes ROM and the .rbf file are placed in the same directory on the Everdrive's sdcard, the bitfile will be loaded onto the FPGA automatically before starting the game.

The bbstudio.rbf mapper is licensed under the GNU General Public License, and all source code is available here: https://github.com/michel-iwaniec/EDN8-PRO/blob/gbdk8x8_wip/mappers/007/map_098.v

A .nes ROM that may be present in the same directory is NOT licensed under the GPL, but owned by its creator and provided on their terms.

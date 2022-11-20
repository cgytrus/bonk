# bonk
play a bonk sound when you bonk your head against a solid object

## features
- mhv7 integration (see [below](#manual-configuration) if you wanna change settings but dont have mhv7)
- bonk
- bonk
- bonk
- bonk
- _\*gasp\* \*bonk\* \*dies\*_ "FUCJ"

## installation
1. put `bonk.dll` into your mod loader's folder:
   - mhv7: `extensions`
   - gdhm: `.GDHM/dll` on gdhm
   - quickldr: `quickldr` and add to the `settings.txt`
2. put `bonk.ogg` into the `Resources` folder
3. bonk !!!!

## manual configuration
- create a folder called `config` and a text file in it called `bonk.txt`
- default configuration is `1 1 1 0`
- 1 means on, 0 means off
- first number is whether the mod is enabled
- second number is 'bonk only on death'
- third is 'bonk on wave death'
- fourth is 'bonk on wave slide' (requires second to be off)

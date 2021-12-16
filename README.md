# Refraction
Refraction is an open-source Source 2013 mod.  
The mod at its current state is pretty much mapping-ready but there's still no concrete ideas about what levels to create.

## Differences from Valve's SDK
1. Refraction uses [QPC](https://github.com/quiverteam/QuiverProjectCreator) to generate the project files.
2. Refraction is Windows-only modification (Linux users can use Proton/Wine).
3. Refraction is not split into two branches, it works on SP version of Source.
4. The project structure is a bit different: it is made game-based, not engine-based.

## Features
1. Custom post-processing system allowing programmers to add custom screen-space post-processing effects easier than it was before. The `postprocess_controller` entity (replaces env_tonemap_controller) is responsible for toggling such effects.
2. Lots of fixes from [VDC](https://developer.valvesoftware.com) and from Pull Requests of Valve's SDK.
3. NovelUI allows programmers to easily create new GameUI elements without pain.
4. Something that I don't remember that good but it exists and it is awesome `:)`

## Miscellaneous
1. There's `openal` branch with an attempt to use OpenAL instead of Deferred Sound System.
2. I was planning on adding real shadow mapping (CSM-ish) to the mod but eventually gave up. May get back to it in the future.


## Licensing
Refraction is a dual-licensed project.  
* All the code that is left unchanged from the original Source Engine SDK
repo is licensed under the SDK license, see [LICENSE_SDK](LICENSE_SDK) for details.  
* Any kind of third-party code (lib and public) is licensed under its
appropriate license and possibly notified in [thirdpartylegalnotices.txt](thirdpartylegalnotices.txt).  
* All the remaining code that was directly contributed, unless explicitly
stated is licensed under the MIT license, see [LICENSE_MIT](LICENSE_MIT) for details.  

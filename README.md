# REFRACTION
Refraction is an open-source Source Engine 2013 mod aimed for adding some Quality-of-Life changes and fixes to the SP SDK.  
At its current state Refraction is about 75% compatible with HL2 and Episodic maps.  

## Core features
1. Replaced VPC with [QuiverProjectCreator](https://github.com/quiverteam/QuiverProjectCreator).
2. Dropped Linux and OSX support for the sake of code simplicity.
3. Small UTL changes and fixes (such as better C++ comment parsing).
4. Restructured file hierarchy in order to make the code to be more game-oriented.

## Game features
1. A fully customizable GameUI replacement (NovelUI).
2. A custom post-processing system allowing programmers to define custom screen-space effects. The `postprocess_controller` entity (replaces `env_tonemap_controller` and is partially compatible with its KV signature) allows toggling those effects (configuring values in a non-hacky way is TBA).
3. Lots of small fixes from [VDC](https://developer.valvesoftware.com) and [SDK pull requests](https://github.com/ValveSoftware/source-sdk-2013/pulls).
4. Improved forward compatibility of HL2 maps running on Episodic builds (some entities were irreversibly changed making some maps unplayable; two of the numerous examples would be jeep entities - buggy was not working correctly, and mounted machine guns - they weren't working at all on HL2 maps).
5. Quake-styled (or HL1-WON-styled) view rolling when strafing with customization.
6. Unlocked Field of View (FOV). I recommend setting it at 105 for widescreen.
7. Custom VGUI color scheme and fonts for the console.

## Optional and discontinued features
1. OpenAL sound system (`openal` branch) - discontinued.
2. Deferred sound system (`deferred-sound` branch) - put on hold.

## Planned features
1. Shadow mapping
2. ????

## Licensing
Refraction is a dual-licensed project.  
* All the code that is left unchanged from the original SDK
repo is licensed under the SDK license, see [LICENSE_SDK](LICENSE_SDK) for details.  
* Any kind of third-party code (lib and public) is licensed under its
appropriate license and notified in [thirdpartylegalnotices.txt](thirdpartylegalnotices.txt).  
* All the remaining code that was directly contributed, unless explicitly
stated is licensed under the MIT (Expat) license, see [LICENSE_MIT](LICENSE_MIT) for details.  

**In short:** you may use the code as long as you have LICENSE_MIT added to your source and binary tree via thirdpartylegalnotices.txt

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
**NOTE:** this may change in future! I don't want to stick with licensing _my_ work under Valve's license!  

Any third-party library or code used by Refraction code or tools is noted in the [thirdpartylegalnotices.txt](thirdpartylegalnotices.txt) file.  
Refraction code (as long as not explicitly noted) and assets present in the repository (as long as not explicitly noted) are licensed under Valve's SDK license ([LICENSE](LICENSE)):
```
               SOURCE 1 SDK LICENSE

Source SDK Copyright(c) Valve Corp.  

THIS DOCUMENT DESCRIBES A CONTRACT BETWEEN YOU AND VALVE 
CORPORATION ("Valve").  PLEASE READ IT BEFORE DOWNLOADING OR USING 
THE SOURCE ENGINE SDK ("SDK"). BY DOWNLOADING AND/OR USING THE 
SOURCE ENGINE SDK YOU ACCEPT THIS LICENSE. IF YOU DO NOT AGREE TO 
THE TERMS OF THIS LICENSE PLEASE DON'T DOWNLOAD OR USE THE SDK.  

  You may, free of charge, download and use the SDK to develop a modified Valve game 
running on the Source engine.  You may distribute your modified Valve game in source and 
object code form, but only for free. Terms of use for Valve games are found in the Steam 
Subscriber Agreement located here: http://store.steampowered.com/subscriber_agreement/ 

  You may copy, modify, and distribute the SDK and any modifications you make to the 
SDK in source and object code form, but only for free.  Any distribution of this SDK must 
include this LICENSE file and thirdpartylegalnotices.txt.  
 
  Any distribution of the SDK or a substantial portion of the SDK must include the above 
copyright notice and the following: 

    DISCLAIMER OF WARRANTIES.  THE SOURCE SDK AND ANY 
    OTHER MATERIAL DOWNLOADED BY LICENSEE IS PROVIDED 
    "AS IS".  VALVE AND ITS SUPPLIERS DISCLAIM ALL 
    WARRANTIES WITH RESPECT TO THE SDK, EITHER EXPRESS 
    OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED 
    WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, 
    TITLE AND FITNESS FOR A PARTICULAR PURPOSE.  

    LIMITATION OF LIABILITY.  IN NO EVENT SHALL VALVE OR 
    ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
    INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER 
    (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF 
    BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF 
    BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) 
    ARISING OUT OF THE USE OF OR INABILITY TO USE THE 
    ENGINE AND/OR THE SDK, EVEN IF VALVE HAS BEEN 
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
 
       
If you would like to use the SDK for a commercial purpose, please contact Valve at 
sourceengine@valvesoftware.com.
```

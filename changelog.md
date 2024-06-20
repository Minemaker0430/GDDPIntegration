# v1.0.0-beta.25

* Added missing nullptr checks (Thanks Alphalaneous!)

# v1.0.0-beta.24

* Fixed a weirdly specific crash
* Converted Stats Menu tabs to ButtonSprites instead of whatever I did
* Pages API Compatibility
* Fixed some sprite alignment issues
* Fixed Settings Page crash
* Attempted to fix save file bugs
* ABSOLUTE PERFECTION now shows the correct percentage

# v1.0.0-beta.23

* Fixed Crash
* Fixed a few Graphical Errors

# v1.0.0-beta.22

* Ported to 2.206
	* Fixed a visual bug with progress bars
	* Ported web requests to the new system
* Added the Stats Menu
* Partially Completed/Completed Monthly Packs now have Epic Fire

# v1.0.0-beta.21

* Added Progress Bars
	* Progress Bar colors in the Lists now match the ones in the menus
* Added as many Node IDs to the GDDP Stuff as I could
* Patched up Beta 20 stuff
	* Insane/Extreme Demon ratings now rely on GDDL Tiers as opposed to 3 seperate APIs
* Fixed Main Menu related issues (I think)

# v1.0.0-beta.20

* Added skillset badges
* Added failsafes for missing textures
* Fixed coin icons in LevelBrowserLayer causing visual bugs
	* This also sets an ID for the Grandpa Demon face, let me know if this conflicts with another mod
* Added an option to enable GDDP Ratings for all difficulties **\([Made by wint0r](https://github.com/wint0r), tweaked by Me\)**
	* Uses IDS, NLW, and Pointercrate API
* Added some more options
* Fixed Disable Grandpa Demon Effects (I think)

# v1.0.0-beta.19

* Fixed save bugs
	* Main packs no longer get rid of ranks if the pack is completed
	* Monthly packs no longer are completed at 0/6
* Fixed Legacy Lock Label

# v1.0.0-beta.18

* Completely Overhauled the save system
	* Save data is now tied to List IDs rather than the ID of the pack in an array
		* This means packs that are added/removed will no longer affect previous data like it once did
		* This is also a simpler method and allows save data to be built off of easier since each value now uses the same structure
	* Saved values are now a proper JSON as opposed to a bunch of arrays
	* Old save data will be unused in favor of the new system
* Monthly Packs now have a "+" suffix if partially completed
* Changed string formatting since I was doing it in a stupid way before

# v1.0.0-beta.17

* Data Caching is now disabled by default, can be enabled in settings
* Updated the "Custom Difficulty Faces" description to not include misinformation (i forgot to update it)
* Packs can now be inserted into any position instead of just beginning/end
* Custom Difficulty Faces can now be enabled outside of the GDDP Menus

# v1.0.0-beta.16

* Attempted to patch out all Save File bugs
* Fixed things crashing in super specific cases
	* Fixed crash when no packs are in a list
	* Fixed custom sprites not functioning with DP_Invisible
* Months are now Color Coded
* Added a Support Button where you can donate to my Ko-Fi c:
* Changed the way new Bonus Pack additions function
* Modified the Lock text to say what rank you need (will not show further ranks)

# v1.0.0-beta.15

* Changed Menu Icon to look more like a regular GD Menu Icon
* Fixed Practice Packs (again)
* Fixed Practice Packs not showing GDDP Graphics
* Added "(Practice)" suffix

# V1.0.0-beta.14

* Main & Legacy Practice Packs are separated now
* Fixed Practice Packs overwriting progress
* Made a very tiny adjustment to the Map Pack replacement button

Side Note:
* Yes, I know nobody has access to Practice Packs yet but I'm doing stuff in preparation.

# v1.0.0-beta.13

* Fixed crash related to level lists
* New config option to replace Map Packs
	* Prioritizes Compact Menu Button over this option

# v1.0.0-beta.12

* Fixed Visual Bug involving Levels in Bonus and Monthly Packs
* Non-GDDP Lists don't get visuals applied to them anymore

# v1.0.0-beta.11

* Prevented Crashes related to fetching pack data
* Added Pack Descriptions
* Difficulty Faces are now collected individually instead of being based on the current pack
	* Fixes incorrect difficulties
	* Fixes difficulties showing on Non-GDDP levels
	* Allows for Bonus Packs and Monthly Packs to show difficulties
	* Not all levels will have the individual difficulties at the start, please be patient with me
* Mod no longer accesses depreciated database values
* Progress evaluation should be significantly better
* Fixed a minor bug where the mod wouldn't detect you as outside of the GDDP menus

Known Issues:
* Due to the way the Grandpa Demon compatibility works currently, when using Disable Effects...
	* Projected Stars particles may be hidden
	* Some effects may not be hidden on some levels

# v1.0.0-beta.10

* Fixed Custom Difficulty Faces in Legacy Packs
* Modified Practice Pack Implementation (will be available later)

# v1.0.0-beta.9

* Added Custom Difficulty Faces (finally)
	* Only applied to Main/Legacy Levels (for now)
	* Grandpa Demon Support w/ Included Config Options
* Heavily Optimized Textures
* Added Setting Sections
* Added Compact Menu Button

# v1.0.0-beta.8

* Fixed Compatibility Issues (thanks SpaghettDev for your help)

# v1.0.0-beta.7

**This mod is now an official part of the GDDP. Thank you for your support! <3**

* Main Pack Completions now show the Plus Sprite like they should've been
* Legacy Packs are now locked behind Main Ranks (can be toggled off)
* Better Progress Evaluation (might fix mobile crashes)
* Rewrote the "plus" name display (Demons + -> + Demons)
* Fixed the Loading Circle showing up on the Creator Menu instead of the GDDP Menu
* Modified the Custom List Menu

# v1.0.0-beta.6

* Added Practice Versions of the Main Tiers. (Will be available later)
* Added a "+" next to tiers you have the rank for.
* Completed Packs now use a Gold Font
* Added Triple Spikes /\\/\\/\\ <3
* Added Mod Tags

# v1.0.0-beta.5

* Fixed Progression not letting you advance even though you have the required levels

# v1.0.0-beta.4

* Fixed Mobile UI
* Added a new Bonus Pack
* Fixed List Disappearing after exiting a Level

# v1.0.0-beta.3

* Added Bonus & Monthly Packs (Will Be Available Later)
* Modified Level Lists and Level Info for GDDP Packs/Levels
* Added Progress Tracking
* Added more config options
* Custom Difficulty Faces will be added in the future when it's compatible with Grandpa Demon

**Thank you guys for 7K Downloads <3**

# v1.0.0-beta.2

* Added Legacy Packs
* Added an Alternate Menu Button (Can be toggled in the settings)
* Made Pack Data Fetching Asynchronous
* Optimized Mod Textures

# v1.0.0-beta.1

* New, more reliable system for getting Lists
* Changed the BG color of the GDDP Menus

# v1.0.0-alpha.1

* Initial Release, adds a new button to the Creator Menu with all Main GDDP Lists.

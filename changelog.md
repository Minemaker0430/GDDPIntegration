# v1.0.6

* Readded Practice Packs since they can now be actively maintained

# v1.0.5

* Fixed levels being moved to Legacy still having XP
* Fixed link in support popup

**Thank you all for 500K Downloads <3**

# v1.0.4

* Updated my username and all references to my old username
* Added Unique Placeholder Pack Sprite
* Added Sprites for some Special Skillset Badges
* Fixed a visual bug involving the "Titles" section of the Stats menu

# v1.0.3

* Fixed levels being marked as completed when they weren't

# v1.0.2

* Attempted to fix a weird mobile crash
* Added some options relating to the side buttons of the GDDP Menu
* Added an option to disable the monthly timer
* Added Descriptions for each Setting Section
* Fixed Aspect Ratio and Z Ordering Issues

# v1.0.1

* Added sprite for 12 Demons of Christmas (2024)

# v1.0.0

This is now the definitive version of the mod. It took me over 8 months to program, and several years for the mod to come into fruition, but it's finally done. I'd like to thank everyone, especially the GDDP Staff, who supported me along the way. More updates will come eventually. Thanks for playing. <3
~ Mocha

* Renamed the mod (GDDP Integration -> GDDP - Demon Progression)
* Added the Roulette Menu
	* Select Packs to use in it
	* Can be Random Seed or Set Seed
	* Extra Challenge Modes included
	* Export and share your Saves and Settings with friends
* Removed Practice Packs and Rate All Demons
	* Practice Packs are too hard to maintain, all startpos levels have to be official, so when new levels get added/replaced, it gets confusing
	* Rate All Demons isn't very useful or necessary now, with the introduction of mods like "Fake Rate" and "Demons in Between"
* Various Bugfixes
	* Futureproofed some stuff
	* Fixed some crashes
	* Searching should be faster
	* Minor XP Speed Improvements
* Quality of Life Improvements
	* The Mod will now try to get completed GDDP Levels from your save file
	* Packs will now detect whether or not levels were completed outside that respective pack
	* Recommendations now check for new ones on refreshing the menu
	* Added time until next Monthly Pack

# v1.0.0-beta.47

* Reorganized Settings
* Updated for GD 2.207 & Geode 4.0
* Added a new setting that moves skillset badges on the Level Info page
* Added a new setting that lets you hide Bonus levels outside of GDDP Menus 

# v1.0.0-beta.46

* Optimized the Recommendations and XP System a bit
* Futureproofed the Recommendations and XP System
* Fixed a minor detail I missed with the Swing XP update, whoopsies!
* Added a new option that lets you hide Monthly levels outside of GDDP Menus
* Tweaked text in the Ranks section of Stats
* Added Compact Lists compatibility (kinda)
* Fixed Current Monthly BG Extending past where it was supposed to
* Monthly Packs now have the Red Progress Bar before Partial Completion
* Monthly Packs now have a subtle background color based on year
	* Can be turned off in settings
* Updated for Geode 3.9
* Tweaked settings a bit

# v1.0.0-beta.45

* Added the Search Menu
	* Includes Filters for difficulties, packs, skillsets, and XP.
	* Sort Results by Pack, Difficulty, Name, or Age (and reverse results as well)
* Fixed some bugs
	* Fixed a bug with Recommendations
	* Fixed a visual bug with skillset badges

Known Issues:
* Rate All Demons is still broken, don't know when I'll fix it :v
* Saved levels sometimes crashes, try going into the GDDP Menu to refresh data and see if that fixes it

# v1.0.0-beta.44

* Fixed a crash
	* Recommendations now always refresh XP to prevent this from happening

# v1.0.0-beta.43

* Fixed Recommendations not recognizing Swing XP
* Ported settings to the new system
* Recommendations now start generating upon beating a level in the GDDP
* Fixed a visual bug involving Stats

# v1.0.0-beta.42

* Added Swing as a new Skill XP
* Removed News since it's kinda useless

# v1.0.0-beta.41

* Fixed a crash involving exiting lists before levels load
* Tried patching up a few crash spots

# v1.0.0-beta.40

* Fixed a major crash and fixed cache not caching due to an oversight

Known Issues:
* Visual bug involving Rate All Demons
	* Literally no clue why it's happening, please help

# v1.0.0-beta.39

* Added the Recommendations Menu
	* Works roughly similar to how the website does it
	* Expect bugs, this is still in beta
	* Recommended Levels have a special appearance (can be disabled)
* Fixed some crashes
* Added an option to lower the Demon XP button
* Lessened the amount of annoying popups
* Fixed some Sprite Alignments

# v1.0.0-beta.38

* Added a Skill XP System based on the one in the GDDP Website
	* You can view all your XP progress on the main GDDP Menu
	* GDDP Levels now have an XP button on the Level Info page
	* While the Max Levels don't match the website, the Level Scaling does.

# v1.0.0-beta.37

* Added a support button
	* Also added a setting to disable it
* Added news
* Fixed a weird crash

# v1.0.0-beta.36

* Fixed Titles not showing correctly
* Having a rank without having the requirements overwrites percent progress for Titles

# v1.0.0-beta.35

* Fixed Rate All Demons
	* Spreadsheet Parsing Code borrowed from hiimjustin
* Still investigating mobile crashes

# v1.0.0-beta.34

* Fixed Various Visual Bugs
	* Rate All Demons is broken due to API Changes, will fix later.
* GDDP Checks no longer apply to Weekly or Gauntlet Levels

# v1.0.0-beta.33

* Fixed a crash

# v1.0.0-beta.32

* Added more crash checks
* (Hopefully) fixed mobile crashes
* Rank progress in the Stats Menu are now clamped

# v1.0.0-beta.31

* Added JSON Validation checks to prevent crashes.
* Depreciated "listID", "practiceID", and "totalLevels"
* You will maintain your progress to a rank even if a level is moved to Legacy
	* Note that this does not apply for Plus Ranks.

# v1.0.0-beta.30

* Attempted to fix crashes (again)

# v1.0.0-beta.29

* The mod now tells you to refresh your data if it's outdated and won't crash on saved levels

# v1.0.0-beta.28

* Attempted to fix some mobile crashes
* Fixed Titles having incorrect requirements
* Fixed a bug involving Your Plus Title in the Stats menu

# v1.0.0-beta.27

* Redid database systems
	* Does not use in game lists anymore (significantly faster & easier to update)
	* Comments are now disabled on packs
	* Any previous saves will be automatically converted
	* Obtaining levels in a pack is much easier
	* Total Levels is now counted dynamically instead of being a number on the database
* Added ADEPT Title
* Various bugfixes
	* Fixed crash when viewing stats with Obsidian or Obsidian+
	* Fixed progress not registering
	* Hopefully fixed crashes related to entering packs

# v1.0.0-beta.26

* Fixed Settings Page Crash (again)

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
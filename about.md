# Pass-Indicators

This mod adds a <cr>Death Effect</cr> that shows the exact positions where you should have clicked to <cg>Survive</cg>. The mod <cy>automatically</cy> tracks the positions of which clicks as you play the level. It is basically <cy>exact jump indicators for every level</cy> which is intended to help you both <cy>learn</cy> and <cg>understand</cg> the level Faster. Since it is a death effect is should be <cg>Allowed</cg> for any leaderboard or list!

## Main Features
- A death effect showing where clicks and releases lead to survival. 
- As you play the level a "indicator grid" gets drawn based on data from your gameplay. 
- Any press or release performed on your run is visualized as a green dot centered to where your icon was. 
### Minor Features
- A keybind to show the visualizer even when alive. (This is obviously not allowed for List Completions).
- Customization for the colors in the mod settings.
- If paused you can hover with your mouse over a horizontal timing line to see the exact width and timing window(these get extremely accurate over time). Mods like [Zoooom!](mod:bobby_shmurner.zoom) or [Megahack](mod:absolllute.megahack) are recommended to hide the pause menu!
### Experimental features
- If you die before your intended input, this mod allows you to see how late that input was and a green dot will appear where you would have been positioned, had you not died, based on your velocity. This only works correctly on mouse, you can rebind your keyboard input key to be a mouse click if you want this to work on keyboard. This option is off be default but quite cool if you want to check it out(if any mod developer knows how to hook clicks and releases after death, hit me up).
## How to Share Data
You can share the timing windows you've found for the level by sending your level data. Go into the settings on the mod page, click the bottom right "save" button, go into the "levels" folder and in there the names of the files match the level ids. Copy that file for a level and you can send that to a friend. You can also copy and rename the data from one level to the id of another level to move the data there. In a future update I may fix a more elegant solution for this.

Ideally a friend of yours or someone online would send you a .json file with a "indicator grid" that is already finished. Feel free to post your .json files online for others to use!
## Notes
CBF or CBS is recommended for more accurate indicators(with CBF or CBS the timing window will converge to the exact value over time). Keep in mind that the accuracy of the "indicators" get more accurate over time as you play the level. The positioning of the "indicators" are placed on a grid and on slow speed there are 5 grid elements per frame on 240 hz. It is very precise and is only more precise on higher speeds.

The "indicators" don't account for your momentum into the click so in gamemodes like ship and swing, you may die even if you press exactly where you survived earlier.

When using the hover feature to see how wide a timing is, the formula for the timing window is based on the speed of the player when they paused. Make sure to be the same speed as the timing you are inspecting.
## Future Features
I would like to implement a way to link levels together so they share data, in case you have multiple copies of the same level. It would also be cool to have a database with already finished "indicators" for extreme demons but that is a huge feature that is not in the works for now. Feel free to post your .json files with indicator data online for others to use! Maybe in youtube completion descriptions? 

## Special Thanks
Huge thanks to Techstudent10! Check out their mod [Jump Markers](mod:techstudent10.jump_markers) which was a huge inspiration and helped me find the correct hooks to use here. If you want a more animated Jump Marker, consider trying it! I built upon their idea when making this mod.

Check out my [youtube](https://www.youtube.com/@Mackan-gd) and [Twitter](https://x.com/yherherherherh) where I will post about future mods(along with occasional completions). There are a couple more cool mods I'm working on(next up is a accurate winrate mod). 
# Cog
Cog are debug tools for Unreal Engine built on top of [ImGui](https://github.com/ocornut/imgui)

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/21659aea-2cd8-4ef6-b3b0-5795f5f3246b)

## Project Structure

- `CogSample` - A Sample that demonstrate various Cog functionalities. The project was saved in Unreal 5.1
- `Plugins/CogAbility` - ImGui windows for the Gameplay Ability System (Abilities, Effects, Tags, ...)
- `Plugins/CogInput` - ImGui windows for the Enhanced Input library (Input action, Gamepad)
- `Plugins/CogEngine` - ImGui windows for the core unreal engine functionalities (Log, Stats, Time, Collisions, Skeleton, ...)
- `Plugins/CogWindow` - ImGui window management (Base Window, Layout)
- `Plugins/CogDebug` - Debug functionalities (Log, Debug Draw, Plot, Metric, ...)
- `Plugins/CogImGui` - Integration of Imgui for Unreal, inspired by [UnrealImGui](https://github.com/segross/UnrealImGui)
- `Plugins/CogInterface` - Interfaces implemented by actors to inspect them

## ImGui Windows

- Most windows display their contents based on a selected actor.
- The selector actor can be chosen using the selection window or widget.
- mouse over a window title to display its help. This behavior can be disabled in the Window menu.
- Window Layouts can be saved/loaded.
- Windows save their own settings in a Cog.ini file to maintain their state between multiple sessions.

### Abilities
Displays the gameplay abilities of the selected actor.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/5eece87c-eba3-45f1-95c5-e132be2133b8)
- Click the ability checkbox to force its activation or deactivation.
- Right click an ability to open or close the ability separate window.
- Use the 'Give Ability' menu to manually give an ability from a list defined in a data asset.

### Actions
Displays the state of Input Action.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/6323e78b-2ee4-43e2-bec6-19aa15716d2c)
- Can be used to inject inputs to help debugging.
- The display input action are read from a Input Mapping Context defined in a Data Asset.

### Attributes
Displays the gameplay attributes of the selected actor

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/a6329ef1-f775-4e6e-9581-6389f9f4b39c)
- Attributes can be sorted by name, category or attribute set.
- Attributes with the Current value greater than the Base value are displayed in green.
- Attributes with the Current value lower than the Base value are displayed in red.
- Use the options 'Show Only Modified' to only show the attributes that have modifiers.

### Cheats
Used to apply cheats to the selected actor.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/b7b89635-7924-49b7-98c0-311199947dfc)
- The cheats are gameplay effects configured in a data asset.
- Key modifiers can be used to change which actor should be affected by the cheat:
- [CTRL] apply the cheat to the controlled actor
- [ALT] apply the cheat to the allies of the selected actor
- [SHIFT] apply the cheat to the enemies of the selected actor

### Collisions
Used to inspect collisions by performing a collision query with the selected channels. 

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/ccee855b-b33b-4dfc-ba14-dff87e1150cf)
- The query can be configured in the options.
- The displayed collision channels can be configured in a Data Asset.

### Debug Settings
Used to tweak how the debug display is drawn.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/e55c8d07-133e-48ab-a974-ed076f3d4a58)

### Effects
Displays the gameplay effects of the selected actor.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/4fffa8fe-bd93-44bc-8ca5-27ae81f5da0c)
- Mouse over an effect to see its details such as its modifiers, the gameplay tags it grants, the remaining time. 

### Gamepad
Display the state of the gamepad

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/734dd08a-5b9f-44cf-8d24-9bf257f08ec6)
- Can be used to inject inputs to help debugging.

### Log Categories
This window can be used to activate and deactivate log categories

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/7ced2751-c3bc-43a3-9280-d11a34bf0482)
- Activating a log category set its verbosity to VeryVerbose.
- Deactivating a log category set its verbosity to Warning.
- The detailed verbosity of each log category can shown by using the Option menu.
- A client can modify the verbosity of the server.
- The log categories are used to display both output log and debug display in the world.

### Metric
This window gather values generated by the selected actor to compute how much output it produces or receives per second. This is typically useful to compute the damage dealt per second, the damage received per second, etc. 

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/64d3cb7c-8731-4897-9ef9-b0868148ebe2)

### Net Emulation
This window is used to configure the network emulation

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/97103f15-fae8-4fe9-8189-8fdbcab5cb20)

### Output Log
Display the output log based on each log categories verbosity.
    
![image](https://github.com/arnaud-jamin/Cog/assets/13844285/71b1de06-a3d0-4e4d-83f3-c3f482c0d8f4)
- The verbosity of each log category can be configured in the 'Log Categories' window.

### Pools
Displays attributes of the selected actor as pools.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/7bb1aadd-9c0b-439f-b263-5ed842d0cd69)
- The pools can be configured in a data asset.

### Plots
Plots values overtime. When applicable, only the values of the selected actor are displayed.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/bc134e95-4887-4245-b34d-c030464f644a)

### Scalability
Used to configure the rendering quality.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/4866b54c-5efa-4efa-a841-74ac8e1713c0)

### Skeleton
This window display the bone hierarchy and the skeleton debug draw of the selected actor if it has a Skeletal Mesh.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/19648e3d-70dc-45bc-940d-e53eb9a99871)
- Mouse over a bone to highlight it.
- Right click a bone to access more debug display.
- Use the [Ctrl] key to toggle the bone debug draw recursively.
        
### Selection
This window can be used to select an actor either by picking an actor in the world or by selecting an actor in the actor list.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/67cf9019-85c0-449f-88bf-0fc4841f8795)
- The actor list can be filtered by actor type (Actor, Character, etc).
- The current selection is used by various debug windows to filter out their content.

### Spawn
Used to spawn new actors in the world. The spawn list can be configured in a Data Asset.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/b37f9307-5fd5-4ea5-9652-c265a8f63e32)

### Stats
Displays engine stats such as FPS, Ping, Packet Loss.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/e394ec7b-02fa-4b09-879b-90c82bd542ef)

### Tags
Displays gameplay tags of the selected actor.

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/3f14be3f-77f2-4d59-887d-1245fc97ed6a)

### Time Scale
Used to change the game global time scale. 

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/d19198c5-37dd-400d-a09f-7a5077eb2511)
- If changed on a client the time scale is also modified on the game server. 

### Tweaks
Used to apply tweaks to all the spawned actors

![image](https://github.com/arnaud-jamin/Cog/assets/13844285/e8a5cdd5-908d-4ae3-901a-48addd4ce353)
- The tweaks are used to test various gameplay settings by actor category
- The tweaks can be configured in a data asset.

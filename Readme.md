# HoloTable

The concept of this game is to have a real time strategy simulation of using a holographic table to command space-faring vessels.

You don't control units directly, but the scripts controlling your units can be edited live!

Current status: drawing board, conceptual stage

Inspirations:
- EVE Online (combat inspirations, economic simulation)
- X games (primarily X3 and X4, concept of a gate network)
- Kenshi (exploration of ruins, sense of danger everywhere)
- Dwarf Fortress (simulation of personnel)
- FTL (damage model)
- Starsector (2d fleet engagement simulation)
- Majesty (indirect control model)

What makes it unique:
- logistics simulation: supply lines!
- simulation scale and performance
- live scripting and debugging (for those so inclined)
- Server Multiplayer model, similar to Avorion or Minecraft

## Compiler Support

GCC or Clang versions that support module std

MSVC doesn't build RGFW 1.7 for some reason, so it's excluded

## Build Dependencies

Windows build should be working.

Wayland isn't working at the moment, but X11 should be.

These dependencies should be sourced and placed under third_party before building

- Version 1.7 of RGFW.h: https://github.com/ColleagueRiley/RGFW

## Project Structure

- lua_scripts: "agent" logic is written in a scripting language
- shaders: shaders live here
- assets: audio-visual asset files live here
- game_lib: the game code itself will be compiled as a DLL
- graphical_client: here is where windowing, GUI, and rendering of the game is handled
- headless: here is where hosting a server from CLI is handled
- tests: here is where tests live
- third_party: here is where we put build dependencies from others, primarily windowing library, graphics API libraries, and math libraries

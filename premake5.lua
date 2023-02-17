-- premake5.lua
workspace "ImGuiDesigner"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "ImGuiDesigner"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "ImGuiDesigner"
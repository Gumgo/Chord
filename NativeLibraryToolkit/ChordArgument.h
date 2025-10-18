#pragma once

#define CHORD_ARG(type, name, ...) ChordArgumentType<#type " " #name __VA_OPT__(,) __VA_ARGS__> name
#define CHORD_IN(type, name, ...) ChordArgumentType<"in " #type " " #name __VA_OPT__(,) __VA_ARGS__> name
#define CHORD_OUT(type, name, ...) ChordArgumentType<"out " #type " " #name __VA_OPT__(,) __VA_ARGS__> name
#define CHORD_RETURN(type, name, ...) ChordArgumentType<"out " #type " " #name, ChordArgumentFlags::IsReturn __VA_OPT__(| (__VA_ARGS__))> name
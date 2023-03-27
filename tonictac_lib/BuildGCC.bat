@echo off
dlltool -d APMmxBVR.def -l APMmxBVR.a
dlltool -d MPGMXBVR.def -l MPGMXBVR.a
gcc -s -Os -Wall -ansi -pedantic tonictac_lib.c _snd_dll_link.c -o tonictac_lib.exe APMmxBVR.a MPGMXBVR.a -l kernel32 -l user32

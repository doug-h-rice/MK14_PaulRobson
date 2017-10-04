@echo off
rem
rem     The library is cpu.c and memory.c, these make the emulator
rem     Individual drivers for systems are in drv_xxx.c
rem
rem     Link these with main.c (debugger version) or minmain.c (emu only)
rem
rem     drv_dos.c provides a BGI graphic and a text console version, to
rem     select BGI define GFX. This requires EGAVGA.BGI and an EGA display.
rem
tcc -DMSDOS -ms -Z -G -emk14.exe cpu.c minmain.c memory.c drv_dos.c
tcc -DMSDOS -DGFX -ml -G -Z -emk14g.exe cpu.c minmain.c memory.c drv_dos.c c:\tc\lib\graphics.lib
rem
rem     Build the zip files.
rem
pkzip -ex    mk14_src.zip cpu.c main.c minmain.c memory.c drv_dos.c scmp.h make.bat run.txt
pkzip -ex -a mk14_src.zip resscmp.h mk14w.c wk14.rc wk14.ide
pkzip -ex    mk14_exe.zip mk14.exe mk14g.exe egavga.bgi wk14.exe run.txt
rem
copy mk14_src.zip e:\mk14\web\mk14
copy mk14_exe.zip e:\mk14\web\mk14


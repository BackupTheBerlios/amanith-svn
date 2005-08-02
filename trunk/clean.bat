nmake clean


del /S .\3rdpart\*.obj
del /S .\3rdpart\*.idb
del /S .\3rdpart\*.pdb
del /S .\3rdpart\*.ncb
del /S .\3rdpart\*.ilk
del /S .\3rdpart\*.sln
del /S .\3rdpart\*.vcproj
del /S /AH .\3rdpart\*.suo


del /S .\build\*.obj
del /S .\build\*.idb
del /S .\build\*.pdb
del /S .\build\*.ncb
del /S .\build\*.ilk
del /S .\build\*.sln
del /S .\build\*.vcproj
del /S /AH .\build\*.suo


del /S .\examples\*.obj
del /S .\examples\*.idb
del /S .\examples\*.pdb
del /S .\examples\*.ncb
del /S .\examples\*.ilk
del /S .\examples\*.sln
del /S .\examples\*.vcproj
del /S /AH .\examples\*.suo
del /S .\examples\*.exe
del /S .\examples\*.res


del /S .\plugins\*.obj
del /S .\plugins\*.idb
del /S .\plugins\*.pdb
del /S .\plugins\*.ncb
del /S .\plugins\*.ilk
del /S .\plugins\*.sln
del /S .\plugins\*.vcproj
del /S /AH .\plugins\*.suo
del .\plugins\*.lib
del .\plugins\*.a
del .\plugins\*.dll


del /S .\tools\*.obj
del /S .\tools\*.idb
del /S .\tools\*.pdb
del /S .\tools\*.ncb
del /S .\tools\*.ilk
del /S .\tools\*.sln
del /S .\tools\*.vcproj
del /S /AH .\tools\*.suo
del /S .\tools\*.exe
del /S .\examples\*.res


del /S .\vs2003\*.obj
del /S .\vs2003\*.idb
del /S .\vs2003\*.pdb
del /S .\vs2003\*.exp
del /S .\vs2003\*.lib
del /S .\vs2003\*.ncb
del /S .\vs2003\*.ilk
del /S .\vs2003\*.exe


del /S BuildLog.*
del /S *.ui.bak
del /S Makefile
del /Q .\lib\*.*

del /S *.jbf

del *.ncb
del /AH *.suo

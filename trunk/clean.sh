make clean

find ./ -name "Makefile" -delete
find ./ -name "Makefile.Release" -delete
find ./ -name "Makefile.Debug" -delete

find ./3rdpart/ -name "*.idb" -delete
find ./3rdpart/ -name "*.pdb" -delete
find ./3rdpart/ -name "*.ncb" -delete
find ./3rdpart/ -name "*.ilk" -delete
find ./3rdpart/ -name "*.sln" -delete
find ./3rdpart/ -name "*.vcproj" -delete
find ./3rdpart/ -name "*.suo" -delete

find ./build/ -name "*.idb" -delete
find ./build/ -name "*.pdb" -delete
find ./build/ -name "*.ncb" -delete
find ./build/ -name "*.ilk" -delete
find ./build/ -name "*.sln" -delete
find ./build/ -name "*.vcproj" -delete
find ./build/ -name "*.suo" -delete

find ./examples/ -name "*.idb" -delete
find ./examples/ -name "*.pdb" -delete
find ./examples/ -name "*.ncb" -delete
find ./examples/ -name "*.ilk" -delete
find ./examples/ -name "*.sln" -delete
find ./examples/ -name "*.vcproj" -delete
find ./examples/ -name "*.suo" -delete
find ./examples/ -name "*.exe" -delete
find ./examples/ -name "*.res" -delete

find ./plugins/ -name "*.idb" -delete
find ./plugins/ -name "*.pdb" -delete
find ./plugins/ -name "*.ncb" -delete
find ./plugins/ -name "*.ilk" -delete
find ./plugins/ -name "*.sln" -delete
find ./plugins/ -name "*.vcproj" -delete
find ./plugins/ -name "*.suo" -delete

rm ./plugins/*.lib
rm ./plugins/*.a
rm ./plugins/*.dll

find ./vs2003/ -name "*.idb" -delete
find ./vs2003/ -name "*.pdb" -delete
find ./vs2003/ -name "*.ncb" -delete
find ./vs2003/ -name "*.ilk" -delete
find ./vs2003/ -name "*.lib" -delete
find ./vs2003/ -name "*.exp" -delete
find ./vs2003/ -name "*.exe" -delete

find ./ -name "BuildLog.*" -delete
find ./ -name "*.ui.bak" -delete
find ./ -name "*manifest*" -delete
find ./lib/ -name "*.*" -delete
find ./ -name "*.jbf" -delete

rm ./*.ncb
rm ./*.suo

set i=%~n1
set j=%~n2

echo Testing %i% with the %j% configuration
copy ..\CK2_Saves\%i%.zip .\%i%.zip
"%SEVENZIP_LOC%\7z.exe" e -tzip "%i%.zip" "*.*" -mx5
del %i%.zip
rmdir %i%
call CK2ToEU3.exe %i%.ck2
del *.ck2 /q
copy *.eu3 ..\testresults\%j%\
del *.eu3 /q
copy log.txt ..\testresults\%j%\%i%-Log.txt
del log.txt /q
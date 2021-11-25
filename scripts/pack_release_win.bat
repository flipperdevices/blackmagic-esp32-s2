@echo off
cd build
setlocal EnableDelayedExpansion

FOR /F "tokens=* USEBACKQ" %%g IN (`git rev-parse --short HEAD`) do (SET "GIT_COMMIT=%%g")
SET "RELEASE_FILENAME=release_%GIT_COMMIT%"

echo | set /p="esptool.py -p (PORT) -b 460800 --before default_reset --after hard_reset --chip esp32s2  write_flash " > flash.command
for /f "tokens=*" %%a in (flash_args) do (
  echo | set /p=%%a >> flash.command
)

del release.zip
"C:\Program Files\7-Zip\7z.exe" a -tzip -mx5 -r0 %RELEASE_FILENAME%.zip blackmagic.bin
"C:\Program Files\7-Zip\7z.exe" a -tzip -mx5 -r0 %RELEASE_FILENAME%.zip bootloader/bootloader.bin
"C:\Program Files\7-Zip\7z.exe" a -tzip -mx5 -r0 %RELEASE_FILENAME%.zip partition_table/partition-table.bin
"C:\Program Files\7-Zip\7z.exe" a -tzip -mx5 -r0 %RELEASE_FILENAME%.zip flash.command
del flash.command

cd ..
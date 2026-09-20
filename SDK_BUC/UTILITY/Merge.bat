:begin
rem -----------------------------------------------------------------------------------------
rem                         Generate bin/hex for current target
rem -----------------------------------------------------------------------------------------

.\UTILITY\binConverter -re .\BIN\%1 %2 0 .\BIN\ISPMemDrive
.\UTILITY\binModify .\CONFIG\Header %5 %errorlevel%
.\UTILITY\binModify .\CONFIG\Header %4 %3h
.\UTILITY\binVerify ..\..\ ..\COMMON_SRC\CMSIS\ .\CONFIG\Header
.\UTILITY\bin2hex .\CONFIG\Header.bin .\CONFIG\Header.hex

del .\BIN\%1.bin
ren .\BIN\%1_Rep.bin %1.bin
del .\BIN\%1_Exec.hex
.\UTILITY\bin2hex_ext 0x%3 .\BIN\%1.bin .\BIN\%1_Exec.hex

rem Book Hook Hex
.\UTILITY\bin2hex_ext 0x2000 .\BIN\Boot_Hook.bin .\BIN\Boot_Hook.hex

rem sFWU.bin
IF EXIST .\BIN\sFWU.bin (
.\UTILITY\bin2hex_ext 0x1A000 .\BIN\sFWU.bin .\BIN\sFWU.hex
.\UTILITY\HexMerger .\BIN\Boot_Hook.hex .\BIN\sFWU.hex .\BIN\temp1.hex
del .\BIN\Boot_Hook.hex
copy .\BIN\temp1.hex .\BIN\Boot_Hook.hex
del .\BIN\temp1.hex
.\UTILITY\binConverter -gs .\BIN\sFWU 40988
)
SET SFWUSZ=%errorlevel%
SET /a SFWUSZ-=32

rem -----------------------------------------------------------------------------------------
rem                            Merge all hex to SN93700 hex
rem -----------------------------------------------------------------------------------------
.\UTILITY\HexMerger .\CONFIG\Header.hex .\BIN\Boot_Hook.hex .\BIN\temp1.hex
IF EXIST .\BIN\SN93700temp.hex (
.\UTILITY\HexMerger .\BIN\temp1.hex .\BIN\SN93700temp.hex .\BIN\temp2.hex
.\UTILITY\HexMerger .\BIN\temp2.hex .\BIN\%1_Exec.hex .\BIN\SN93700IMD.hex
del .\BIN\temp2.hex
del .\BIN\SN93700temp.hex
) ELSE (
.\UTILITY\HexMerger .\BIN\temp1.hex .\BIN\%1_Exec.hex .\BIN\SN93700IMD.hex
)
.\UTILITY\hex2bin .\BIN\SN93700IMD.hex
del .\BIN\temp1.hex

.\UTILITY\binConverter -gs .\BIN\SN93700IMD 192
.\UTILITY\binModify .\BIN\SN93700IMD 192 %errorlevel%

.\UTILITY\bin2hex_ext 0x0 .\BIN\ISPMemDrive.bin .\BIN\temp1.hex
.\UTILITY\bin2hex_ext 0xA000 .\BIN\SN93700IMD.bin .\BIN\temp2.hex
.\UTILITY\HexMerger .\BIN\temp1.hex .\BIN\temp2.hex .\BIN\FW_SN93700.hex
.\UTILITY\hex2bin .\BIN\FW_SN93700.hex
.\UTILITY\binConverter -gs .\BIN\FW_SN93700 41156
SET /a FILEB=%errorlevel%
SET /a FILEKB=%FILEB%/1024
SET /a PROFSF=(%FILEKB%+255)/256*256
IF %PROFSF% lss %6% (
SET PROFSF=%6%
)
.\UTILITY\binModify .\BIN\FW_SN93700 41156 %FILEB%
.\UTILITY\binModify .\BIN\FW_SN93700 41160 %PROFSF%
IF EXIST .\BIN\sFWU.bin (
.\UTILITY\binModify .\BIN\FW_SN93700 40988 %SFWUSZ%
.\UTILITY\binModify .\BIN\FW_SN93700 40984 1A000h
)
del .\BIN\%1.bin
copy .\BIN\FW_SN93700.bin .\BIN\%1.bin
copy .\BIN\FW_SN93700.hex .\BIN\%1.hex
del .\BIN\temp1.hex
del .\BIN\temp2.hex
IF EXIST .\BIN\%1.bin (
.\UTILITY\binConverter -als .\BIN\%1 32768
)
IF EXIST .\BIN\%1_Rep.bin (
del .\BIN\%1.bin
copy .\BIN\%1_Rep.bin .\BIN\%1.bin /Y
del .\BIN\%1_Rep.bin
)

rem -----------------------------------------------------------------------------------------
rem                            Merge IQ or OSD
rem -----------------------------------------------------------------------------------------
SET /a PROFADDR=PROFSF*1024, PROFADDR+=40960

if %1==510PF_BUC_CU_A7130 set result=true
if %1==510PF_BUC_CU_RTC676x set result=true
if %1==510PF_BUC_CU_S2019x set result=true
if "%result%" == "true" (
    goto MergeOSD
)

:MergeIQ
.\UTILITY\bin2hex_ext  %PROFADDR% .\SYSTEM\Profile\Profile.dat .\BIN\temp1.hex
.\UTILITY\HexMerger  .\BIN\FW_SN93700.hex .\BIN\temp1.hex .\BIN\temp2.hex
.\UTILITY\hex2bin  .\BIN\temp2.hex
IF EXIST .\BIN\FW_SN93700_IQ.bin (
del .\BIN\FW_SN93700_IQ.bin
)
IF EXIST .\BIN\FW_SN93700_OSD.bin (
del .\BIN\FW_SN93700_OSD.bin
)
del .\BIN\temp1.hex
rem IQ address
SET FILESZ=0
SET /A FILESZ=%PROFADDR%-40960+8192
.\UTILITY\binModify .\BIN\temp2 %PROFADDR% %FILESZ%
rem copy /b .\BIN\temp2.bin+.\SYSTEM\IQ\H62\H62.bin .\BIN\FW_SN93700_IQ.bin
copy /b .\BIN\temp2.bin+.\SYSTEM\IQ\H62\H62.bin .\BIN\temp1.bin
SET /a DSADDR=PROFADDR+1028
.\UTILITY\binConverter -gs .\BIN\temp1 %DSADDR%
SET FILESZ=0
SET /A FILESZ=%errorlevel%-40960
.\UTILITY\binModify .\BIN\temp1 %DSADDR% %FILESZ%
copy /b .\BIN\temp1.bin+.\SYSTEM\DS\DS_OsdImage.dat .\BIN\FW_SN93700_IQ.bin
.\UTILITY\binConverter -als .\BIN\FW_SN93700_IQ 32768
IF EXIST .\BIN\FW_SN93700_IQ_Rep.bin (
del .\BIN\FW_SN93700_IQ.bin
copy .\BIN\FW_SN93700_IQ_Rep.bin .\BIN\FW_SN93700_IQ.bin /Y
del .\BIN\FW_SN93700_IQ_Rep.bin
)
.\UTILITY\binConverter -gs .\BIN\FW_SN93700_IQ 41156
.\UTILITY\binModify .\BIN\FW_SN93700_IQ 41156 %errorlevel%
.\UTILITY\binModify .\BIN\FW_SN93700_IQ 41160 %PROFSF%
del .\BIN\temp2.hex
del .\BIN\temp2.bin
del .\BIN\temp1.bin
goto end_merge

:MergeOSD
.\UTILITY\bin2hex_ext %PROFADDR% .\SYSTEM\Profile\Profile.dat .\BIN\temp1.hex
.\UTILITY\HexMerger .\BIN\FW_SN93700.hex .\BIN\temp1.hex .\BIN\temp2.hex
.\UTILITY\hex2bin .\BIN\temp2.hex
IF EXIST .\BIN\FW_SN93700_OSD.bin (
del .\BIN\FW_SN93700_OSD.bin
)
IF EXIST .\BIN\FW_SN93700_IQ.bin (
del .\BIN\FW_SN93700_IQ.bin
)
del .\BIN\temp1.hex
rem OSD font address
SET /a OSDFNTADDR=PROFADDR+1032
SET FILESZ=0
SET /A FILESZ=%PROFADDR%-40960+8192
.\UTILITY\binModify .\BIN\temp2 %OSDFNTADDR% %FILESZ%
rem OSD image address
copy .\SYSTEM\OSD\OSD_Font_HD_90¢X.pat .\SYSTEM\OSD\temp1.bin
SET /a OSDIMGADDR=PROFADDR+1028
.\UTILITY\binConverter -gs .\SYSTEM\OSD\temp1 %OSDIMGADDR%
SET FILESZ=0
SET /A FILESZ=%PROFADDR%-40960+8192+%errorlevel%
.\UTILITY\binModify .\BIN\temp2 %OSDIMGADDR% %FILESZ%
del .\SYSTEM\OSD\temp1.bin
copy /b .\BIN\temp2.bin+.\SYSTEM\OSD\OSD_Font_HD_90¢X.pat+.\SYSTEM\OSD\OSD_Image_WSVGA_0¢X.dat .\BIN\temp1.bin
rem copy /b .\BIN\temp2.bin+.\SYSTEM\OSD\OSD_Font_HD_90¢X.pat+.\SYSTEM\OSD\OSD_Image_HD_90¢X.dat .\BIN\temp1.bin
SET /a LOGOADDR=PROFADDR+12
.\UTILITY\binConverter -gs .\BIN\temp1 %LOGOADDR%
SET FILESZ=0
SET /A FILESZ=%errorlevel%-40960
.\UTILITY\binModify .\BIN\temp1 %LOGOADDR% %FILESZ%
copy /b .\BIN\temp1.bin+.\SYSTEM\OSD\OSD_Logo_WSVGA_0¢X.dat .\BIN\FW_SN93700_OSD.bin
rem copy /b .\BIN\temp1.bin+.\SYSTEM\OSD\OSD_Logo_HD_90¢X.dat .\BIN\FW_SN93700_OSD.bin
.\UTILITY\binConverter -als .\BIN\FW_SN93700_OSD 32768
IF EXIST .\BIN\FW_SN93700_OSD_Rep.bin (
del .\BIN\FW_SN93700_OSD.bin
copy .\BIN\FW_SN93700_OSD_Rep.bin .\BIN\FW_SN93700_OSD.bin /Y
del .\BIN\FW_SN93700_OSD_Rep.bin
)
.\UTILITY\binConverter -gs .\BIN\FW_SN93700_OSD 41156
.\UTILITY\binModify .\BIN\FW_SN93700_OSD 41156 %errorlevel%
.\UTILITY\binModify .\BIN\FW_SN93700_OSD 41160 %PROFSF%
del .\BIN\temp1.bin
del .\BIN\temp2.hex
del .\BIN\temp2.bin

:end_merge
del .\BIN\FW_SN93700.bin
del .\BIN\FW_SN93700.hex

@echo Finish time: %date:~0,10% %time:~0,8%

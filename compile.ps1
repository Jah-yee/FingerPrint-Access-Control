$cli = "C:\Users\DELL\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
$lib = "C:\Users\DELL\OneDrive - Pan-Atlantic University\Arduino\libraries"
& $cli compile --fqbn STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE --libraries "$lib" "$PSScriptRoot"

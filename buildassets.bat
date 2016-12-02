REM #!/bin/bash

cd /d build\bin\debug
modelbuild -t %~dp0\assets\sphere.obj -m
modelbuild -t %~dp0\assets\cube.obj -m
modelbuild -t %~dp0\assets\sponza\bannder.obj -m
modelbuild -t %~dp0\assets\sponza\sponza.obj

pause
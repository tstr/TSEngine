REM #!/bin/bash

cd /d build\bin\debug
modelbuilder -t %~dp0\assets\sphere.obj -m
modelbuilder -t %~dp0\assets\cube.obj -m
modelbuilder -t %~dp0\assets\sponza\sponza.obj -m
modelbuilder -t %~dp0\assets\sponza\bannder.obj -m

pause
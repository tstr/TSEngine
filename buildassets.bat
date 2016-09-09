REM #!/bin/bash

cd /d build\bin\debug
modelbuilder -t %~dp0\assets\sphere.obj
modelbuilder -t %~dp0\assets\cube.obj -m

pause
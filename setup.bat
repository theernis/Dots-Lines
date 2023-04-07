setup.py install
xcopy /s /y %CD%\build\lib.win-amd64-cpython-311 %CD%
pause
python dots.py
pause
@echo off
cd /d %~dp0
echo Generating all FPSC Tools infographics...
python generate_all.py
echo Done!
pause

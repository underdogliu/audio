@echo off

set IS_CONDA=1
set BUILD_FFMPEG=1
python setup.py install --single-version-externally-managed --record=record.txt

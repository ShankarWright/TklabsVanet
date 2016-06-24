@echo off

set sw=%1
set hw=%2

if [%sw%]==[] set sw=pdg
if [%hw%]==[] set hw=VanetDaughterboardRevB

set file="%~dp0%sw%-%hw%.hex"

if not exist %file% (
    echo Invalid configuration
    echo %file%
    goto :eof
)

echo Erasing Chip
atprogram -t jtagice3 -i jtag -d at32uc3c1512c chiperase

echo Programming %sw%-%hw%
atprogram -t jtagice3 -i jtag -d at32uc3c1512c program -e --verify -f %file%

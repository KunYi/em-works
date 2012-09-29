@echo off
rem Remove previous cec file

for %%p in (eetiusb,eetitool,eeti232) do (
    pbcec -r %%p.cec
)

@echo on

if %1==r GOTO leave

:addcec

rem Add the new current elo components
for %%p in (eetiusb,eetitool,eeti232) do (
    pbcec c:\eeticedb\wince500\%%p.cec
)

:leave

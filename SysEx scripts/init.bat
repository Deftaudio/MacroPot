REM program all macros to default name "macro# " from 0 to 9
call macrodefaultnames.bat

REM macro 0 button programming, note push mode
REM sendmidi dev "MacroPot" hex syx 7d [macro# ] 00 [USBport# ] [MIDIport# ] [type] [channel] [number] [value] [push]
sendmidi.exe dev "MacroPot" hex syx 7d 00 00 01 01 02 01 30 7f 01

REM macro 0 program controller 1 from 0 to 127 all range to port 1, others to erase
REM sendmidi dev "MacroPot" hex syx 7d [macro#] [controller# 1-5] [USBport#] [MIDIport#] [type] [channel] [number] [lowvalue] [highvalue] [mapstart] [mapend] [curve] [velocity]
sendmidi.exe dev "MacroPot" hex syx 7d 00 01 01 01 01 01 30 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 00 02 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 00 03 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 00 04 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 00 05 01 01 00 01 00 00 7f 00 7f 40 7f

REM macro 1 button programming, note toggle mode
REM sendmidi dev "MacroPot" hex syx 7d [macro#] 00 [USBport#] [MIDIport#] [type] [channel] [number] [value] [push]
sendmidi.exe dev "MacroPot" hex syx 7d 01 00 01 01 02 01 30 7f 00

REM macro 1 program controller 1-4 from 0 to 127 all range to ALL ports, others to erase
REM sendmidi dev "MacroPot" hex syx 7d [macro#] [controller# 1-5] [USBport#] [MIDIport#] [type] [channel] [number] [lowvalue] [highvalue] [mapstart] [mapend] [curve] [velocity]
sendmidi.exe dev "MacroPot" hex syx 7d 01 01 01 01 01 01 30 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 01 02 02 02 01 01 31 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 01 03 03 03 01 01 32 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 01 04 04 04 01 01 33 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 01 05 01 01 00 01 00 00 7f 00 7f 40 7f

REM macro 2 button programming, CC push mode
REM sendmidi dev "MacroPot" hex syx 7d [macro#] 00 [USBport#] [MIDIport#] [type] [channel] [number] [value] [push]
sendmidi.exe dev "MacroPot" hex syx 7d 02 00 01 01 01 01 30 7f 01

REM macro 2 program controller 1 and 2 from 0 to 127 different ranges to 1 port, others to erase
REM sendmidi dev "MacroPot" hex syx 7d [macro#] [controller# 1-5] [USBport#] [MIDIport#] [type] [channel] [number] [lowvalue] [highvalue] [mapstart] [mapend] [curve] [velocity]
sendmidi.exe dev "MacroPot" hex syx 7d 02 01 01 01 01 01 30 00 7f 00 40 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 02 02 01 01 01 01 31 00 7f 41 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 02 03 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 02 04 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 02 05 01 01 00 01 00 00 7f 00 7f 40 7f

REM macro 3 button programming, CC toggle mode
REM sendmidi dev "MacroPot" hex syx 7d [macro#] 00 [USBport#] [MIDIport#] [type] [channel] [number] [value] [push]
sendmidi.exe dev "MacroPot" hex syx 7d 03 00 01 01 01 01 30 7f 00

REM macro 3 program controller 1 and 2 from 0 to 127 on 1 and 2 ports, while Note message is sent to port 3 and the end of the range
REM sendmidi dev "MacroPot" hex syx 7d [macro#] [controller# 1-5] [USBport#] [MIDIport#] [type] [channel] [number] [lowvalue] [highvalue] [mapstart] [mapend] [curve] [velocity]
sendmidi.exe dev "MacroPot" hex syx 7d 03 01 01 01 01 01 30 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 03 02 02 02 01 01 31 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 03 03 03 03 02 01 00 00 7f 7e 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 03 04 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 03 05 01 01 00 01 00 00 7f 00 7f 40 7f

REM macro 4 button programming, note push mode
REM sendmidi dev "MacroPot" hex syx 7d [macro#] 00 [USBport#] [MIDIport#] [type] [channel] [number] [value] [push]
sendmidi.exe dev "MacroPot" hex syx 7d 04 00 01 01 02 01 30 7f 01

REM macro 4 program controller 1 and 2 from 0 to 127 on 1 and 2 ports, adds a Curve on controller 2, while Program Change # 64 message is sent to port 3 and the end of the range
REM sendmidi dev "MacroPot" hex syx 7d [macro#] [controller# 1-5] [USBport#] [MIDIport#] [type] [channel] [number] [lowvalue] [highvalue] [mapstart] [mapend] [curve] [velocity]
sendmidi.exe dev "MacroPot" hex syx 7d 04 01 01 01 01 01 30 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 04 02 02 02 01 01 31 00 7f 00 7f 7f 7f
sendmidi.exe dev "MacroPot" hex syx 7d 04 03 03 03 03 01 40 00 7f 70 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 04 04 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 04 05 01 01 00 01 00 00 7f 00 7f 40 7f

REM macro 5 button programming, note push mode
REM sendmidi dev "MacroPot" hex syx 7d [macro#] 00 [USBport#] [MIDIport#] [type] [channel] [number] [value] [push]
sendmidi.exe dev "MacroPot" hex syx 7d 05 00 01 01 02 01 30 7f 01

REM macro 5 program controller 1 from 0 to 127 all range to port 1 and Merging Midi Input, others to erase
REM sendmidi dev "MacroPot" hex syx 7d [macro#] [controller# 1-5] [USBport#] [MIDIport#] [type] [channel] [number] [lowvalue] [highvalue] [mapstart] [mapend] [curve] [velocity]
sendmidi.exe dev "MacroPot" hex syx 7d 05 01 01 01 01 01 30 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 05 02 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 05 03 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 05 04 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 05 05 01 01 00 01 00 00 7f 00 7f 40 7f
REM enable merging to port 1 and change a name to "Merge"
sendmidi.exe dev "MacroPot" hex syx 7d 05 64 6d 65 72 67 65 20 20 01

REM macro 6 button programming, note push mode
REM sendmidi dev "MacroPot" hex syx 7d [macro#] 00 [USBport#] [MIDIport#] [type] [channel] [number] [value] [push]
sendmidi.exe dev "MacroPot" hex syx 7d 06 00 01 01 02 01 30 7f 01

REM macro 6 program controller 1 from 0 to 127 all range to port 1 and Merging Midi Input, others to erase
REM sendmidi dev "MacroPot" hex syx 7d [macro#] [controller# 1-5] [USBport#] [MIDIport#] [type] [channel] [number] [lowvalue] [highvalue] [mapstart] [mapend] [curve] [velocity]
sendmidi.exe dev "MacroPot" hex syx 7d 06 01 01 01 01 01 30 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 06 02 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 06 03 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 06 04 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 06 05 01 01 00 01 00 00 7f 00 7f 40 7f
REM enable merging to all ports and change a name to "MergALL"
sendmidi.exe dev "MacroPot" hex syx 7d 06 64 4d 65 72 67 41 6c 6c 07

REM macro 7 empty
sendmidi.exe dev "MacroPot" hex syx 7d 07 64 65 6d 70 74 79 31 20 00

REM macro 8 empty
sendmidi.exe dev "MacroPot" hex syx 7d 08 64 65 6d 70 74 79 32 20 00

REM macro 9 Synclavier mode can be the only controller in such macro setup
sendmidi.exe dev "MacroPot" hex syx 7d 09 01 01 01 05 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 09 02 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 09 03 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 09 04 01 01 00 01 00 00 7f 00 7f 40 7f
sendmidi.exe dev "MacroPot" hex syx 7d 09 05 01 01 00 01 00 00 7f 00 7f 40 7f



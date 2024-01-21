<p align="center">
    <img src="https://raw.githubusercontent.com/iliasizmaylov/six502/master/demo.gif" />
</p>

# six502 - The 6502 processor emulator

This project is supposed to be a library which will enable users to build their own 6502 CPU based
emulation systems in C++. The idea is that what if we have an already emulated CPU and bus and an
interface to create and connect our own devices to this bus and only implement their behavior when
databus is poking into their address range with either read or write request.

Anyway, this project is a part of me making my own NES emulator which is a system that is based on a
6502 processor so I thought why not first make a generic 6502 system emulator and then build my NES
emulator on it as a sort of an exercise.

With that being said - I'm not a 6502 enthusiast or a retro gamer (I never had nintendo when I was a kid
and not even old enough to be born in that era) so I don't think it's worth it to try and hone this
library so that it's actually usable and properly made. Well I mean it works now certainly but it definitely
is not anywhere near user friendly.

Regardles, this project is considered to still be work in progress and I think it'll be best for me to
not overspend my time on it until I finish the whole NES emulator.

Wooptie-doo.

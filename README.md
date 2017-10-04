# MK14_PaulRobson
Copy of Paul Robson's MK14 for DOS from 1998

To build this code I have used DOSbox

The code comes from 1998, so you need Borland Turbo C 3.0 which is run within DOS box

The compiled code runs within DOSbox

I have not built the Windows version.

This includes the revised monitor ROM


There is a bug with jumps when the displacement is 0x80. It uses the E register instead. It should not.

This bug prevents the original ---- -- monitor ROM from working.

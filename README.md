# Sequence Unique Patterns Counter

A simple application that counts each pattern in a file. This is an experimental application.

## Usage

This application can be used to detect the frequency of patterns in genomic sequences, for example.

The output for patterns of length 3 is something similar to the following.

```
TCT	563
AAA	1727
CGA	827
CAG	1484
CCA	1561
ATT	1298
CTT	824
CCG	1950
TCC	555
GCA	1570
TCG	1035
GCC	757
GTG	1610
GAG	534
AAG	1267
```

The first column contains the pattern, not including the prefix, and the second column includes the frequency of that pattern in the file.

## Compiling

You need a compiler capable of compiling C source-code written in at least the C89 standard.

## License

Copyright (c) 2019 Felipe Ferreira da Silva

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

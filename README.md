X-Fi Titanium Streaming Source Fix
==================================

!!! WARNING !!! WORK-IN-PROGRESS PROJECT
----------------------------------------

Contents
========

1. Overview
2. Minimum requirements
3. Log file
4. Compiling requirements
5. Disclaimer


1 - Overview
============

The project fixes the stuck streaming sources for X-Fi Titanium sound card.


2 - Minimum requirements
========================

Operating system:
- Windows XP with Service Pack 3


3 - Log file
============

The wrapper creates a log file (`xfitssfix_log.txt`) in the working directory, set by the application.
Usually it's identical to the directory where the wrapper is placed.

If the working directory or the log file itself is write-protected the logger tries to create the log in the default application data directory.

On Windows it's `%APPDATA%\bibendovsky\xfitssfix` (e.g., `C:\users\john\AppData\Roaming\bibendovsky\xfitssfix`).

There are different levels of the logging. Set environment variable `XFITSSFIX_LOG_LEVEL` to the one of the following values:
- 0 - disable the logging (default).
- 1 - log errors.
- 2 - log errors and warnings.
- 3 - log everything.


4 - Compiling requirements
==========================

Common:
- Windows 32/64-bit build target.

Minimum:
- CMake v3.15.7
- C++17 compatible compiler.


5 - Disclaimer
===============

Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.

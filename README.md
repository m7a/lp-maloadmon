---
section: 11
x-masysma-name: maloadmon
title: Ma_Sys.ma Load Monitor
date: 2020/02/14 14:55:57
lang: en-US
author: ["Linux-Fan, Ma_Sys.ma (Ma_Sys.ma@web.de)"]
keywords: ["monitoring", "loadmon", "c"]
x-masysma-version: 1.0.0
x-masysma-repository: https://www.github.com/m7a/lp-maloadmon
x-masysma-website: https://masysma.net/11/maloadmon.xhtml
x-masysma-owned: 1
x-masysma-copyright: |
  Copyright (c) 2013, 2020 Ma_Sys.ma.
  For further info send an e-mail to Ma_Sys.ma@web.de.
---
Name
====

`maloadmon` -- Display load average figures and uptimes continuously.

Synopsis
========

	maloadmon [delay_sec]

Description
===========

The Ma_Sys.ma load monitor uses terminal colors to indicate different load
averages over time. By default, it prints one triplet every minute. Once the
screen is full, a line indicating the current uptime is printed.

It is intended to be used in case of long-running computations to check that
if the machine is evenly loaded or as expected. The non-interactive and
lightweight design makes this program also a good choice for a monitor to run
on one of the virtual terminals.

Note that while it captures certain workloads, load averages are not suited
as a comprehensive monitoring facility.

![`maloadmon` Screenshot with `delay_sec=5` on a lowly loaded Laptop](maloadmon_att/scr.png)

`delay_sec`
:   Configures the delay between updates (default: 60sec).

Order of colors is as follows (t is for “thin”):

gray < tblue < blue < tcyan < cyan < tyellow < yellow < tmangentha < mangentha
< tred < red

Compilation
===========

Compile the program as follows:

	$ ant

Build a Debian package (having the necessary dependencies for package creation
installed) as follows:

	$ ant package

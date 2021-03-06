/*	$NetBSD: macropath.cpp,v 1.1.1.1 2016/01/13 18:41:48 christos Exp $	*/

// -*- C++ -*-
/* Copyright (C) 1989, 1990, 1991, 1992, 2000 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com)

This file is part of groff.

groff is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

groff is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with groff; see the file COPYING.  If not, write to the Free Software
Foundation, 51 Franklin St - Fifth Floor, Boston, MA 02110-1301, USA. */

#include "lib.h"
#include "searchpath.h"
#include "macropath.h"
#include "defs.h"

#define MACROPATH_ENVVAR "GROFF_TMAC_PATH"

search_path macro_path(MACROPATH_ENVVAR, MACROPATH, 1, 1);
search_path safer_macro_path(MACROPATH_ENVVAR, MACROPATH, 1, 0);
search_path config_macro_path(MACROPATH_ENVVAR, MACROPATH, 0, 0);

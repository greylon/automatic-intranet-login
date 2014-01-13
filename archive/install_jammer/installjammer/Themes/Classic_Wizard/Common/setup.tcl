## $Id$
##
## BEGIN LICENSE BLOCK
##
## Copyright (C) 2002  Damon Courtney
## 
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## version 2 as published by the Free Software Foundation.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License version 2 for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the
##     Free Software Foundation, Inc.
##     51 Franklin Street, Fifth Floor
##     Boston, MA  02110-1301, USA.
##
## END LICENSE BLOCK

proc ThemeSetup {} {
    if {$::tcl_platform(platform) ne "windows"} {
        option add *font                TkTextFont
	option add *background          [style default . -background]
	option add *selectForeground    [style default . -selectforeground]
	option add *selectBackground    [style default . -selectbackground]

	option add *Listbox.background  #FFFFFF

	option add *Entry.background    #FFFFFF
	option add *Entry.borderWidth   1

	option add *Text.background     #FFFFFF
    }
}

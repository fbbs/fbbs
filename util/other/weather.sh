#! /bin/bash
#
# Auto Weather Predictor
# 
# Author: jamguo@sh163.net
# 
# Copyright (C) 2003 Wicretrend Workgroup. 
# 
# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License as published by 
# the Free Software Foundation; either version 2 of the License, or 
# (at your option) any later version. 
# 
# This program is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU General Public License for more details. 
# 
# You should have received a copy of the GNU General Public License 
# along with this program; if not, write to the Free Software 
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
#

LOCKFILE='weather.lock'
LOGFILE='weather.log'

if (test -e $LOCKFILE)
        then
                echo "$LOCKFILE exists.  Maybe there is another daemon running. Please";
                echo "check it.  If there is no other daemon running, please remove $LOCKFILE.";
        else
        touch $LOCKFILE

	/usr/bin/wget -O China24.temp.html "http://weather.sina.com.cn/index.html"
	./weather_china24.pl < China24.temp.html > weather_china24.txt
	rm -rf China24.temp.html

	/usr/bin/wget -O China48.temp.html "http://weather.sina.com.cn/gn48w.html"
	./weather_china48.pl < China48.temp.html > weather_china48.txt
	rm -rf China48.temp.html

	/usr/bin/wget -O World.temp.html "http://weather.sina.com.cn/gw24w.html"
	./weather_world.pl < World.temp.html > weather_world.txt
	rm -rf World.temp.html

        touch $LOGFILE

        rm $LOCKFILE
fi

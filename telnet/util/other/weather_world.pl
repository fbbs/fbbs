#! /usr/bin/perl
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

$TEMPLATE_HEADER = <<_X_;
                            ¹ú¼Ê³ÇÊÐ24Ð¡Ê±ÌìÆøÔ¤±¨
                          <<<<<<<<TimeUpdate>>>>>>>
                            http://www.ycul.com
©³©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©·
©§   ³ÇÊÐ   <<<<<<<<<<<<<<Time1>>>>>>>>>>>> <<<<<<<<<<<<<<<Time2>>>>>>>>>>> ©§
©§              ÌìÆø       ·çÏò·çÁ¦   ×îµÍ      ÌìÆø       ·çÏò·çÁ¦   ×î¸ß  ©§
©Ä©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ì
_X_

$TEMPLATE_ROW = <<_X_;
©§<<City>>> <<Weather1>> <<<Wind1>>>> <Tp1> <<Weather2>> <<<Wind2>>>> <Tp2> ©§
_X_

$TEMPLATE_FOOTER = <<_X_;
©»©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¿
_X_

$TEMPLATE_SHANGHAI_PREFIX = "[1;34;47m";

$TEMPLATE_SHANGHAI_SUFFIX = "[m";

$started = 0;

$content = "";
while (<STDIN>) {

	# check for beginning
	if ($started == 0) {

		# rip the updating time
		if (/(\d+Äê\d+ÔÂ\d+ÈÕ\d+):?(\d+)¸üÐÂ/) {
			$started = 1;
			$var{'TimeUpdate'} = $1.'Ê±';

			# read until next table
			while (<STDIN>) {
				if (/<table/) {
					last;
				}
			}
		}
	} else {
		# check for ending
		if (/<\/table/) {
			$started = 0;
			last;
		}

		s/\n//g;
		s/\r//g;
		s/&nbsp;//g;

		$content .= $_;
	}
}

$content =~ s/> +/>/ig;
$content =~ s/ +</</ig;
$content =~ s/<\/tr>/<\/tr>\n/ig;
$content =~ s/ *align=center//ig;

@content = split /\n/, $content;

# rip $var{'Time1'} and $var{'Time2'} from line 0
$content[0] =~ ?<tr[^>]*><td[^>]*>[^<]*</td><td[^>]*>[^<]*</td><td[^>]*>([^<]*)</td><td[^>]*>([^<]*)</td></tr>?i;
$var{'Time1'} = $1;
$var{'Time2'} = $2;

&output($TEMPLATE_HEADER);

# skip line 1
for ($i = 2; $i <= $#content; $i++) {

	$content[$i] =~ /<td>([^<]*)<\/td><td>([^<]*)<\/td><td>([^<]*)<\/td><td>([^<]*)<\/td><td>([^<]*)<\/td><td>([^<]*)<\/td><td>([^<]*)<\/td><\/tr>$/i;

	$var{'City'} = $1;
	$var{'Weather1'} = $2;
	$var{'Wind1'} = $3;
	$var{'Tp1'} = $4;
	$var{'Weather2'} = $5;
	$var{'Wind2'} = $6;
	$var{'Tp2'} = $7;

	if ($var{'City'} eq "ÉÏº£") {
		$var{'City'} = $TEMPLATE_SHANGHAI_PREFIX.$var{'City'};
		$var{'Tp2'} .= $TEMPLATE_SHANGHAI_SUFFIX;
	}
	
	&output($TEMPLATE_ROW);

}

&output($TEMPLATE_FOOTER);
print <<_X_;
                    Powered by Wicretrend Workgroup & Ycul!
                 http://www.wicretrend.com http://www.ycul.com
_X_


sub output {

	$s = $_[0];

	while ($s =~ /(<+(\w+)>+)/m) {
		$vFormat = $1;
		$vName = $2;
		while (length($var{$vName}) < length($vFormat) - 1) {
			$var{$vName} = " ".$var{$vName}." ";
		}
		while (length($var{$vName}) < length($vFormat)) {
			$var{$vName} .= " ";
		}
		$s =~ s/$vFormat/$var{$vName}/;
	}

	print $s;
}

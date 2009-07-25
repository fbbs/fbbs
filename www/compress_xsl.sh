#!/bin/sh

for xsl in $1/*.xsl
do
	sed 's/^[ \t]*//g' $xsl | tr -d '\n' > $xsl
done

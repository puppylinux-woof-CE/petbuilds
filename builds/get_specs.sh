#!/bin/sh

# get'pet.specs'
[ -f 0pets_out.specs ] && rm 0pets_out.specs
cd 0pets_out
for pet in *.pet; do 
	echo -n "$pet "
	specs=`tar -xvJf "$pet" --no-anchored 'pet.specs' 2>/dev/null`
	cat $specs >> ../0pets_out.specs
	rm -r ${pet%.*}
done
cd -

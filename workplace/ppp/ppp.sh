#!/bin/bash


ln -f -s /Users/apple/Documents/develop/CynoNav/PPPBox/tables/OCEAN-GOT00.dat OCEAN-GOT00.dat
ln -f -s /Users/apple/Documents/develop/CynoNav/PPPBox/tables/msc.txt msc.txt
ln -f -s /Users/apple/Documents/develop/CynoNav/PPPBox/tables/URL_LIST.txt url.list
ln -f -s /Users/apple/Documents/develop/CynoNav/PPPBox/tables/igs08.atx igs08.atx
ln -f -s /Users/apple/Documents/develop/CynoNav/PPPBox/tables/receiver_bernese.lis recType.list

proj="igs16577"

# firstly, download the rnx files from IGS or IGS analysis center
get_rnx.sh -b "2011 10 9 0 0 0" -e "2011 10 15 0 0 0" -i 24 -a "IGS" -u "url.list" -s "$proj.stalist.test" -l "$proj" -p "/Users/apple/Documents/data/IGS/data" > get_rnx.log 

# then, download the ephemeris files from IGS or IGS analysis center
get_eph.sh -b "2011 10 9 0 0 0" -e "2011 10 15 0 0 0" -i 24 -a "IGS" -u "url.list" -t "type.list" -l "$proj" -p "/Users/apple/Documents/data/IGS/data" > get_eph.log 

# convert ssc2msc
cat $proj.ssclist | while read line
do
   ssc2msc -s $line
done

rm msc.txt
# now, let's merge all the msc files together for pppar positioning
ls *.msc | while read line
do
   cat $line >> msc.txt
done

rm $proj.outlist
cat $proj.rnxlist | while read line
do
   echo "$line".out >> $proj.outlist
done

# now, Let's perform the ppp positioning
ppp -r $proj.rnxlist -s $proj.ephlist -k $proj.clklist -e $proj.erplist -m msc.txt -o $proj.outlist



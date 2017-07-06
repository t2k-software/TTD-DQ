#!/bin/bash

#CURDIR=$(dirname $PWD)
source /home/t2k/mlawe/t2k/nd280devel/soffTasks/v1r46/cmt/setup.sh
#cd $CURDIR

read -p "Enter your name: " author
echo
# Period must be Sunday to Saturday with format MMDD-MMDD, where MM = month and DD = day. e.g. 0610-0616
read -p "Enter week period (MMDD-MMDD): " period 
read -p "Enter year (YYYY): " year
echo
read -p "Enter first Run of the week: " first
read -p "Enter last Run of the week: " last
echo

for det in ecal p0d smrd; do
    
    echo "Running Beam Timing DQ for the ${det}"
    
#    $CURDIR/scripts/RunTTDBeamTimingDQ.sh <<EOF
    ./RunTTDBeamTimingDQ.sh <<EOF
${det}
${period}
${year}
EOF
    
    echo "Running Gain and Pedestal DQ for the ${det}"
    
    read -p "Do you want to compare gain and peds to a specified reference file for the ecal? (y/n) " useref
    
    case $useref in
	[Yy]* ) read -p "Enter a reference Run: " refrun;;
	[Nn]* ) refrun="";;
	* ) echo "Answer not valid, no reference will be used"; refrun="";;
    esac
    
#    $CURDIR/scripts/RunTTDGainPedDQ.sh <<EOF
    ./RunTTDGainPedDQ.sh <<EOF
${det}
${period}
${year}
${first}
${last}
${useref}
${refrun}
EOF

done

echo "Making Slides"

#$CURDIR/scripts/MakeSlides.sh <<EOF
./MakeSlides.sh <<EOF
${period}
${year}
${author}
EOF

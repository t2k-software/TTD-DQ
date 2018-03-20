#!/bin/bash

### AUTHOR: MATTHEW LAWE

f=$(readlink -f $0)
. ${f%/*}/dateutils.sh

author=""
week=-1
USAGE="Usage: ${0##*/} [OPTION...]
Options:
  -w, --week=WEEK     Week relative to current week for which the plots
                      and presentation are generated. The default value 
                      is ${week} (Ie show data from last week).
  -a, --author=NAME    Name of the operator.
  -?, --help          Show this help list."


while [ "$1" != "" ] ; do
    case "$1" in
        -w|--week)
            week="$2"
            shift;;
        -a|--author)
            author="$2"
            shift;;
        "-?"|--help)
            echo "$USAGE"
            exit 0
            ;;
        *)
            echo "$USAGE"
            exit 1
            ;;
    esac
    shift
done

if [ "$author" = "" ] ; then
    read -p "Enter your name: " author
    echo
fi


# Download the subrun database to ~/.data/
dbfile="nd280_subrun_info.sqlite"
[ ! -d ~/.data ] && mkdir ~/.data
curl -o ~/.data/$dbfile http://repo.nd280.org/nd280files/nd280DQ/$dbfile

# Get the first and last runs of the week from the subrun database

for det in ecal p0d smrd; do
    
    echo "Running Beam Timing DQ for the ${det}"
#    $SOFFTASKSROOT/share/tript/RunTTDBeamTimingDQ.sh -w $week $det
    ${f%/*}/RunTTDBeamTimingDQ.sh -w $week $det
    
    echo "Running Gain and Pedestal DQ for the ${det}"
    
    read -p "Do you want to compare gain and peds to a specified reference file for the ${det}? (y/n) " useref
    
    case $useref in
	[Yy]* ) read -p "Enter a reference Run: " refrun
		refrun="-r $refrun"
		;;
	[Nn]* ) refrun="";;
	* ) echo "Answer not valid, no reference will be used"; refrun="";;
    esac
#    $SOFFTASKSROOT/share/tript/RunTTDGainPedDQ.sh -w $week -l $(lastRunOfWeek $week) -f $(firstRunOfWeek $week) $refrun $det
    ${f%/*}/RunTTDGainPedDQ.sh -w $week -l $(lastRunOfWeek $week) -f $(firstRunOfWeek $week) $refrun $det
    
done

echo "Making Slides"
${f%/*}/MakeSlides.sh -w $week -a "${author}"
#$SOFFTASKSROOT/share/tript/MakeSlides.sh -w $week -a "${author}"

#!/usr/bin/env bash

####################################################################
# After processing, copy the beam timing, gain, and pedestal drifts
# into tarballs to be given to the corresponding DQ experts
#
# AUTHOR: Matthew Hogan
# EMAIL: hoganman@rams.colostate.edu
####################################################################

f=$(readlink -f $0)
. ${f%/*}/dateutils.sh

CURDIR=$(dirname ${f%/*})

week="-1"
USAGE="Usage: ${0##*/} [OPTION...]
Create presentation file TTD data quality assesment.

Options:
  -w, --week=WEEK     Week relative to current week for which the plots
                      and presentation are generated. The default value 
                      is ${week} (Ie show data from last week).
  -?, --help          Show this help list."


while [ "$1" != "" ] ; do
    case "$1" in
	-w|--week)
	    week="$2"
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

date_dir=$(dateDir $week)

OLDDIR=$CURDIR
OUTDIR=$CURDIR/RunPeriods/$date_dir
cd $OUTDIR
# create tarballs for each detector
for det in ECAL P0D SMRD; do

    find . -type f -iname "*${det}*root" -print0 | tar -cvf ${det}DQ.tar --null -T -

done
cd $OLDIDR

echo "DONE making tarballs for DQ experts."
echo "Submit the following: "
for det in ECAL P0D SMRD; do
    echo "$CURDIR/RunPeriods/$date_dir/${det}DQ.tar to the $det DQ expert"
done
echo "To the DQ experts!"

#!/usr/bin/env bash
TAR=$(which tar)
FIND=$(which find)

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

for det in ECAL P0D SMRD; do
    OUTDIR=$CURDIR/RunPeriods/$date_dir
    $FIND $OUTDIR -type f "*${det}*root" -print0 | tar cvf $OUTDIR/${det}DQ.tar --null -T -
done
echo ${date_dir}

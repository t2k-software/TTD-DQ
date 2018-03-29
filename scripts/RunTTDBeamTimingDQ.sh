#!/bin/bash

### AUTHOR: MATTHEW LAWE

f=$(readlink -f $0)
. ${f%/*}/dateutils.sh

CURDIR=$(dirname ${f%/*})

week=-1
USAGE="Usage: ${0##*/} [OPTION...] <ecal|p0d|smrd>
Where <DET> is on of ecal, p0d or smrd

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
        ecal|smrd|p0d)
            det=$1
            ;;
	* )
	    echo $USAGE
	    exit 1
	    ;;
    esac
    shift
done

# Date of the data
date_data_dir=$(dateDir $week)

# Make a working directory
mkdir -p $CURDIR/RunPeriods/$date_data_dir/BeamTiming/$det $CURDIR/RunPeriods/$date_data_dir/BeamTiming/Files

cd $CURDIR/RunPeriods/$date_data_dir/BeamTiming/Files

echo "Retrieving Beam Timing summary files for ${det}"
echo
if [ -d "dq-${det}-b-v00" ]; then
    echo "Directory dq-${det}-b-v00 already exists, you must have downloaded these files already."
    echo "If you wish to download the files again, completely remove the directory dq-${det}-b-v00 from $PWD."
else
    echo "Working on directory /KEK-T2K/home/dataquality/data/summaryFiles/tript/${date_data_dir}/dq-${det}-b-v00"
    iget -rV /KEK-T2K/home/dataquality/data/summaryFiles/tript/${date_data_dir}/dq-${det}-b-v00 .
fi

echo

# Remove any empty files, there shouldn't be any though.
echo "Searching for and removing any empty files, any located will be listed below:"
find . -size 0 -type f -delete -print
echo

cd $CURDIR/RunPeriods/$date_data_dir/BeamTiming

# Make file lists
ls -1 $PWD/Files/dq-${det}-b-v00/${det}-b_*.root > $CURDIR/RunPeriods/$date_data_dir/BeamTiming/$det/files.list

  #-------------------------------------------------------------------------------------#
  # Usage: BeamTimingData.exe -f <file list>                                            #
  # <file list>: path to the list of root files                                         #
  #-------------------------------------------------------------------------------------#
 
# Beam Timing
cd $CURDIR/RunPeriods/$date_data_dir/BeamTiming/$det
echo "Processing beam timing files"
$CURDIR/BeamTiming.exe -f files.list > beamtiming.out

echo "DONE."

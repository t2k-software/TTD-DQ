#!/bin/bash

CURDIR=$(dirname $PWD)
source /home/t2k/mlawe/t2k/nd280devel/soffTasks/v1r46/cmt/setup.sh
cd $CURDIR

# Period must be Sunday to Saturday with format MMDD-MMDD, where MM = month and DD = day. e.g. 0610-0616
read -p "Choose a detector (ecal, p0d, smrd): " det
echo
read -p "Enter week period (MMDD-MMDD): " period 
read -p "Enter year (YYYY): " year
echo

# Check for valid detector
if [ $det = "ecal" -o $det = "p0d" -o $det = "smrd" ]; then 
    :
else
    echo "Invalid detector name, exiting"; exit
fi

# Make a working directory
mkdir -p $CURDIR/RunPeriods/$year/$period/BeamTiming/$det $CURDIR/RunPeriods/$year/$period/BeamTiming/Files

cd $CURDIR/RunPeriods/$year/$period/BeamTiming/Files

echo "Retrieving Beam Timing summary files for ${det}"
echo
if [ -d "dq-${det}-b-v00" ]; then
    echo "Directory dq-${det}-b-v00 already exists, you must have downloaded these files already."
    echo "If you wish to download the files again, completely remove the directory dq-${det}-b-v00 from $PWD."
else
    echo "Working on directory /KEK-T2K/home/dataquality/data/summaryFiles/${det}/${year}/${period}/dq-${det}-b-v00"
    iget -rV /KEK-T2K/home/dataquality/data/summaryFiles/${det}/${year}/${period}/dq-${det}-b-v00 .
fi

echo

# Remove any empty files, there shouldn't be any though.
echo "Searching for and removing any empty files, any located will be listed below:"
find . -size 0 -type f -delete -print
echo

cd $CURDIR/RunPeriods/$year/$period/BeamTiming

# Make file lists
ls -1 $PWD/Files/dq-${det}-b-v00/dq-${det}-b*.root > $CURDIR/RunPeriods/$year/$period/BeamTiming/$det/files.list

  #-------------------------------------------------------------------------------------#
  # Usage: BeamTimingData.exe -f <file list>                                            #
  # <file list>: path to the list of root files                                         #
  #-------------------------------------------------------------------------------------#
 
# Beam Timing
cd $CURDIR/RunPeriods/$year/$period/BeamTiming/$det
echo "Processing beam timing files"
$CURDIR/macros/BeamTimingData.exe -f files.list > beamtiming.out

echo "DONE."
#!/bin/bash

# Written by M. Lawe

CURDIR=$(dirname $PWD)
source $CMTROOT/mgr/setup.sh
source $SOFFTASKSROOT/cmt/setup.sh
cd $CURDIR

# Period must be Sunday to Saturday with format MMDD-MMDD, where MM = month and DD = day. e.g. 0610-0616
read -p "Choose a detector (ecal, p0d, smrd): " det
echo
read -p "Enter week period (MMDD-MMDD): " period 
read -p "Enter year (YYYY): " year
echo
read -p "Enter first Run of the week: " first
read -p "Enter last Run of the week: " last
echo
read -p "Do you want to compare gain and peds to a specified reference file? (y/n) " useref
echo

case $useref in
    [Yy]* ) read -p "Enter a reference Run: " refrun;;
    [Nn]* ) refrun="";;
    * ) echo "Answer not valid, no reference will be used"; refrun="";;
esac

# Set detector names and abbreviations 
if [ $det = "ecal" ]; then
    DET="ECAL"
    DET_A="E"
elif [ $det = "p0d" ]; then
    DET="P0D"
    DET_A="P"
elif [ $det = "smrd" ]; then 
    DET="SMRD"
    DET_A="S"
else
    echo "Invalid detector name, exiting"; exit
fi


# Make a working directory
mkdir -p $CURDIR/RunPeriods/$year/$period/Channels/$det $CURDIR/RunPeriods/$year/$period/Gain/$det $CURDIR/RunPeriods/$year/$period/Gain/Files $CURDIR/RunPeriods/$year/$period/Ped/$det

cd $CURDIR/RunPeriods/$year/$period/Gain/Files

echo "Retrieving Gain and Ped files"
echo
for (( i=first; i<last+1; i++))
do  
    if [ -d "nd280_000${i}" ]; then
	echo "Directory Gain/Files/nd280_000${i} already exists, you must have downloaded these files already, perhaps when doing the DQ for another detector."
	echo "If you wish to download the files again, completely remove the directory nd280_000${i} from $PWD."
    else
	echo "Working on directory /KEK-T2K/home/dataquality/data/DPT/nd280_000${i}"
	iget -rV /KEK-T2K/home/dataquality/data/DPT/nd280_000$i .
    fi
done

echo

# Get reference file if required
if [[ ! -z $refrun ]]; then  
    echo "Retrieving reference file"
    mkdir -p ref
    if [ -d "ref/nd280_000${refrun}" ]; then
	echo "Directory Gain/Files/ref/nd280_000${refrun} already exists, you must have downloaded these files already, perhaps when doing the DQ for another detector."
        echo "If you wish to download the files again, completely remove the directory nd280_000${refrun} from $PWD/ref."
    else
        echo "Working on directory /KEK-T2K/home/dataquality/data/DPT/nd280_000${refrun}"
        iget -rV /KEK-T2K/home/dataquality/data/DPT/nd280_000$refrun ref/.
    fi
fi

# Remove any empty files, they'll crash the getgainTripT.exe/getpedTripT.exe executables otherwise
echo "Searching for and removing any empty files, any located will be listed below:"
find . -size 0 -type f -delete -print
echo

cd $CURDIR/RunPeriods/$year/$period/Gain

# Make file lists
ls -1 $PWD/Files/nd280_*/nd280_*_gainDrift/${DET_A}PEU_DBREADY*.dat > $CURDIR/RunPeriods/$year/$period/Gain/$det/gain.list
ls -1 $PWD/Files/nd280_*/nd280_*_dptPeds/${DET_A}.*.DPT.db.ped.dat > $CURDIR/RunPeriods/$year/$period/Ped/$det/ped.list

# Set reference file if required
refgainfile=""
refpedfile=""

if [[ ! -z $refrun ]]; then
    refgainfile="$PWD/Files/ref/nd280_000${refrun}/nd280_000${refrun}_0000_gainDrift/${DET_A}PEU_DBREADY*.dat"
    refpedfile="$PWD/Files/ref/nd280_000${refrun}/nd280_000${refrun}_0000_dptPeds/${DET_A}.*.DPT.db.ped.dat"
fi

  #-------------------------------------------------------------------------------------#
  # Usage: getgainTripT.exe <detector> <file list> [<ref file>]                         #
  # <detector>:  ECAL/P0D/SMRD                                                          #
  # <file list>: path to the list of constants files                                    #
  # <ref file>:  path to the reference constants file                                   #
  #              (optional, if not specified the first file from the list will be used) #
  #-------------------------------------------------------------------------------------#
 
# Gain drift
cd $CURDIR/RunPeriods/$year/$period/Gain/$det
echo "Processing gain files"
getgainTripT.exe $DET gain.list $refgainfile > gain.out
# Adjust output file name
gainrootfile="DQGainPlots${DET}.root"
if [[ -z $refrun ]]; then
    gainrootfile=${gainrootfile/".root"/"_${year}_${period}_${first}_${last}_ref${first}.root"}
else
    gainrootfile=${gainrootfile/".root"/"_${year}_${period}_${first}_${last}_ref${refrun}.root"}
fi
mv DQGainPlots${DET}.root $gainrootfile
ls ${gainrootfile} > files.list
# Produce summary plots
echo "Producing gain plots"
root -l -b -q $CURDIR/macros/plot_gaindrift.C

# Ped drift
cd $CURDIR/RunPeriods/$year/$period/Ped/$det
echo "Processing ped files"
getpedTripT.exe $DET ped.list $refpedfile > ped.out
# Adjust output file name
pedrootfile="DQPedPlots${DET}.root"
if [[ -z $refrun ]]; then
    pedrootfile=${pedrootfile/".root"/"_${year}_${period}_${first}_${last}_ref${first}.root"}
else
    pedrootfile=${pedrootfile/".root"/"_${year}_${period}_${first}_${last}_ref${refrun}.root"}
fi
mv DQPedPlots${DET}.root $pedrootfile
ls ${pedrootfile} > files.list
echo "Producing ped plots"  
root -l -b -q $CURDIR/macros/plot_peddrift.C

# Channels
cd $CURDIR/RunPeriods/$year/$period/Channels/$det
echo "Producing channel plots"
ls $CURDIR/RunPeriods/$year/$period/Gain/$det/$gainrootfile > files.list
root -l -b -q $CURDIR/macros/plot_channels.C

echo "DONE."

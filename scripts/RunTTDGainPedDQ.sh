#!/bin/bash

### AUTHOR: MATTHEW LAWE

f=$(readlink -f $0)
. ${f%/*}/dateutils.sh

# source /home/t2k/mlawe/t2k/nd280devel/soffTasks/v1r50/cmt/setup.sh
source $SOFFTASKSROOT/cmt/setup.sh

CURDIR=$(dirname ${f%/*})

USAGE="Usage: ${0##*/} [OPTION...] <ecal|p0d|smrd>
Options:
  -w, --week=WEEK     Week relative to current week for which the plots
                      and presentation are generated. The default value 
                      is ${week} (Ie show data from last week).
  -f,--first=RUN      First run of the week
  -l,--last=RUN       Last run of the week
  -?, --help          Show this help list."


week="-1"
first=""
last=""
det=""
refrun=""
while [ "$1" != "" ] ; do
    case "$1" in
        -w|--week)
            week="$2"
            shift;;
        -l|--last)
            last="$2"
            shift;;
        -f|--first)
            first="$2"
            shift;;
        -r|--ref-run)
            refrun="$2"
            shift;;
        ecal|smrd|p0d)
            det=$1
            ;;
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

# These can be set on the commandline... remove this interactive part?
if [ "$det" = "" ] ; then
    read -p "Choose a detector (ecal, p0d, smrd): " det
    echo
fi
if [ "$first" = "" ] ; then
    read -p "Enter first Run of the week: " first
fi
if [ "$last" = "" ] ; then
    read -p "Enter last Run of the week: " last
    echo
fi

if [ "${det}" = "" ]; then
    echo $USAGE
    exit 1
fi

# Set detector names and abbreviations
DET=${det^^} # Uppercase
DET_A=${DET:0:1} # First letter

date_dir=$(dateDir $week)

# Make a working directory
mkdir -p $CURDIR/RunPeriods/$date_dir/Channels/$det $CURDIR/RunPeriods/$date_dir/Gain/$det $CURDIR/RunPeriods/$date_dir/Gain/Files $CURDIR/RunPeriods/$date_dir/Ped/$det

cd $CURDIR/RunPeriods/$date_dir/Gain/Files

echo "Retrieving Gain and Ped files"
echo
for (( i=first; i<last+1; i++))
do
    dirname="$(printf 'nd280_%08i' $i)"
    if [ -d "$dirname" ]; then
	echo "Directory Gain/Files/${dirname} already exists, you must have downloaded these files already, perhaps when doing the DQ for another detector."
	echo "If you wish to download the files again, completely remove the directory ${dirname} from $PWD."
    else
	echo "Working on directory /KEK-T2K/home/dataquality/data/DPT/${dirname}"
	iget -rV /KEK-T2K/home/dataquality/data/DPT/${dirname} .
    fi
done

echo

# Get reference file if required
if [[ ! -z $refrun ]]; then  
    echo "Retrieving reference file"
    mkdir -p ref

    refname="$(printf 'nd280_%08i' $refrun)"
    if [ -d "ref/${refname}" ]; then
	echo "Directory Gain/Files/ref/${refname} already exists, you must have downloaded these files already, perhaps when doing the DQ for another detector."
        echo "If you wish to download the files again, completely remove the directory ${refname} from $PWD/ref."
    else
        echo "Working on directory /KEK-T2K/home/dataquality/data/DPT/${refname}"
        iget -rV /KEK-T2K/home/dataquality/data/DPT/${refname} ref/.
    fi
fi

# Remove any empty files, they'll crash the getgainTripT.exe/getpedTripT.exe executables otherwise
echo "Searching for and removing any empty files, any located will be listed below:"
find . -size 0 -type f -delete -print
echo

cd $CURDIR/RunPeriods/$date_dir/Gain

# Make file lists
ls -1 $PWD/Files/nd280_*/nd280_*_gainDrift/${DET_A}PEU_DBREADY*.dat > $CURDIR/RunPeriods/$date_dir/Gain/$det/gain.list
ls -1 $PWD/Files/nd280_*/nd280_*_dptPeds/${DET_A}.*.DPT.db.ped.dat > $CURDIR/RunPeriods/$date_dir/Ped/$det/ped.list

# Set reference file if required
refgainfile=""
refpedfile=""

if [[ ! -z $refrun ]]; then
    refgainfile="$PWD/Files/ref/${refname}/${refname}_0000_gainDrift/${DET_A}PEU_DBREADY*.dat"
    refpedfile="$PWD/Files/ref/${refname}/${refname}_0000_dptPeds/${DET_A}.*.DPT.db.ped.dat"
fi

  #-------------------------------------------------------------------------------------#
  # Usage: getgainTripT.exe <detector> <file list> [<ref file>]                         #
  # <detector>:  ECAL/P0D/SMRD                                                          #
  # <file list>: path to the list of constants files                                    #
  # <ref file>:  path to the reference constants file                                   #
  #              (optional, if not specified the first file from the list will be used) #
  #-------------------------------------------------------------------------------------#
 
# Gain drift
cd $CURDIR/RunPeriods/$date_dir/Gain/$det
echo "Processing gain files"
getgainTripT.exe $DET gain.list $refgainfile > gain.out
# Adjust output file name
gainrootfile="DQGainPlots${DET}.root"
datename=$(echo $date_dir | tr '/' '-')
if [[ -z $refrun ]]; then
    gainrootfile=${gainrootfile/".root"/"_${datename}_${first}_${last}_ref${first}.root"}
else
    gainrootfile=${gainrootfile/".root"/"_${datename}_${first}_${last}_ref${refrun}.root"}
fi
mv DQGainPlots${DET}.root $gainrootfile
ls ${gainrootfile} > files.list
# Produce summary plots
echo "Producing gain plots"
root -l -b -q $CURDIR/macros/plot_gaindrift.C

# Ped drift
cd $CURDIR/RunPeriods/$date_dir/Ped/$det
echo "Processing ped files"
getpedTripT.exe $DET ped.list $refpedfile > ped.out
# Adjust output file name
pedrootfile="DQPedPlots${DET}.root"
if [[ -z $refrun ]]; then
    pedrootfile=${pedrootfile/".root"/"_${datename}_${first}_${last}_ref${first}.root"}
else
    pedrootfile=${pedrootfile/".root"/"_${datename}_${first}_${last}_ref${refrun}.root"}
fi
mv DQPedPlots${DET}.root $pedrootfile
ls ${pedrootfile} > files.list
echo "Producing ped plots"  
root -l -b -q $CURDIR/macros/plot_peddrift.C

# Channels
cd $CURDIR/RunPeriods/$date_dir/Channels/$det
echo "Producing channel plots"
ls $CURDIR/RunPeriods/$date_dir/Gain/$det/$gainrootfile > files.list
root -l -b -q $CURDIR/macros/plot_channels.C

echo "DONE."

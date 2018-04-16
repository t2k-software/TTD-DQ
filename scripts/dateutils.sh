### AUTHOR: MATTHEW LAWE (modified by Jacek M. Holeczek)

function dateDir
{
    # Input: Integer representing the week relative to the current week. Eg last week => -1
    # Return YYYY/MMDD-MMDD DQ week string
    local week=${1}
    if [ "`date +%w`" == "0" ] ; then # "0" is "Sunday"
        local sun="Sunday ${week} week";
    else
        local sun="Sunday $((week-1)) week"
    fi
    local sat="Saturday ${week} week"
    local date_data_dir=$(date +'%Y/%m%d-' --date="${sun}")$(date +'%m%d' --date="${sat}")
    echo ${date_data_dir}
}

function dateName
{
    # Input: Integer representing the week relative to the current week. Eg last week => -1
    # Return YYYY-MMDD-MMDD DQ week string
    dateDir ${1} | tr '/' '-'
}

function firstRunOfWeek
{
    # Input: Integer representing the week relative to the current week. Eg last week => -1
    local week=${1}
    if [ "`date +%w`" == "0" ] ; then # "0" is "Sunday"
        local sun="Sunday ${week} week";
    else
        local sun="Sunday $((week-1)) week"
    fi
    # local subrun_info="${2:-${HOME}/.data/nd280_subrun_info.sqlite}" # warning: uses TZ="Asia/Tokyo"
    subruninfo --start-min $(date +%Y-%m-%d --date="${sun}") -l |head -1 | cut -f1 -d,
    # subruninfo -i ${subrun_info} --start-min $(date +%Y-%m-%d --date="${sun}") |head -1 | cut -f1 -d\|
}

function lastRunOfWeek
{
    # Input: Integer representing the week relative to the current week. Eg last week => -1
    local week=${1}
    local sat="Saturday ${week} week"
    # local subrun_info="${2:-${HOME}/.data/nd280_subrun_info.sqlite}" # warning: uses TZ="Asia/Tokyo"
    subruninfo --start-max $(date +%Y-%m-%d --date="${sat}") -l |tail -1 | cut -f1 -d,
    # subruninfo -i ${subrun_info} --start-max $(date +%Y-%m-%d --date="${sat}") |tail -1 | cut -f1 -d\|
}

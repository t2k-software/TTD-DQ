# TTD-DQ

Guide to producing TTD Data Quality reports
## Introduction

This guide describes how to produce a TTD DQ report, the documentation is a work in progress, but if you find mistakes, have questions or wish to contribute please contact Matt Lawe, who will attempt to help.
## Software

The software used to produce a TTD DQ report is currently hosted on Matt's github. This fork is hosted on the t2k-software github account, with the core functionality also being present within the T2K CMT repository.

You can download the software yourself to prepare the DQ report, but is probably easiest to use the version already on the pontecorvo computer in the control room. The software can be found in the directory /home/t2kuser/TTD-DQ.

### Requirements

 * __(g)make__ : Comes with linux unless you removed it yourself. If you cannot compile software since you don't have gmake, just alias gmake as make!
 * __CMT__ : Details on [ND280 Software Workbook](http://nd280.lancs.ac.uk/devel/invariant/nd280Doc/workbook/SoftwareUser.html) under "Installation and Setup Instructions for bash users"
 * __nd280__ : v11r31 and v11r31p## with same details in the [Workbook](http://nd280.lancs.ac.uk/devel/invariant/nd280Doc/workbook/SoftwareUser.html). Make sure you have __soffTasks__
 * __iRODS__ : Installation instructions at [t2k.org](https://www.t2k.org/nd280/runco/data/quality/forexperts/DQiRODSfolder/DQiRODS)
 * __pdfLatex__ : Availabe with texLive, texLiveLatex, or MacTex for Mac OS X

### Installation

```bash 
git clone https://github.com/t2k-software/TTD-DQ.git
cd TTD-DQ
make
```

### Running the Package
```bash
./TTD_DQ.exe
```

### Software Structure

#### TTD-DQ : Top Level to the software
 * __beamer\_style__ : Beamer style files used when producing the presentation, it should not be necessary to look at these files.
 * __images__ : Image files used within the presentation (plot legends, T2K and institution logos), you may wish to add your institutions logo to this directory for us in the presentation.
 * __macros__ : Root macros used to draw the plots used in the presentation, the functionality of relevant macros will be described later. You may under rare circumstances need to adjust these macros, but it is unlikely.
 * __RunPeriods__ : Contains the files, plots and presentation for each week the DQ processing has been ran.
 * __scripts__ : Bash scripts used to control the processing, they download necessary files from the iRods server, run the processing macros and produce the final presentation. Most of the work is conducted from this directory.

### Scripts

Within the scripts directory are the following four scripts; two that control the download and processing of the data, one which produces the data quality report slides, and one master control script that runs the other three.

 * __RunTTDBeamTimingDQ.sh__ : Script to download the beam timing related data files from iRods and produce the associated data quality plots showing the bunch times, bunch separation and bunch widths. The script can be ran from the command line as ./RunTTDBeamTimingDQ.sh, and it will ask you to enter which TripT detector you want to process, which weekly period to cover, and the year. The necessary files are then downloaded from iRods and the executable BeamTimingData.exe from the macros directory is called to produce the plots.
 * __RunTTDGainPedDQ.sh__ : Script to download the gain and pedestal related data files from iRods and produce the associated data quality plots showing the gain (absolute and drift), pedestal drift (high and low gain channels), and channel information. The script can be ran from the command line as ./RunTTDGainPedDQ.sh, and it will ask you to enter which TripT detector you want to process, which weekly period to cover, the year, the first and last ND280 run numbers of the week, and if you would like to compare the gains to reference file, and if so what file. A reference file should in general always be used, and this should be an ND280 run taken shortly after the HV trims were last updated for the relevant detector. The necessary files are then downloaded from iRods and the executables getgainTripT.exe and getpedTripT.exe from the soffTasks package are called to do some initial processing. These are followed by the CINT root macros plot\_gaindrift.C, plot\_peddrift.C and plot\_channels.C from the macros directory which then produce the plots.
 * __MakeSlides.sh__ : Script to produce and compile the beamer slides for the data quality report. The script can be ran from the command line as ./MakeSlides.sh, and it will ask you to enter which weekly period to cover, the year, and your name. It will then write a .tex file for the beamer presentation, and compile it using pdflatex if installed.
 * __RunAllTTDDQ.sh__ : Master control script which will run RunTTDBeamTimingDQ.sh and RunTTDGainPedDQ.sh for each TripT detector in turn, asking you for relevant inputs as required. It will then call MakeSlides.sh to produce and compile the data quality presentation.

### Macros

Within the macros directory are the following.

 * __BeamTimingData.cxx/hxx__ : Beam timing plotting files. These should be compiled _using the provided Makefile_ into the executable BeamTimingData.exe. The executable is then called to make all the beam timing plots.
 * __plot\_gaindrift.C__ : CINT root macro for plotting gain (drift) for each sub-detector.
 * __plot\_peddrift.C__ : CINT root macro for plotting high and low gain pedestal drift for each sub-detector.
 * __plot\_channels.C__ : CINT root macro for plotting the number of scanned, bad, dead, underflow and overflow channels from the gain data.
 * __LegendDraw.C__ :  CINT root macro for generating the legends for the beam timing plots. It should not be necessary to use this macro as the legends should already be stored in the images directory.

## Preparing the TTD DQ Report
### Keeping Informed

The first thing you need to do is ensure you are on the nd280-dataquality@mailman.t2k.org mailing list. This mailing list is used to announce when the semi-offline processing for the previous week has completed. The processing is started each Sunday, and usually completes by the Monday. Once the processing is complete Nick Hastings will email the data quality mailing list and you can then begin to generate the report.

### Generating the Report

Once the processing has completed you can begin to generate the TTD DQ report. First you need to find out the following information:

 * __Run Period__ : This is the dates of the week for which you are doing the processing, in the form MMDD-MMDD. Here the first date is Sunday at the start of the previous week, and the second date is the following Saturday.
 * __Year__ : Self explanatory I hope!
 * __First Run of the Period__ : The 5 digit ND280 run number that was going as the first date of the run period started (be aware that the run was probably started the day before). This information is best obtained by checking the Run Control elog.
 * __Last__ Run of the Period : The 5 digit ND280 run number that was going as the last date of the run period finished. This information is best obtained by checking the Run Control elog.
 * __Reference Gain File (for each TripT Detector)__ : The 5 digit ND280 run number that was taken shortly after the HV Trims for each TripT detector were taken. The run needs to have been written to disc and lasted a while to have collected sufficient statistics to do the comparison against. The reference files can be (and probably will be) different for each TripT detector. To find the relevant files you'll probably need to cross reference the Run Control elog against the Subsystem elogs (ECal, P0D, SMRD). If the HV Trims haven't changed within the past week then the reference files will not have changed since the previous processing.

It is then possible to proceed with creating the TTD DQ report. Navigate to the scripts directory within the TTD DQ software and execute the script ./RunAllTTDDQ.sh. You will be promoted to input relevant information on the command line, and the script will proceed to download the relevant files and run the data processing. Watch the terminal for obvious errors thrown by the processing.

All being well a pdf containing the slides will be produced and its location will be written to the terminal; it will be of the form 

    /home/t2kuser/TTD-DQ/RunPeriods/<year>/<MMDD-MMDD>/Slides/ttd_dq_slides_<year>-<MMDD-MMDD>.pdf 
    
You should check through the slides ensuring that they have been correctly populated. If there are any issues with the beam timing or gain/ped plots for a given sub-detector these can be reprocessed using the scripts ./RunTTDBeamTimingDQ.sh and ./RunTTDGainPedDQ.sh respectively. The slides should then be regenerated with ./MakeSlides.sh.

A detailed description of the slides and the relevant checks to make will be described in the next section, but once you are satisfied with slides they need distributing to the DQ experts for each sub-detector for checking over.
Distributing the Report

After the report has been produced this should be distributed to the sub-detector DQ experts for them to check. This should be done on Monday when possible (sometimes the processing will finish late and you may have to delay distributing until the Tuesday) to give the experts time to go over the data prior to the DQ meeting on Wednesday.

You should send an email to the experts contain the pdf slides, along with the root files produced when processing the ped and gain for each sub-detector. The root files will be found in directories with the following structure 

    /home/t2kuser/TTD-DQ/RunPeriods/<year>/<MMDD-MMDD>/<Gain or Ped>/<sub-detector>/DQ<Gain or Ped>Plots<sub-detector>_<year>_<MMDD-MMDD>_<FirstRun>_<LastRun>_ref<ReferenceRun>.root

In your email you should also include any information relevant to the DQ for the previous week (e.g. operational problems with a given sub-detector).

The DQ experts for each sub-detector are currently:

* __ECal__ : __Stephane Zsoldos__ (s.zsoldos@qmul.ac.uk)
* __P0D__ : ~~__Jackie Schwehr__ (jackie.schwehr@colostate.edu)~~ __Hang Su__ (has137@pitt.edu)
* __SMRD__ : __Nikolai Yershov__ (yershov@inr.ru) and __Kamil Porwit__ (kporwit@us.edu.pl)

## Detailed Checks

For detailed information go to the [TTD-DQ Guide](https://www.t2k.org/nd280/ttd/ttddqguide "t2k.org TTD-DQ Guide")

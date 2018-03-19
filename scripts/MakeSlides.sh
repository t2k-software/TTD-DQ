#!/bin/bash

f=$(readlink -f $0)
. ${f%/*}/dateutils.sh

CURDIR=$(dirname ${f%/*})

USAGE="Usage: ${0##*/} [OPTION...]
Create presentation file TTD data quality assesment.

Options:
  -w, --week=WEEK     Week relative to current week for which the plots
                      and presentation are generated. The default value 
                      is ${week} (Ie show data from last week).
  -a, --author=NAME   TTD experts name.
  -?, --help          Show this help list."
week="-1"
author="TTD Expert"
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

date_dir=$(dateDir $week)
date_name=$(echo $date_dir | tr '/' '-')

# Make a working directory
mkdir -p $CURDIR/RunPeriods/$date_dir/Slides
cd $CURDIR/RunPeriods/$date_dir/Slides

SLIDESNAME=ttd_dq_slides_$date_name.tex

# Make slides tex file.
cat <<EOF > $SLIDESNAME
\documentclass{beamer}
\usepackage[english] {babel}
\usepackage[T1]      {fontenc}
\usepackage{amsmath, amsfonts, graphicx}
\usepackage{bibunits, tikz, version}
\usepackage{multicol}
\usetheme[pageofpages=of,% String used between the current page and the
% total page count.
alternativetitlepage=true,% Use the fancy title page.
titlepagelogo=${CURDIR}/images/t2k_logo_medium,% Logo for the first page.
]{Torino}
\usecolortheme{nouvelle}

\AtBeginSection[]{
  \begin{frame}
  \vfill
  \centering
  \begin{beamercolorbox}[sep=8pt,center,shadow=true,rounded=true]{title}
    \usebeamerfont{title}\insertsectionhead\par%
  \end{beamercolorbox}
  \vfill
  \end{frame}
}

\title{TTD Data Quality Assessment}
\subtitle{Data Quality Checks for the period : ${date_dir}}
\author{${author}}
\date{\today}

\logo{\includegraphics[height=0.1\paperheight]{${CURDIR}/images/t2k_logo_medium.png}}

\institute{\includegraphics[scale=0.16]{${CURDIR}/images/LU-Logo-PositiveRGBweb.jpg}}
%\institute{\includegraphics[scale=0.032]{${CURDIR}/images/QM_logo.png}}

\begin{document}
\maketitle

\begin{frame}{Overview}
  \begin{multicols}{2}
    \tableofcontents
  \end{multicols}
\end{frame}
EOF

# Loop over each detector adding all the plots in for each one
for det in ecal p0d smrd; do
    
    # Set detector titles 
    if [ $det = "ecal" ]; then
	DET="ECal"
	DET_U=$DET
	DET_Title="ECal Data Quality"
	let NRMM=12
    elif [ $det = "p0d" ]; then
	DET="P\O{}D"
	DET_U="P0D"
	DET_Title="P\O{}D Data Quality"
	let NRMM=6
    elif [ $det = "smrd" ]; then 
	DET="SMRD"
	DET_U=$DET
	DET_Title="SMRD Data Quality"
	let NRMM=4
    fi

    # Beam Timing Plots
    cat <<EOF >> $SLIDESNAME

\section{${DET_Title}}
EOF

    for plot in timing width separation; do

	if [ $plot = "timing" ]; then
	    TITLE="Timing"
	    LEGEND="BunchMeanWidth"
	elif [ $plot = "width" ]; then
	    TITLE="Width"
	    LEGEND="BunchMeanWidth"
	elif [ $plot = "separation" ]; then
	    TITLE="Separation"
	    LEGEND="BunchSeparation"
	fi

	if [ ! $(ls -1 ${CURDIR}/RunPeriods/${year}/${date_dir}/BeamTiming/${det}/${DET_U}_bunch${plot}_weekly_*.png | wc -l) -eq $((${NRMM}+1)) ]; then
	    echo "Did not find $((${NRMM}+1)) bunch ${plot} plots for the ${det}, these plots will not be added to the presentation."
	    echo
	else

            # Beam Timing Plots
	    cat <<EOF >> $SLIDESNAME

\subsection{${DET} Bunch ${TITLE}}
\begin{frame}{${DET} Bunch ${TITLE} (All RMMs) 1/$((${NRMM}/2+1))}
  \begin{center}
    \includegraphics[width=0.6\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/BeamTiming/${det}/${DET_U}_bunch${plot}_weekly_all.png}
    \hspace{0.5cm}
    \includegraphics[width=0.3\textwidth]{${CURDIR}/images/${LEGEND}Legend.png}
  \end{center}
\end{frame}
EOF

	    let imax=${NRMM}/2;
	    for ((i=1; i<=$imax; i++)); do
		cat <<EOF >> $SLIDESNAME
\begin{frame}{${DET} Bunch ${TITLE} (by RMM) $((${i}+1))/$((${NRMM}/2+1))}
  \begin{center}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/BeamTiming/${det}/${DET_U}_bunch${plot}_weekly_rmm$((${i}*2-2)).png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/BeamTiming/${det}/${DET_U}_bunch${plot}_weekly_rmm$((${i}*2-1)).png}
  \end{center}
\end{frame}
EOF
	    done # Loop over number of slides
	fi
    done # Loop over timing plots
    
    # Gain Plots    
    
    let imax=${NRMM}/6;
    if [ $imax -eq 0 ]; then 
	let imax=1;
    fi
    
    if [ ! $(ls -1 ${CURDIR}/RunPeriods/${year}/${date_dir}/Gain/${det}/gain*${DET_U}_*.png | wc -l) -eq $(((${NRMM}+1)*2)) ]; then
        echo "Did not find $(((${NRMM}+1)*2)) gain plots for the ${det}, these plots will not be added to the presentation."
        echo
    else

	cat <<EOF >> $SLIDESNAME

\subsection{${DET} Gain Drift}
\begin{frame}{${DET} Gain Drift (All RMMs) 1/$((${imax}+1))}
  \begin{center}
    \includegraphics[width=0.68\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Gain/${det}/gainnew${DET_U}_All.png}
    \\
    \includegraphics[width=0.68\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Gain/${det}/gainDriftnew${DET_U}_All.png}
  \end{center}
\end{frame}
EOF
    
	for ((i=1; i<=$imax; i++)); do
	    cat <<EOF >> $SLIDESNAME
\begin{frame}{${DET} Gain Drift (by RMM) $((${i}+1))/$((${imax}+1))}
  \begin{center}
EOF
	    if [ $det = "smrd" ]; then
		let jmax=2
	    else
		let jmax=3
	    fi
	    for ((j=$jmax; j>=1; j--)); do
		cat <<EOF >> $SLIDESNAME
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Gain/${det}/gainDriftnew${DET_U}_RMM$((${i}*2*${jmax}-2*${j})).png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Gain/${det}/gainDriftnew${DET_U}_RMM$((${i}*2*${jmax}-2*${j}+1)).png}
    \\
EOF
	    done # Loop over figures on slide
	
	    cat <<EOF >> $SLIDESNAME
  \end{center}
\end{frame}
EOF
	done # Loop over number of slides
    fi

    if [ ! $(ls -1 ${CURDIR}/RunPeriods/${year}/${date_dir}/Ped/${det}/peddrift*${DET_U}_*.png | wc -l) -eq $((${NRMM}*2)) ]; then
        echo "Did not find $((${NRMM}*2)) pedestal plots for the ${det}, these plots will not be added to the presentation."
        echo
    else
	
        # Pedestal Plots    
	cat <<EOF >> $SLIDESNAME

\subsection{${DET} Pedestal Drift}
EOF
    
	let k=1
	for gain in Low High; do
	    
	    let imax=${NRMM}/6;
	    if [ $imax -eq 0 ]; then 
		let imax=1;
	    fi
	    for ((i=1; i<=$imax; i++)); do
		cat <<EOF >> $SLIDESNAME
\begin{frame}{${DET} Pedestal Drift (${gain} Gain by RMM) ${k}/$((${imax}*2))}
  \begin{center}
EOF
		if [ $det = "smrd" ]; then
		    let jmax=2
		else
		    let jmax=3
		fi
		for ((j=$jmax; j>=1; j--)); do
		    cat <<EOF >> $SLIDESNAME
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Ped/${det}/peddrift${gain}new${DET_U}_RMM$((${i}*2*${jmax}-2*${j})).png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Ped/${det}/peddrift${gain}new${DET_U}_RMM$((${i}*2*${jmax}-2*${j}+1)).png}
    \\
EOF
		done # Loop over figures on slide
		
		cat <<EOF >> $SLIDESNAME
  \end{center}
\end{frame}
EOF
		
		let k++
	    done # Loop over number of slides
	done # Loop over pedestal gains
    fi
    
    if [ ! $(ls -1 ${CURDIR}/RunPeriods/${year}/${date_dir}/Channels/${det}/*Channels${DET_U}.png | wc -l) -eq 5 ]; then
        echo "Did not find 5 channel plots for the ${det}, these plots will not be added to the presentation."
        echo
    else

        # Channels Plots
	cat <<EOF >> $SLIDESNAME
\subsection{${DET} Channels}
\begin{frame}{${DET} Channels Info (from Gain Files)}
  \begin{center}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Channels/${det}/DeadChannels${DET_U}.png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Channels/${det}/BadChannels${DET_U}.png}
    \\
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Channels/${det}/OverChannels${DET_U}.png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Channels/${det}/underChannels${DET_U}.png}
    \\
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${date_dir}/Channels/${det}/totChannels${DET_U}.png}
  \end{center}
\end{frame}

EOF
    fi

done # Loop over detectors

cat <<EOF >> $SLIDESNAME

\end{document}
EOF

echo "Slides made, running pdflatex..."

export TEXINPUTS=$CURDIR/beamer_style:$TEXINPUTS

pdflatex -output-directory $CURDIR/RunPeriods/$date_dir/Slides $CURDIR/RunPeriods/$date_dir/Slides/$SLIDESNAME
pdflatex -output-directory $CURDIR/RunPeriods/$date_dir/Slides $CURDIR/RunPeriods/$date_dir/Slides/$SLIDESNAME

echo
echo "Check output slides in" $CURDIR/RunPeriods/$date_dir/Slides
echo "DONE."

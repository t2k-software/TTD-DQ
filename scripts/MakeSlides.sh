#!/bin/bash

CURDIR=$(dirname $PWD)
cd $CURDIR

# Period must be Sunday to Saturday with format MMDD-MMDD, where MM = month and DD = day. e.g. 0610-0616
read -p "Enter week period (MMDD-MMDD): " period 
read -p "Enter year (YYYY): " year
echo
read -p "Enter your name: " author

month1=${period:0:2}
day1=${period:2:2}
month2=${period:5:2}
day2=${period:7:2}

# Make a working directory
mkdir -p $CURDIR/RunPeriods/$year/$period/Slides

cd $CURDIR/RunPeriods/$year/$period/Slides
for f in $CURDIR/beamer_style/*sty; do
    if [ ! -f $(basename $f) ]; then
	ln -s $f $(basename $f)
    fi
done

SLIDESNAME=ttd_dq_slides_$year-$period.tex

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
\subtitle{Data Quality Checks for the period : ${day1}/${month1}/${year} - ${day2}/${month2}/${year}}
\author{${author}}
\date{\today}

\logo{\includegraphics[height=0.1\paperheight]{${CURDIR}/images/t2k_logo_medium.png}}

%\institute{\includegraphics[scale=0.032]{images/QM_logo.png}}

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
    
    # Beam Timing Plots (Time)
    cat <<EOF >> $SLIDESNAME

\section{${DET_Title}}
\subsection{${DET} Bunch Timing}
\begin{frame}{${Det} Bunch Timing (All RMMs) 1/$((${NRMM}/2+1))}
  \begin{center}
    \includegraphics[width=0.6\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/BeamTiming/${det}/${DET_U}_bunchtiming_weekly_all.png}
    \hspace{0.5cm}
    \includegraphics[width=0.3\textwidth]{${CURDIR}/images/BunchMeanWidthLegend.png}
  \end{center}
\end{frame}
EOF

    let imax=${NRMM}/2;
    for ((i=1; i<=$imax; i++)); do
	cat <<EOF >> $SLIDESNAME
\begin{frame}{${Det} Bunch Timing (by RMM) $((${i}+1))/$((${NRMM}/2+1))}
  \begin{center}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/BeamTiming/${det}/${DET_U}_bunchtiming_weekly_rmm$((${i}*2-2)).png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/BeamTiming/${det}/${DET_U}_bunchtiming_weekly_rmm$((${i}*2-1)).png}
  \end{center}
\end{frame}
EOF
    done # Loop over number of slides

    # Beam Timing Plots (Width)
    cat <<EOF >> $SLIDESNAME

\subsection{${DET} Bunch Width}
\begin{frame}{${Det} Bunch Width (All RMMs) 1/$((${NRMM}/2+1))}
  \begin{center}
    \includegraphics[width=0.6\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/BeamTiming//${det}/${DET_U}_bunchwidth_weekly_all.png}
    \hspace{0.5cm}
    \includegraphics[width=0.3\textwidth]{${CURDIR}/images/BunchMeanWidthLegend.png}
  \end{center}
\end{frame}
EOF

    let imax=${NRMM}/2;
    for ((i=1; i<=$imax; i++)); do
	cat <<EOF >> $SLIDESNAME
\begin{frame}{${Det} Bunch Width (by RMM) $((${i}+1))/$((${NRMM}/2+1))}
  \begin{center}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/BeamTiming/${det}/${DET_U}_bunchwidth_weekly_rmm$((${i}*2-2)).png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/BeamTiming/${det}/${DET_U}_bunchwidth_weekly_rmm$((${i}*2-1)).png}
  \end{center}
\end{frame}
EOF
    done # Loop over number of slides

    # Beam Timing Plots (Separation)
    cat <<EOF >> $SLIDESNAME

\subsection{${DET} Bunch Separation}
\begin{frame}{${Det} Bunch Separation (All RMMs) 1/$((${NRMM}/2+1))}
  \begin{center}
    \includegraphics[width=0.6\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/BeamTiming/${det}/${DET_U}_bunchseparation_weekly_all.png}
    \hspace{0.5cm}
    \includegraphics[width=0.3\textwidth]{${CURDIR}/images/BunchSeparationLegend.png}
  \end{center}
\end{frame}
EOF

    let imax=${NRMM}/2;
    for ((i=1; i<=$imax; i++)); do
	cat <<EOF >> $SLIDESNAME
\begin{frame}{${Det} Bunch Separation (by RMM) $((${i}+1))/$((${NRMM}/2+1))}
  \begin{center}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/BeamTiming/${det}/${DET_U}_bunchseparation_weekly_rmm$((${i}*2-2)).png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/BeamTiming/${det}/${DET_U}_bunchseparation_weekly_rmm$((${i}*2-1)).png}
  \end{center}
\end{frame}
EOF
    done # Loop over number of slides


    # Gain Plots    
    cat <<EOF >> $SLIDESNAME

\subsection{${DET} Gain Drift}
EOF
    
    let imax=${NRMM}/6;
    if [ $imax -eq 0 ]; then 
	let imax=1;
    fi
    for ((i=1; i<=$imax; i++)); do
	cat <<EOF >> $SLIDESNAME
\begin{frame}{${DET} Gain Drift ${i}/${imax}}
  \begin{center}
EOF
	if [ $det = "smrd" ]; then
	    let jmax=2
	else
	    let jmax=3
	fi
	for ((j=$jmax; j>=1; j--)); do
	    cat <<EOF >> $SLIDESNAME
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/Gain/${det}/gaindriftnew${DET_U}_RMM$((${i}*2*${jmax}-2*${j})).png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/Gain/${det}/gaindriftnew${DET_U}_RMM$((${i}*2*${jmax}-2*${j}+1)).png}
    \\
EOF
	done # Loop over figures on slide
	
	cat <<EOF >> $SLIDESNAME
  \end{center}
\end{frame}
EOF
    done # Loop over number of slides
    

    # Pedestal Plots    
    cat <<EOF >> $SLIDESNAME

\subsection{${DET} Pedestal Drift}
EOF
    
    let k=1
    for gain in low high; do
	
	let imax=${NRMM}/6;
	if [ $imax -eq 0 ]; then 
	    let imax=1;
	fi
	for ((i=1; i<=$imax; i++)); do
	    cat <<EOF >> $SLIDESNAME
\begin{frame}{${DET} Pedestal Drift ${k}/$((${imax}*2))}
  \begin{center}
EOF
	    if [ $det = "smrd" ]; then
		let jmax=2
	    else
		let jmax=3
	    fi
	    for ((j=$jmax; j>=1; j--)); do
		cat <<EOF >> $SLIDESNAME
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/Ped/${det}/peddrift${gain}new${DET_U}_RMM$((${i}*2*${jmax}-2*${j})).png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/Ped/${det}/peddrift${gain}new${DET_U}_RMM$((${i}*2*${jmax}-2*${j}+1)).png}
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
    
    # Channels Plots
    cat <<EOF >> $SLIDESNAME
\subsection{${DET} Channels}
\begin{frame}{${DET} Channels Info (from Gain Files)}
  \begin{center}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/Channels/${det}/DeadChannels${DET_U}.png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/Channels/${det}/BadChannels${DET_U}.png}
    \\
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/Channels/${det}/OverChannels${DET_U}.png}
    \hspace{0.5cm}
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/Channels/${det}/underChannels${DET_U}.png}
    \\
    \includegraphics[width=0.45\textwidth]{${CURDIR}/RunPeriods/${year}/${period}/Channels/${det}/totChannels${DET_U}.png}
  \end{center}
\end{frame}

EOF


done # Loop over detectors

cat <<EOF >> $SLIDESNAME

\end{document}
EOF



echo "DONE."
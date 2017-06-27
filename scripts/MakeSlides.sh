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

# Make slides tex file.
cat <<EOF > ttd_dq_slides_$year-$period.tex
\documentclass{beamer}
\usepackage[english] {babel}
\usepackage[T1]      {fontenc}
\usepackage{amsmath, amsfonts, graphicx}
\usepackage{bibunits, tikz, version}
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
  \tableofcontents
\end{frame}
EOF

# Loop over each detector adding all the plots in for each one
for det in ecal p0d smrd; do
    
    # Set detector titles 
    if [ $det = "ecal" ]; then
	DET="ECal"
	DET_Title="ECal Data Quality"
    elif [ $det = "p0d" ]; then
	DET="P\O{}D"
	DET_Title="P\O{}D Data Quality"
    elif [ $det = "smrd" ]; then 
	DET="SMRD"
	DET_Title="SMRD Data Quality"
    fi
    
    cat <<EOF >> ttd_dq_slides_$year-$period.tex

\section{${DET_Title}}
\begin{frame}
Place holder\dots
\end{frame}
EOF
    
done    

cat <<EOF >> ttd_dq_slides_$year-$period.tex

\end{document}
EOF



echo "DONE."
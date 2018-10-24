#!/usr/bin/env bash

while getopts "f:s:e:" OPTS; do
	case $OPTS in
		"f")  LOGF=log/`ls log/ | grep -E "^.*(${OPTARG}).*$" | tail -1`;
			  LOGF=`echo $LOGF | sed -e "s/^\(.*\)\..*/\1/"`;
			  TSTART=`cat "${LOGF}.log" | awk 'BEGIN{min=10000} {if(min>$1){min=$1}} END{print min}'`;
			  TEND=`cat "${LOGF}.log" | awk 'BEGIN{max=-10000} {if(max<$1){max=$1}} END{print max}'`;
			  MINIDX=`cat "${LOGF}.log" | awk 'BEGIN{min=10000} {if(min>$10){min=$10; minidx=NR}} END{print minidx}'`;;
		"s")  TSTART=`cat "${LOGF}.log" | awk 'NR=='"${MINIDX}"'-'"${OPTARG}"' {print $1}'` ;;
		"e")  TEND=`cat "${LOGF}.log" | awk 'NR=='"${MINIDX}"'+'"${OPTARG}"' {print $1}'` ;;
	esac
done
# echo -en "\e[34m"; echo $LOGF; echo -en "\e[m"
# echo $LOGF | xsel --clipboard --input
# echo -en "\e[34m"; echo -n "TSTART="; echo $TSTART
# echo -n "TEND="; echo $TEND; echo -en "\e[m"

# set grid
# set xlabel 'Time [msec]'
# set ylabel 'Force [N]'
# set key right bottom
echo "set xrange [0:(${TEND}-${TSTART})*1000]"
echo "plot \"${LOGF}.log\" using (\$1-${TSTART})*1000:10 title \"${LOGF}Fz\", \"${LOGF}.log\" using (\$1-${TSTART})*1000:10 with line linewidth 1 title \"\""

LINEWIDTH=1

gnuplot <<EOF
set terminal pngcairo enhanced size 2000, 1000
# set terminal postscript eps color enhanced
# set tics font "Times New Roman,9"
# set xlabel font "Times New Roman,10"
# set ylabel font "Times New Roman,10"
# set key font "Times New Roman,9"
# set key right top
# set key spacing 0.8
set output "${LOGF}.png"
set grid
# set size ratio 0.25
# stats "${LOGF}.log" using 1:10
set multiplot layout 2,3 rowsfirst downward
set xlabel "Time [msec]"
set ylabel "Force [N]"
set xrange [0:("${TEND}"-"${TSTART}")*1000]
# set yrange [-10:10]
# set ytics 3
# set xtics 0.1
plot\
"${LOGF}.log" using (\$1-$TSTART)*1000:8 title "Fx",\
"${LOGF}.log" using (\$1-$TSTART)*1000:8 with line linewidth ${LINEWIDTH} title ""
plot\
"${LOGF}.log" using (\$1-$TSTART)*1000:9 title "Fy",\
"${LOGF}.log" using (\$1-$TSTART)*1000:9 with line linewidth ${LINEWIDTH} title ""
plot\
"${LOGF}.log" using (\$1-$TSTART)*1000:10 title "Fz",\
"${LOGF}.log" using (\$1-$TSTART)*1000:10 with line linewidth ${LINEWIDTH} title ""
set ylabel "Moment [Nm]"
plot\
"${LOGF}.log" using (\$1-$TSTART)*1000:11 title "Mx",\
"${LOGF}.log" using (\$1-$TSTART)*1000:11 with line linewidth ${LINEWIDTH} title ""
plot\
"${LOGF}.log" using (\$1-$TSTART)*1000:12 title "My",\
"${LOGF}.log" using (\$1-$TSTART)*1000:12 with line linewidth ${LINEWIDTH} title ""
plot\
"${LOGF}.log" using (\$1-$TSTART)*1000:13 title "Mz",\
"${LOGF}.log" using (\$1-$TSTART)*1000:13 with line linewidth ${LINEWIDTH} title ""
unset multiplot
EOF

#!/bin/bash

Pt=10.0
Gt=1.0
Gr=1.0

./ptd2ns2 -Pt $Pt -Gt $Gt -Gr $Gr > data.dat
gnuplot nakagami.plt

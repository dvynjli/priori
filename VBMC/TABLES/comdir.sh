rm $1/compiled/*
for f in $1/*.c; do ./cseq-feeder.py -i $f --viewSwitches 2 --rounds 1; done
rm $1/*.log
mv $1/_cs_* $1/compiled/

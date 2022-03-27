if [[ $# != 3 ]]; then
    echo $#
    echo usage: $0 input output title
    exit 1
fi

tmpfile=$(mktemp)
pdfcrop $1 $tmpfile
gs -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=$2 -c "[ /Title ($3) /DOCINFO pdfmark" -f $tmpfile
rm $tmpfile

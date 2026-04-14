EXEC="../test/parser"
cd "test_subjects"
for f in ./*.png; do
    echo "$($EXEC $f)" > "../reports/${f%.*}"
done
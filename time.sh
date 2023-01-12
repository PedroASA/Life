for number in {1..$1}; do
    time  mpirun -n $i $2 $3 2>>$4;	
done
MAX=$1
for i in $(seq 1 $MAX); do
	echo "Executando para n igual $i"; \
	/usr/bin/time -f "%E real\n%U user\n%S sys\n%P CPU\n" mpirun --oversubscribe -n $i $2 $3; 	
done	2>$4;

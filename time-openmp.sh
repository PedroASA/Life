MAX=$1
for i in $(seq 1 $MAX); do
	echo "Executando para n igual $i"; \
	export OMP_NUM_THREADS=$i; \
	/usr/bin/time -f "%E real\n%U user\n%S sys\n%P CPU\n" $2 <$3; 	
done	2>$4;

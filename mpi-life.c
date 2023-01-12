/*
 * The Game of Life
 *
 * a cell is born, if it has exactly three neighbours 
 * a cell dies of loneliness, if it has less than two neighbours 
 * a cell dies of overcrowding, if it has more than three neighbours 
 * a cell survives to the next generation, if it does not die of loneliness 
 * or overcrowding 
 *
 * In this version, a 2D array of ints is used.  A 1 cell is on, a 0 cell is off.
 * The game plays a number of steps (given by the input), printing to the screen each time.  'x' printed
 * means on, space means off.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

typedef unsigned char cell_t;
int procid, numprocs;
MPI_Status status;
int partition;

cell_t ** allocate_board (int size) {
	cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
	int	i;
	for (i=0; i<size; i++)
		board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
	return board;
}

void free_board (cell_t ** board, int size) {
        int     i;
        for (i=0; i<size; i++)
                free(board[i]);
	free(board);
}


/* return the number of on cells adjacent to the i,j cell */
int adjacent_to (cell_t ** board, int size, int i, int j) {
	int	k, l, count=0;
	
	int sk = (i>0) ? i-1 : i;
	int ek = (i+1 < size) ? i+1 : i;
	int sl = (j>0) ? j-1 : j;
        int el = (j+1 < size) ? j+1 : j;

	for (k=sk; k<=ek; k++)
		for (l=sl; l<=el; l++)
			count+=board[k][l];
	count-=board[i][j];
	return count;
}

void play (cell_t ** board, cell_t ** newboard, int line, int size) {
	int	i, j, a;
	i = line;
	for (j=0; j<size; j++) {
		a = adjacent_to (board, size, i, j);
		if (a == 2) newboard[i][j] = board[i][j];
		if (a == 3) newboard[i][j] = 1;
		if (a < 2) newboard[i][j] = 0;
		if (a > 3) newboard[i][j] = 0;
	}
}


/* print the life board */
void print (cell_t ** board, int size) {
	int	i, j;
	/* for each row */
	for (j=0; j<size; j++) {
		/* print each column position... */
		for (i=0; i<size; i++) 
			printf ("%c", board[i][j] ? 'x' : ' ');
		/* followed by a carriage return */
		printf ("\n");
	}
}

/* read a file into the life board */
void read_file (FILE * f, cell_t ** board, int size) {
	int	i, j;
	char	*s = (char *) malloc(size+10);
	char c;
	for (j=0; j<size; j++) {
		/* get a string */
		fgets (s, size+10,f);
		/* copy the string to the life board */
		for (i=0; i<size; i++)
		{
		 	//c=fgetc(f);
			//putchar(c);
			board[i][j] = s[i] == 'x';
		}
		//fscanf(f,"\n");
	}
}

void divide_and_play (cell_t** prev, cell_t** next, int size) {
	// Envio da Matriz incial
    if(procid == 0) {
		for(int i=1; i<numprocs; i++) {
			for(int j=i*partition; j<(i+1)*partition; j++) {
				MPI_Send(prev[j-1], size, MPI_CHAR, i, 0, MPI_COMM_WORLD); 
				MPI_Send(prev[j], size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
				if(j != size -1)
					MPI_Send(prev[j+1], size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
			}
        }
		// Possível Resto
		for(int j=partition*numprocs; j<size; j++) {
			MPI_Send(prev[j-1], size, MPI_CHAR, numprocs-1, 0, MPI_COMM_WORLD); 
			MPI_Send(prev[j], size, MPI_CHAR, numprocs-1, 0, MPI_COMM_WORLD);
			if(j != size -1)
				MPI_Send(prev[j+1], size, MPI_CHAR, numprocs-1, 0, MPI_COMM_WORLD);
		}
    }
	// Recebimento da Matriz incial 
    else { 
		for(int j=procid*partition; j<(procid+1)*partition; j++) {
			MPI_Recv(prev[j-1], size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
			MPI_Recv(prev[j], size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
			if(j != size -1)
				MPI_Recv(prev[j+1], size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
		}
		// Possível Resto
		if(procid == numprocs-1) {
			for(int j=partition*numprocs; j<size; j++) {
				MPI_Recv(prev[j-1], size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
				MPI_Recv(prev[j], size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
				if(j != size -1)
					MPI_Recv(prev[j+1], size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
			}
		}
	}
	// Execução do método "play()" em todos os processadores

	for(int j=procid*partition; j<(procid+1)*partition; j++) {
		play(prev, next, j, size);
	}

	// Possível Resto
	if(procid == numprocs-1) {
		for(int j=partition*numprocs; j<size; j++) {
			play(prev, next, j, size);
		}
	}

	// Recebimento da Matriz Processada
	if(procid == 0) {
		for(int i =1; i<numprocs; i++){
			for(int j=i*partition; j<(i+1)*partition; j++) {
				MPI_Recv(next[j], size, MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);
			}
		}

		// Possível Resto
		for(int j=partition*numprocs; j<size; j++) {
			MPI_Recv(next[j], size, MPI_CHAR, numprocs-1, 0, MPI_COMM_WORLD, &status);
		}
	} 
	// Envio da Matriz Processada
	else {
		for(int j=procid*partition; j<(procid+1)*partition; j++) {
			MPI_Send(next[j], size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		}

		// Possível Resto
		if(procid == numprocs-1) {
			for(int j=partition*numprocs; j<size; j++) {
				MPI_Send(next[j], size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
			}
		}
	}
}

int main (int argc, char** argv) {

	int size, steps;
	FILE    *f;
  	f = stdin;
    int ierr = MPI_Init(&argc,&argv);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &procid);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    if(procid == 0) {
		fscanf(f,"%d %d", &size, &steps);
		assert(size >= numprocs);
	}

	MPI_Bcast(&size,1, MPI_INT, 0, MPI_COMM_WORLD);
	partition = size / numprocs;
	MPI_Bcast(&steps,1, MPI_INT, 0, MPI_COMM_WORLD);

	cell_t ** prev = allocate_board (size);
	cell_t ** next = allocate_board (size);
	cell_t ** tmp;
	
	char* debug;
	int i,j;

	if(procid == 0) {
		read_file (f, prev,size);
		fclose(f);
		debug = getenv("DEBUG");
		if(debug) {
			printf("Initial \n");
			print(prev,size);
			printf("----------\n");
		}
	}
	for (i=0; i<steps; i++) {
		divide_and_play (prev,next,size);
		if(procid == 0) {
			if(debug) {
				printf("%d ----------\n", i);
				print (next,size);
			}
		}
		tmp = next;
		next = prev;
		prev = tmp;
	}

    if(procid == 0) {
		print (prev,size);
	}

	free_board(prev,size);
	free_board(next,size);
    MPI_Finalize();
}

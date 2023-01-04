#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int rank, size;

int Partner_bul(int phase) {
	int partner;
	if (phase % 2 == 0) {
		if (rank % 2 == 0)
			partner = rank + 1;
		else
			partner = rank - 1;
	}
	else {
		if (rank % 2 == 0)
			partner = rank - 1;
		else
			partner = rank + 1;
	}
	if (partner == -1 || partner == size)
		partner = MPI_PROC_NULL;
	return partner;
}

int main(int argc, char** argv) {
	// MPI ba�lang�� kurallar�n� y�kle
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Kullan�c�dan dizi uzunlu�unu al
	int n;
	if (rank == 0) {
		printf("Dizi Uzunlugunu Giriniz: ");
		fflush(stdout);
		scanf("%d", &n);
	}

	// Dizi uzunlu�unu t�m i�lemciler ile payla�
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	auto local_n = n / size;
	auto local_arr = new int[local_n];

	// Rastgele say� �retmeyi t�m i�lemler aras�nda da��t
	srand(time(NULL) * rank);
	for (auto i = 0; i < local_n; i++)	local_arr[i] = rand();


	// Lokal say�lar� kendi i�inde s�rala
	for (auto i = 0; i < local_n - 1; i++) {
		for (auto j = i + 1; j < local_n; j++) {
			if (local_arr[i] > local_arr[j]) {
				auto temp = local_arr[i];
				local_arr[i] = local_arr[j];
				local_arr[j] = temp;
			}
		}
	}

	// Odd-Even sort algoritmas�n� uygula
	MPI_Status status;
	for (auto phase = 0; phase < size; phase++) {
		auto partner = Partner_bul(phase);
		auto temp_arr = new int[local_n];
		if (partner != MPI_PROC_NULL) {
			auto partner_arr = new int[local_n];
			if (rank < partner) {
				// Listemi partner'a g�nder
				MPI_Send(local_arr, local_n, MPI_INT, partner, 0, MPI_COMM_WORLD);
				// Partner'�n listesini al
				MPI_Recv(partner_arr, local_n, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
				// K���k say�lar bende kals�n
				for (auto i = 0, k = 0, j = 0; k < local_n; k++)
					if (local_arr[i] > partner_arr[j]) {
						temp_arr[k] = partner_arr[j];
						j++;
					}
					else {
						temp_arr[k] = local_arr[i];
						i++;
					}
			}
			else {
				// Partner'�n listesini al
				MPI_Recv(partner_arr, local_n, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
				// Listemi partner'a g�nder
				MPI_Send(local_arr, local_n, MPI_INT, partner, 0, MPI_COMM_WORLD);
				// B�y�k say�lar bende kals�n
				for (auto i = local_n - 1, k = local_n - 1, j = local_n - 1; k >= 0; k--)
					if (local_arr[i] < partner_arr[j]) {
						temp_arr[k] = partner_arr[j];
						j--;
					}
					else {
						temp_arr[k] = local_arr[i];
						i--;
					}
			}
			delete[] partner_arr;
			delete[] local_arr;
			local_arr = temp_arr;
		}
		else delete[] temp_arr;
	}

	// S�ral� diziyi rank 0 olan i�lemciye topla
	int * arr = NULL;
	if (rank == 0)	arr = new int[n];
	MPI_Gather(local_arr, local_n, MPI_INT, arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

	// S�ral� diziyi ekrana yazd�r
	if (rank == 0) {
		printf("Siralanmis Dizi: ");
		for (int i = 0; i < n; i++)
			printf("%d ", arr[i]);
		printf("\n");
	}

	// Haf�zay� bo�alt
	delete[] local_arr;
	delete[] arr;

	// MPI'� kapat
	MPI_Finalize();
	return 0;
}

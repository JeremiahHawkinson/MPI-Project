//sieve.cpp
//Jeremiah Hawkinson
//CSE 5250 Final
#include <mpi.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <math.h>

using namespace std::chrono;

int main(int argc, char** argv) {

	auto start = high_resolution_clock::now();

	int commsize;
	int rank;
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &commsize);

	int search_max = 1e6;
	if (argc == 2) {
		//std::cout << argv[1] << std::endl;
		search_max = atoi(argv[1]);
	}

	int sqrt_search_max = (int)sqrt(search_max);

	std::vector<int> numbers(search_max, 0);

	if (rank == 0) {

		for (int i = 0; i < search_max; i++) {
			numbers[i] = i + 1;
		}

		numbers[0] = 0;

		for (int i = 0; i < sqrt_search_max; i++) {
			int current_number = numbers[i];

			for (int j = 1; j < commsize; j++) {
				MPI_Send(&current_number, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
			}

			if (current_number != 0) {
				for (int i = 1; i < commsize; i++) {
					int number_of_indices;
					MPI_Recv(&number_of_indices, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					std::vector<int> temp_vector(number_of_indices, 0);

					MPI_Recv(&temp_vector[0], number_of_indices, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

					for (int j = 0; j < number_of_indices; j++) {
						int index = temp_vector[j];
						numbers[index] = 0;
					}
				}
			}
		}

		auto stop = high_resolution_clock::now();
		auto exec_time = duration_cast<microseconds>(stop - start);

		std::cout << " Found prime numbers up to " << search_max << " in " << exec_time.count() << " microseconds using " << commsize << " processes.\n";

		std::ofstream file;
		file.open("output.txt", std::ios::out | std::ios::app);
		file << "Found prime numbers up to " << search_max << " in " << exec_time.count() << " microseconds using " << commsize << " processes.\n";
		file.close();


	}
	else {
		for (int i = 0; i < sqrt_search_max; i++) {
			int received_number;
			MPI_Recv(&received_number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


			if (received_number != 0) {

				int current_prime = received_number;
				int starting_index = received_number - 1 + rank * current_prime;
				int stride = current_prime * (commsize - 1);

				std::vector<int> indices_of_nonprimes;

				for (int i = starting_index; i < search_max; i += stride) {
					indices_of_nonprimes.push_back(i);
				}
				int number_of_indices = indices_of_nonprimes.size();
				MPI_Send(&number_of_indices, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
				MPI_Send(&indices_of_nonprimes[0], indices_of_nonprimes.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
			}
		}
	}
	MPI_Finalize();
	return 0;
}
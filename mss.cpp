/*  Project: merge-splitting sort, project for PRL course at FIT BUT
    Author: Martin Krajnak
*/

#include <mpi.h>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <climits>

using namespace std;
#define TAG 0

/**
 * merge and sort even int arrays to one
 * @param: *array1, *array2, length of the arrays
 */
void merge_sort_split(int *arr1, int *arr2, int length) {
  // merge received arrays to the common array and sort it
  int tmp[2*length];
  copy(arr1, arr1 + length, tmp);
  copy(arr2, arr2 + length, tmp + length);
  sort(tmp, tmp + (2*length));
  // split the sorted arrays back to the subarrays
  copy(tmp,tmp + length, arr1);
  copy(tmp + length, tmp + (2*length), arr2);
}

int main(int argc, char **argv) {
  int numprocs;               // number of cpus obtained from mpi
  int myid;                   // cpu identifier
  MPI_Status stat;

  int split_factor;           // determines the length of subarray
  int length = atoi(argv[1]); // actual count of numbers to be sorted
  int tmp_length = length;    // make sure that every cpu has even cout of numbers

  MPI_Init(&argc,&argv);                          // MPI init
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // obtain numprocs
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // obtain of of each proc

  if (length % numprocs  == 0)  { // even count of numbers/cpu out of the box
    split_factor = (int)(length/numprocs);
  } else {  // optimization to make sure every cpu has even count of numbers
    split_factor = ((int)(length/numprocs))+1;
    tmp_length = (length + (numprocs - (length%numprocs)));
  }
  int numbers[tmp_length];

  if (myid == 0) {                        // read the file inside 0th proces
    char input[] = "numbers";             // filename
    int number;                           // read number from file
    int invar = 0;                        // count numbers that are read
    fstream fin;
    fin.open(input, ios::in);

    while (fin.good()) {                  // read file
      number = fin.get();
      if (!fin.good()) break;
      cout << number << " ";              // print obtained value on one line
      numbers[invar] = number;
      invar++;
    }
    cout << endl;
    // Fill the rest of the array with maximum values if required
    for (int i = invar; i < tmp_length; i++) {
      numbers[i] = INT_MAX;             // They will always be sorted to the end
    }
    fin.close();
  }
  // precausions to make sure that we won't try to address wrong procs
  int oddlimit = numprocs-1;
  int evenlimit = 2*((numprocs-1)/2);
  // numprocs/2 should be enough cycles to sort array
  int halfcycles = numprocs/2;
  int sub_nums[split_factor]; // initialize subarray for every proc

  // obtain the subarray (sub_nums) for every proc
  MPI_Scatter(numbers, split_factor, MPI_INT, sub_nums, split_factor, MPI_INT, 0, MPI_COMM_WORLD);

  // Sort Start
  for (int i = 1; i < halfcycles+1; i++) {

      if ((!(myid%2) || myid==0) && (myid<oddlimit)) { // Odd cpu
        int recv[split_factor];
        MPI_Recv(&recv, split_factor, MPI_INT, myid+1, TAG, MPI_COMM_WORLD, &stat);
        // merge it, sort it, split it back
        merge_sort_split(sub_nums, recv, split_factor);
        // send second half to neighbor
        MPI_Send(&recv, split_factor, MPI_INT, myid+1, TAG, MPI_COMM_WORLD);

      } else if(myid<=oddlimit){
        // Send your number to the neigbor
        MPI_Send(&sub_nums, split_factor, MPI_INT, myid-1, TAG, MPI_COMM_WORLD);
        // Wait for the sorted half from him
        MPI_Recv(&sub_nums, split_factor, MPI_INT, myid-1, TAG, MPI_COMM_WORLD, &stat);
      }

      if ((myid%2) && (myid<evenlimit)) { // Even cpus
        int recv[split_factor];
        MPI_Recv(&recv, split_factor, MPI_INT, myid+1, TAG, MPI_COMM_WORLD, &stat);
        // merge it, sort it, split it back
        merge_sort_split(sub_nums, recv, split_factor);
        // send second half to neighbor
        MPI_Send(&recv, split_factor, MPI_INT, myid+1, TAG, MPI_COMM_WORLD);

      } else if(myid<=evenlimit && myid!=0){
        // Send your number to the neigbor
        MPI_Send(&sub_nums, split_factor, MPI_INT, myid-1, TAG, MPI_COMM_WORLD);
        // Wait for the sorted half from him
        MPI_Recv(&sub_nums, split_factor, MPI_INT, myid-1, TAG, MPI_COMM_WORLD, &stat);
      }
  } // End sort

  int res[tmp_length];
  // Gather the sorted values from each procs subarray to the new array
  MPI_Gather(sub_nums, split_factor, MPI_INT, res, split_factor, MPI_INT, 0, MPI_COMM_WORLD);

  if (myid == 0) {  // print the sorted array one per line
    for (size_t i = 0; i < length; i++) {
      cout << res[i] << endl;
    }
  }
  MPI_Finalize();
  return 0;
 }

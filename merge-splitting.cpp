/*  Project: merge-splitting sort
    Author: Martin Krajnak
*/

#include <mpi.h>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>

using namespace std;
#define TAG 0

int main(int argc, char **argv) {
  int numprocs;               // number of cpus obtained from mpi
  int myid;                   // cpu identifier
  MPI_Status stat;
  int split_factor;           // determines the length of subarray
  int length = atoi(argv[1]); // actual cout of numbers to be sorted
  int numbers[length];        // array in which numbers will be stored

  MPI_Init(&argc,&argv);                          // inicializace MPI
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // zjistíme, kolik procesů běží
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // zjistíme id svého procesu

  if (length % numprocs  == 0)  {
    split_factor = (int)(length/numprocs);
  } else {
    split_factor = ((int)(length/numprocs))+1;
  }

  if (myid == 0) {                        // read the file inside 0 proces
    char input[]= "numbers";
    int number;
    int invar= 0;
    fstream fin;
    fin.open(input, ios::in);

    while (fin.good()) {
      number = fin.get();
      if (!fin.good()) break;
      //cout<<invar<<":"<<number<<endl;
      numbers[invar] = number;
      invar++;
    }
    fin.close();
  }

  int sub_nums[split_factor];
  int oddlimit = numprocs-1;
  int evenlimit = 2*((numprocs-1)/2);
  int halfcycles = numprocs/2;
  int cycles = 0;
  // Send the values to the cpus
  MPI_Scatter(numbers, split_factor, MPI_INT, sub_nums, split_factor, MPI_INT, 0, MPI_COMM_WORLD);
  // Sort Start
  for (int i = 0; i < halfcycles; i++) {
      cycles++;

      if ((!(myid%2) || myid==0) && (myid<oddlimit)) { // Odd cpu
        int recv[split_factor];
        MPI_Recv(&recv, split_factor, MPI_INT, myid+1, TAG, MPI_COMM_WORLD, &stat);

        // merge received value to the common array and sort it
        int tmp[2*split_factor];
        copy(sub_nums, sub_nums + split_factor, tmp);
        copy(recv, recv + split_factor, tmp + split_factor);
        sort(tmp,tmp+(2*split_factor));
        // split array, keep first half, send second half to neighbor
        copy(tmp,tmp + split_factor, sub_nums);
        copy(tmp + split_factor, tmp + (2*split_factor), recv);
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

        // merge received value to the common array and sort it
        int tmp[2*split_factor];
        copy(sub_nums, sub_nums + split_factor, tmp);
        copy(recv, recv + split_factor, tmp + split_factor);
        sort(tmp,tmp+(2*split_factor));
        // split array, keep first half, send second half to neighbor
        copy(tmp,tmp + split_factor, sub_nums);
        copy(tmp + split_factor, tmp + (2*split_factor), recv);
        MPI_Send(&recv, split_factor, MPI_INT, myid+1, TAG, MPI_COMM_WORLD);

      } else if(myid<=evenlimit && myid!=0){
        // Send your number to the neigbor
        MPI_Send(&sub_nums, split_factor, MPI_INT, myid-1, TAG, MPI_COMM_WORLD);
        // Wait for the sorted half from him
        MPI_Recv(&sub_nums, split_factor, MPI_INT, myid-1, TAG, MPI_COMM_WORLD, &stat);
      }
  }
  int res[length];
  // Gather the values from the cpus to the new array
  MPI_Gather(sub_nums, split_factor, MPI_INT, res, split_factor, MPI_INT, 0, MPI_COMM_WORLD);

  if (myid == 0) {
    for (size_t i = 0; i < length; i++) {
      cout << res[i] << endl;
    }
  }

  MPI_Finalize();
  return 0;
 }

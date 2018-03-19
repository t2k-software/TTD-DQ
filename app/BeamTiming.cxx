#define BEAMTIMING_CXX

#include "BeamTimingData.hxx"
#include <iostream>
#include <cstdlib>

void Usage() {
    std::cout << "BeamTimingData.exe -f [files.list] > [beamtiming.out]" << std::endl;
    exit(1);
}

int main(int argc, char* argv[]) {
  
  // Process the options.
  for (;;) {
    int c = getopt(argc, argv, "f:");
    //if (c<0) break;
    switch (c) {
      case 'f': {
        BeamTimingData run(optarg);
        break; 
      }
      default : {
        Usage();
      }
    }
  } // Closes process options for loop  
  return 0;
}

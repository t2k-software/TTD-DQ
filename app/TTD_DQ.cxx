#define TTD_DQ_CXX

#include "TSystem.h"
#include "TString.h"

#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

void Usage();
void MustHave(std::string envVar);
void TestEnvVariables();
void RunScripts(const Int_t &weeksAgo);

void Usage()
{
    std::cout << "TTD_DQ.exe [OPTIONS]" << std::endl;
    std::cout << std::endl;
    std::cout << "OPTIONS" <<std::endl;
    std::cout << "  -h       : This help message " <<std::endl;
    std::cout << "  -w <num> : The <num> of weeks to analyze. Default -1" <<std::endl;
    std::cout << "The following environment variables must be set" << std::endl;
    std::cout << "CMTROOT, ROOTSYS, and SOFFTASKSROOT " << std::endl;
    std::cout << "  and pdflatex must be in your PATH!" << std::endl;
    exit(1);
}

void MustHave(std::string envVar)
{
   if(!gSystem->Getenv(envVar.c_str()))
   {
       std::cout << "ERROR: " << envVar.c_str() << " is not set!" << std::endl;
       Usage();
   }
}

void TestEnvVariables()
{
    MustHave("CMTROOT");
    MustHave("ROOTSYS");
    MustHave("SOFFTASKSROOT");
    TString pdflatex("pdflatex");
    if(!gSystem->FindFile(gSystem->Getenv("PATH"),pdflatex))
    {
       std::cout << "ERROR: pdfLatex NOT found!" << std::endl;
       exit(1);
    }
}

void RunScripts(const Int_t& weeksAgo)
{
    gSystem->cd("scripts");
    char buffer[100];
    sprintf(buffer,"./RunAllTTDDQ.sh -w %d",weeksAgo);
    gSystem->Exec(buffer);
    gSystem->cd("../");
}

int main(int argc, char* argv[]) {

  Int_t weeksAgo = -1;
  // Process the options.
  for (;;) {
    int c = getopt(argc, argv, "hw:");
    //if (c<0) break;
    switch (c) {
      case 'h':
      {
        Usage();
	    break;
      }
      case 'w':
      {
        std::istringstream in(optarg);
        in >> weeksAgo;
        break;
      }
      default :
      {
	    TestEnvVariables();
	    RunScripts(weeksAgo);
      }
	  break;
    }
  }
  // Closes process options for loop
  return 0;
}

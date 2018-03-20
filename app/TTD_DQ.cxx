#define TTD_DQ_CXX

#include "TSystem.h"
#include "TString.h"

#include <string>
#include <iostream>
#include <cstdlib>

void Usage();
void MustHave(std::string envVar);
void TestEnvVariables();
void RunScripts();

void Usage() 
{
    std::cout << "TTD_DQ.exe [-h]" << std::endl;
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

void RunScripts()
{
    gSystem->cd("scripts");
    gSystem->Exec("./RunAllTTDDQ.sh");
    gSystem->cd("../");
}

int main(int argc, char* argv[]) {
  
  // Process the options.
  for (;;) {
    int c = getopt(argc, argv, "h");
    //if (c<0) break;
    switch (c) {
      case 'h': {
        Usage();
	break;
      }
      default : {
	TestEnvVariables();
	RunScripts();
	break;
      }
    }
  } // Closes process options for loop  
  return 0;
}

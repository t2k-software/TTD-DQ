#define TTD_DQ_CXX

#include "TSystem.h"
#include "TString.h"

#include <string>
#include <iostream>
#include <cstdlib>

void Usage();
void MustHave(std::string envVar);
void MustHave(const char* searchPath, TString program);
void TestEnvVariables();
void RunScripts();

void Usage() 
{
    std::cout << "TTD_DQ.exe [-h]" << std::endl;
    std::cout << "The following environment variables must be set" << std::endl;
    std::cout << "CMTROOT, ROOTSYS, and SOFFTASKSROOT " << std::endl;
    std::cout << "  with iget and pdflatex must be in your PATH!" << std::endl;
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

void MustHave(const char* searchPath, TString program)
{
  if(!gSystem->FindFile(searchPath, program))
  {
       std::cout << "ERROR: " << program.Data() << " NOT found in " << searchPath << std::endl;
       exit(1);
  }
}

void TestEnvVariables() 
{
    const std::string PATH = gSystem->Getenv("PATH");
    MustHave(PATH.c_str(), "pdflatex");
    MustHave(PATH.c_str(), "iget");
    MustHave("CMTROOT");
    MustHave("ROOTSYS");
    MustHave("SOFFTASKSROOT");
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
    if (c<0)
    {
      TestEnvVariables();
      RunScripts();
      break;
    }
    Usage();
    //switch (c) {
    //  case 'h': {
    //    Usage(); 
    //    break;
    //  }
    //  default : {
    //    Usage(); 
    //  }
    //}
  } // Closes process options for loop  
  return 0;
}

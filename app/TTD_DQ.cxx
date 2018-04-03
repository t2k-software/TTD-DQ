#define TTD_DQ_CXX

#include "TSystem.h"
#include "TString.h"

#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

const TString SUPPORTED_SOFFTASKS_VERSION = "v1r51";
void Usage();
void MustHaveSet(std::string envVar);
void MustHave(TString program, std::string searchPath = "PATH");
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
    std::cout << "  with iget and pdflatex must be in your PATH!" << std::endl;
    std::cout << "The supported version of soffTasks is " << SUPPORTED_SOFFTASKS_VERSION.Data() << std::endl;
    exit(1);
}

void MustHaveSet(TString envVar)
{
   if(!gSystem->Getenv(envVar.Data()))
   {
       std::cout << "ERROR: " << envVar.Data() << " is not set!" << std::endl;
       Usage();
   }
}

void MustHave(TString program, std::string searchPath)
{
  if(!gSystem->FindFile(searchPath.c_str(), program))
  {
       std::cout << "ERROR: " << program.Data() << " NOT found in " << searchPath << std::endl;
       exit(1);
  }
}

void Check_soffTasks_Version()
{
    const TString sofftasksroot = gSystem->Getenv("SOFFTASKSROOT");
    if (!sofftasksroot.Contains(SUPPORTED_SOFFTASKS_VERSION) || !sofftasksroot.Contains(Form("%sp0", SUPPORTED_SOFFTASKS_VERSION.Data())))
    {
        TString current_soffTasks_ver = "";
        //get the current softTasks version string
        for (int i =sofftasksroot.Last('/')+1; i < sofftasksroot.Length(); i++)
            current_soffTasks_ver.Append(sofftasksroot[i]);
        std::cout << Form("WARNING: soffTasks %s does NOT match version supported version %s", current_soffTasks_ver.Data(), SUPPORTED_SOFFTASKS_VERSION.Data()) << std::endl;
    }
}

void TestEnvVariables()
{
    TString iget = "iget";
    TString CMTROOT = "CMTROOT";
    TString ROOTSYS = "ROOTSYS";
    TString SOFFTASKSROOT = "SOFFTASKSROOT";
    TString pdflatex = "pdflatex";

    MustHaveSet(CMTROOT);
    MustHaveSet(ROOTSYS);
    MustHaveSet(SOFFTASKSROOT);
    MustHave(iget);
    MustHave(pdflatex);
    Check_soffTasks_Version();
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
	return 0;
      }
      break;
    }
  }
  // Closes process options for loop
  return 0;
}

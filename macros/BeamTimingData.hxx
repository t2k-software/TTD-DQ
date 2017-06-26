#include "TGraphErrors.h"
#include "TFile.h"
#include "TMath.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TText.h"
#include "TLatex.h"
#include "TF1.h"
#include "TLine.h"

#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

// Compile with g++ -o BeamTimingData.exe BeamTimingData.*xx `root-config --cflags --libs`

// Bunch of global variables because I'm a bad person
static const int nBunch = 8;
static const double TimeRange = 100;
static const double minTimeBunchECAL[8] = {2775,3350,3950,4525,5100,5700,6275,6860};
static const double minTimeBunchP0D [8] = {2725,3300,3900,4475,5050,5650,6225,6810};
static const double minTimeBunchSMRD[8] = {2750,3325,3925,4500,5075,5675,6250,6835};
static const int nRMM_ECAL = 12;
static const int nRMM_P0D  = 6;
static const int nRMM_SMRD = 4;

inline bool exists_file(const std::string& name){
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

class BeamTimingData {

public:
  BeamTimingData(std::string);

protected:
  
private:

  void SetGraphStyle()            {SetGraphStyleMean(); SetGraphStyleSigma(); SetGraphStyleSep();};
  void DrawGraphs(int rmm = -999) {DrawGraphMean(rmm);  DrawGraphSigma(rmm);  DrawGraphSep(rmm); };
  
  void SetGraphStyleMean();  // Set the bunch timing draw style
  void SetGraphStyleSigma(); // Set the bunch width draw style
  void SetGraphStyleSep();   // Set the bunch spearation draw style

  void DrawGraphMean(int rmm = -999);  // Draw the bunch timing graph
  void DrawGraphSigma(int rmm = -999); // Draw the bunch width graph
  void DrawGraphSep(int rmm = -999);   // Draw the bunch spearation graph
  
  void FillGraphs(int rmm = -999); // Fill the graphs
  
  double minTimeBunch[nBunch]; // Bunch times

  std::string fDet; // Detector
  int fnRMMs; // Number of RMMs

  int OutOfRange[nBunch]; // Number of out of range data fits
  
  std::vector<TFile*> fRootFiles; // Files read

  TGraphErrors *fHistoMean [nBunch]; // Bunch mean
  TGraphErrors *fHistoSigma[nBunch]; // Bunch width
  TGraphErrors *fHistoSep  [nBunch-1]; // Bunch separation

  double fMinSigma; // Minimum Sigma
  double fMaxSigma; // Maximum Sigma
  double fMinSep;   // Minimum Separation
  double fMaxSep;   // Maximum Separation

};

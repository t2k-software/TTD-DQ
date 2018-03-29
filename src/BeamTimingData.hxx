#ifndef BEAMTIMINGDATA_HXX
#define BEAMTIMINGDATA_HXX

#include "TFile.h"
#include "TGraphErrors.h"

#include <sys/stat.h>
#include <vector>
#include <string>

// Compile with g++ -o BeamTimingData.exe BeamTimingData.*xx `root-config --cflags --libs`

// Bunch of global variables because I'm a bad person
static const Int_t nBunch = 8;
static const Double_t TimeRange = 100;
static const Double_t minTimeBunchECAL[8] = {2775,3350,3950,4525,5100,5700,6275,6860};
static const Double_t minTimeBunchP0D [8] = {2725,3300,3900,4475,5050,5650,6225,6810};
static const Double_t minTimeBunchSMRD[8] = {2750,3325,3925,4500,5075,5675,6250,6835};
static const Int_t nRMM_ECAL = 12;
static const Int_t nRMM_P0D  = 6;
static const Int_t nRMM_SMRD = 4;
static const Int_t bunchSeparationDesign = 581; // Design Bunch Separation 581 ns
static const Int_t bunchWidthDesign = 58; // Design Bunch Width 58 ns
static const Int_t minBunchSigma = 30;
static const Int_t maxBunchSigma = 120;
static const Int_t minBunchSep = 520;
static const Int_t maxBunchSep = 650;


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
  void DrawGraphs(Int_t rmm = -999) {DrawGraphMean(rmm);  DrawGraphSigma(rmm);  DrawGraphSep(rmm); };
  
  void SetGraphStyleMean();  // Set the bunch timing draw style
  void SetGraphStyleSigma(); // Set the bunch width draw style
  void SetGraphStyleSep();   // Set the bunch spearation draw style

  void DrawGraphMean(Int_t rmm = -999);  // Draw the bunch timing graph
  void DrawGraphSigma(Int_t rmm = -999); // Draw the bunch width graph
  void DrawGraphSep(Int_t rmm = -999);   // Draw the bunch spearation graph
  
  void FillGraphs(Int_t rmm = -999); // Fill the graphs
  
  Double_t minTimeBunch[nBunch]; // Bunch times

  std::string fDet; // Detector
  Int_t fnRMMs; // Number of RMMs

  Int_t OutOfRangeMean[nBunch]; // Number of out of range data fits to extract mean;
  Int_t OutOfRangeSigma; // Number of out of range Sigma values;
  Int_t OutOfRangeSep; // Number of out of range Separation values;
  
  std::vector<TFile*> fRootFiles; // Files read

  TGraphErrors *fHistoMean [nBunch]; // Bunch mean
  TGraphErrors *fHistoSigma[nBunch]; // Bunch width
  TGraphErrors *fHistoSep  [nBunch-1]; // Bunch separation

  Double_t fMinSigma; // Minimum Sigma
  Double_t fMaxSigma; // Maximum Sigma
  Double_t fMinSep;   // Minimum Separation
  Double_t fMaxSep;   // Maximum Separation

};

#endif

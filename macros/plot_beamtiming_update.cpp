#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TStyle.h"
#include <iostream>
#include <fstream>
#include "TPad.h"
#include "TVirtualPad.h"
#include "TAxis.h"
#include "TLine.h"
#include "TF1.h"
#include "TText.h"
#include "TGraph.h"
#include "TLine.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TLatex.h"
#include "TMath.h"
#include <algorithm>

// const int nRMM = 12;
const int nBunch = 8;

const int NMAXFILES = 200;
const double TimeRange = 100;
bool verbose = false;
bool printFitResult = true;

inline bool exists_file(const std::string& name){
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

int plot_beamtiming_update(){

  gROOT->Reset();
  gROOT->SetStyle("Plain");
  //gROOT->SetStyle("color2D");
  gStyle->SetHistLineWidth(1);
  gStyle->SetPalette(1);
  gStyle->SetPaperSize(20,26);
  // Turn off some borders
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetDrawBorder(0);
  gStyle->SetCanvasBorderSize(0);
  gStyle->SetFrameBorderSize(0);
  gStyle->SetPadBorderSize(0);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleSize(0.07,"t");
  // Say it in black and white!
  gStyle->SetAxisColor(1, "xyz");
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);
  gStyle->SetFrameLineWidth(0);
  gStyle->SetFrameLineColor(1);
  gStyle->SetHistFillColor(0);
  gStyle->SetHistLineColor(1);
  //gStyle->SetPadColor(1);
  gStyle->SetPadColor(kWhite);
  gStyle->SetStatColor(0);
  gStyle->SetStatTextColor(1);
  gStyle->SetTitleColor(1);
  gStyle->SetTitleTextColor(1);
  gStyle->SetLabelColor(1,"xyz");
  // Show functions in red...
  gStyle->SetFuncColor(2);
  gStyle->SetOptStat(0);

  ////////////////////////////////////////////////////////////////////////
  //  Get the plots of the means from the BeamTiming/Files repository   //
  ////////////////////////////////////////////////////////////////////////
  // double minTimeBunch[8] = {2775,3350,3950,4525,5100,5700,6275,6860}; // ML I don't think these are low enough for the P0D, set these dependent on the detector?
  double minTimeBunchECAL[8] = {2775,3350,3950,4525,5100,5700,6275,6860};
  double minTimeBunchP0D [8] = {2725,3300,3900,4475,5050,5650,6225,6810};
  double minTimeBunchSMRD[8] = {2750,3325,3925,4500,5075,5675,6250,6835};
  
  double minTimeBunch[8];

  int nlines = 0;
  std::string line;
  std::ifstream myfile("files.list");
  TFile *rootfile[200] = {NULL};
  TString fileName("files.list");
  std::ifstream inputFile(fileName.Data(), std::ios::in);
  if (!inputFile){
    std::cerr << "Can not open input file: " << fileName.Data() << std::endl;
    return 0;
  }

  std::string inputString;

  while (inputFile >> inputString){
    std::cout << "Adding file: " << inputString << std::endl;
    if(exists_file(inputString.c_str())){
      rootfile[nlines] = new TFile(inputString.c_str());
      nlines++;
    }else{
      std::cout << "File doesn't exists: " << inputString.c_str() << std::endl;
    }
  }
  
  inputString = rootfile[0]->GetName();
  
  // Read detector information
  int nRMMs = 0;
  std::string det = "";
  
  if (inputString.find("ECAL") != std::string::npos)
    {nRMMs = 12; det = "ECAL"; std::copy(minTimeBunchECAL, minTimeBunchECAL+8, minTimeBunch);}
  else if (inputString.find("P0D") != std::string::npos)
    {nRMMs = 6; det = "P0D";  std::copy(minTimeBunchP0D, minTimeBunchP0D+8, minTimeBunch);}
  else if (inputString.find("SMRD") != std::string::npos)
    {nRMMs = 4; det = "SMRD"; std::copy(minTimeBunchSMRD, minTimeBunchSMRD+8, minTimeBunch);}
  else
    std::cout<<"Detector name not found in input file";
      
  TCanvas* mc1[nRMMs];
  TCanvas* sc1[nRMMs];
  TCanvas* dc1[nRMMs];
  TLatex *textbox[nRMMs][nBunch];

  for(int ican = 0; ican < nRMMs; ican++){
    mc1[ican] = new TCanvas(Form("mc_%d",ican),Form("%s RMM%d Bunch Timing", det.c_str(), ican), 600, 600);
    mc1[ican]->SetTitle("");
    sc1[ican] = new TCanvas(Form("sc_%d",ican),Form("%s RMM%d Bunch Width", det.c_str(), ican), 600, 600);
    sc1[ican]->SetTitle("");
    dc1[ican] = new TCanvas(Form("dc_%d",ican),Form("%s RMM%d Bunch Separation", det.c_str(), ican), 600, 600);
    dc1[ican]->SetTitle("");
  }

  TF1* lineFits[nRMMs][nBunch];
  int OutOfRange[nRMMs][nBunch];
  std::string DrawOpt[nBunch];
  DrawOpt[0] = "AP";
  for(int iBunch = 1; iBunch < nBunch; iBunch++)
  DrawOpt[iBunch] = "AP same";

  TGraphErrors *fileHistoMean;    // define all the graphs for each file, each RMM and each bunch
  TGraphErrors *fileHistoSigma;   // define all the graphs for each file, each RMM and each bunch

  TGraphErrors *plottedHistoMean [nRMMs][nBunch]; // Bunch mean
  TGraphErrors *plottedHistoSigma[nRMMs][nBunch]; // Bunch width
  TGraphErrors *plottedHistoSep  [nRMMs][nBunch-1]; // Bunch separation

  for(int iRMM = 0; iRMM < nRMMs; iRMM++){                    // loop over the RMMs
    mc1[iRMM]->cd();
    std::cout << "RMM : " << iRMM << std::endl;

    double minSigma = 1000.0;
    double maxSigma = 0.0;
    double minSep = 1000.0;
    double maxSep = 0.0;

    // loop over the RMMs
    for(int iBunch = 0; iBunch < nBunch; iBunch++){
      OutOfRange[iRMM][iBunch] = 0;

      plottedHistoMean [iRMM][iBunch] = new TGraphErrors();
      plottedHistoSigma[iRMM][iBunch] = new TGraphErrors();
      if (iBunch) plottedHistoSep[iRMM][iBunch-1] = new TGraphErrors(); // Don't great for first bunch
      
      char *NameMean  = Form("ecal_timingfit_rmm%02d_b%d_Mean",  iRMM, iBunch+1);
      char *NameSigma = Form("ecal_timingfit_rmm%02d_b%d_Sigma", iRMM, iBunch+1);
      TKey *keyMean  = rootfile[0]->FindKey(NameMean );
      TKey *keySigma = rootfile[0]->FindKey(NameSigma);

      if(keyMean == NULL){
        std::cout << "!!MeanHistogram does not exist!!" << std::endl;
        std::cout << "iRMM: " << iRMM << " bunch: " << iBunch+1 << std::endl;
        std::cout << "Tried:" << NameMean << std::endl;
        throw 1;
      }
      if(keySigma == NULL){
        std::cout << "!!SigmaHistogram does not exist!!" << std::endl;
        std::cout << "iRMM: " << iRMM << " bunch:" << iBunch+1 << std::endl;
        std::cout << "Tried: " << NameSigma << std::endl;
        throw 1;
      }
      for(int ilines = 0; ilines < nlines; ilines++){
        fileHistoMean  = NULL;
        fileHistoSigma = NULL;

        fileHistoMean  = (TGraphErrors*)rootfile[ilines]->Get(NameMean );
        fileHistoSigma = (TGraphErrors*)rootfile[ilines]->Get(NameSigma);

        if(!fileHistoMean){
          std::cout << "Mean Histo[ilines = "<< ilines << "][iRMM = " << iRMM
          << "][iBunch = " << iBunch << "] doesnt exist!" << std::endl;
          std::cout << "Tried: " << NameMean << std::endl;

        }else if(!fileHistoSigma){
          std::cout << "Sigma Histo[ilines = "<< ilines << "][iRMM = " << iRMM
          << "][iBunch = " << iBunch << "] doesnt exist!" << std::endl;
          std::cout << "Tried: " << NameSigma << std::endl;

        }else if(!fileHistoSigma && !fileHistoMean){
          std::cout << "Mean Histo[ilines = "<< ilines << "][iRMM = " << iRMM
          << "][iBunch = " << iBunch << "] doesnt exist!" << std::endl;
          std::cout << "Tried: " << NameMean << std::endl;
          std::cout << "Sigma Histo[ilines = "<< ilines << "][iRMM = " << iRMM
          << "][iBunch = " << iBunch << "] doesnt exist!" << std::endl;
          std::cout << "Tried: " << NameSigma << std::endl;

        }else{
          for(int iPoint = 0; iPoint < fileHistoMean->GetN(); iPoint++){
            double x,y,ey,s,es,xp,yp,eyp; // x = date/time, y = mean, ey = mean error, s = sigmna, es = sigma error, xp = previous date/time, yp = previous mean, eyp = previous mean error
            fileHistoMean->GetPoint(iPoint, x, y);
	    ey = fileHistoMean->GetErrorY(iPoint); // ML
	    fileHistoSigma->GetPoint(iPoint, x, s); // ML
	    es = fileHistoSigma->GetErrorY(iPoint); // ML

            if(x > 0 && y > 0){
              bool flag = false;
              if(minTimeBunch[iBunch] < y && y < minTimeBunch[iBunch]+TimeRange) flag = true;
              if(flag){
		// Bunch mean time
                plottedHistoMean[iRMM][iBunch]->SetPoint     (plottedHistoMean[iRMM][iBunch]->GetN(), x, y);
                plottedHistoMean[iRMM][iBunch]->SetPointError(plottedHistoMean[iRMM][iBunch]->GetN()-1, 0, ey); // -1 because a point has just been added in previous line
		// Bunch width (sigma)
		plottedHistoSigma[iRMM][iBunch]->SetPoint     (plottedHistoSigma[iRMM][iBunch]->GetN(), x, 2*s); // Bunch width is twice the sigma
		plottedHistoSigma[iRMM][iBunch]->SetPointError(plottedHistoSigma[iRMM][iBunch]->GetN()-1, 0, 2*es); // Double error too
		if(2*s < minSigma) minSigma = 2*s;
		if(2*s > maxSigma) maxSigma = 2*s;
		// Bunch separation
		if(iBunch){
		  for(int nPoint = 0; nPoint < plottedHistoMean[iRMM][iBunch-1]->GetN(); nPoint++){
		    plottedHistoMean[iRMM][iBunch-1]->GetPoint(nPoint, xp, yp);
		    eyp = plottedHistoMean[iRMM][iBunch-1]->GetErrorY(nPoint);
		    if(xp == x) { // if x value (time) are the same, subtract previous mean from current mean
		      plottedHistoSep[iRMM][iBunch-1]->SetPoint     (plottedHistoSep[iRMM][iBunch-1]->GetN(), x, y-yp);
		      plottedHistoSep[iRMM][iBunch-1]->SetPointError(plottedHistoSep[iRMM][iBunch-1]->GetN()-1, 0, TMath::Sqrt((ey*ey)+(eyp*eyp)) );
		      if(y-yp < minSep) minSep = y-yp;
		      if(y-yp > maxSep) maxSep = y-yp;
		    } // if(xp==x)
		  } // for(int nPoint...
		} // if(iBunch)
	      }else{
                OutOfRange[iRMM][iBunch]++;
              }

              if(verbose && !flag){
                std::cout << "warning [iRMM = " << iRMM << "][iBunch = " << iBunch << "]" << std::endl;
                std::cout << "Out of limit at timestamp: " << x << " (" << y << ")" << std::endl;
              }
            }
          }
        }
      }
    }

    // Drawing bunch mean time //

    TPad *pad[nBunch];

    pad[0] = new TPad("pad[0]","pad[0]",0.01,0.010,0.99,0.211); pad[0]->SetBottomMargin(0.4); pad[0]->SetBorderMode(0); pad[0]->Draw();
    pad[1] = new TPad("pad[1]","pad[1]",0.01,0.211,0.99,0.316); pad[1]->SetBottomMargin(0.);  pad[1]->SetBorderMode(0); pad[1]->Draw();
    pad[2] = new TPad("pad[2]","pad[2]",0.01,0.316,0.99,0.422); pad[2]->SetBottomMargin(0.);  pad[2]->SetBorderMode(0); pad[2]->Draw();
    pad[3] = new TPad("pad[3]","pad[3]",0.01,0.422,0.99,0.527); pad[3]->SetBottomMargin(0.);  pad[3]->SetBorderMode(0); pad[3]->Draw();
    pad[4] = new TPad("pad[4]","pad[4]",0.01,0.527,0.99,0.632); pad[4]->SetBottomMargin(0.);  pad[4]->SetBorderMode(0); pad[4]->Draw();
    pad[5] = new TPad("pad[5]","pad[5]",0.01,0.632,0.99,0.739); pad[5]->SetBottomMargin(0.);  pad[5]->SetBorderMode(0); pad[5]->Draw();
    pad[6] = new TPad("pad[6]","pad[6]",0.01,0.739,0.99,0.844); pad[6]->SetBottomMargin(0.);  pad[6]->SetBorderMode(0); pad[6]->Draw();
    pad[7] = new TPad("pad[7]","pad[7]",0.01,0.844,0.99,0.950); pad[7]->SetBottomMargin(0.);  pad[7]->SetBorderMode(0); pad[7]->Draw();

    // Set the X axis
    plottedHistoMean[iRMM][0]->GetXaxis()->SetTimeDisplay(1);
    plottedHistoMean[iRMM][0]->GetXaxis()->SetTimeFormat("%d/%m");
    plottedHistoMean[iRMM][0]->GetXaxis()->SetTimeOffset(0, "gmt");
    plottedHistoMean[iRMM][0]->GetXaxis()->SetTitle("Date GMT");
    plottedHistoMean[iRMM][0]->GetYaxis()->SetTitle("Bunch Time [ns]");
    plottedHistoMean[iRMM][0]->GetXaxis()->SetTitleSize(0.15);
    plottedHistoMean[iRMM][0]->GetXaxis()->SetLabelSize(0.15);
    plottedHistoMean[iRMM][0]->GetXaxis()->SetTitleOffset(1);
    plottedHistoMean[iRMM][0]->GetXaxis()->SetLabelOffset(0.05);
    double min = plottedHistoMean[iRMM][0]->GetXaxis()->GetXmin();
    double max = plottedHistoMean[iRMM][0]->GetXaxis()->GetXmax();
    std::cout << "min " << min << std::endl;
    std::cout << "max " << max << std::endl;

    /////////////////////////////////////////////////////
    // Set Marker properties
    /////////////////////////////////////////////////////

    for(int iBunch = 0; iBunch < nBunch; iBunch++){        // loop over the bunches
      std::string lineFitName = Form("linefit_%i_%i", iBunch, iRMM);
      lineFits[iRMM][iBunch] = new TF1(lineFitName.c_str(), "[0]", min, max);
      lineFits[iRMM][iBunch]->SetLineWidth(0.35);
      lineFits[iRMM][iBunch]->SetLineColor(kBlack);


      if(!plottedHistoMean[iRMM][iBunch]){
        std::cout << "all the histo are not instantiated, return" << std::endl;
        return 1;
      }else{

        std::cout << "line Fit starting [iRMM = " << iRMM << "][iBunch = " << iBunch << "]" << std::endl;
        plottedHistoMean[iRMM][iBunch]->Fit(lineFits[iRMM][iBunch]);
        std::cout << "line Fit done!! [iRMM = " << iRMM << "][iBunch = " << iBunch << "]" << std::endl;

      }

      if(iBunch != 0){
        // Remove the X axis
        plottedHistoMean[iRMM][iBunch]->GetXaxis()->SetLabelOffset(999);
        plottedHistoMean[iRMM][iBunch]->GetXaxis()->SetLabelSize(0);
        plottedHistoMean[iRMM][iBunch]->GetXaxis()->SetTitle("");
        plottedHistoMean[iRMM][iBunch]->GetXaxis()->SetTickLength(0);
        // Set Y axis
        plottedHistoMean[iRMM][iBunch]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
        plottedHistoMean[iRMM][iBunch]->GetYaxis()->SetLabelSize(0.3);
        plottedHistoMean[iRMM][iBunch]->GetYaxis()->SetTickLength(0.02);
      }else{
        plottedHistoMean[iRMM][0]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
        plottedHistoMean[iRMM][0]->GetYaxis()->SetLabelSize(0.15);
        plottedHistoMean[iRMM][0]->GetYaxis()->SetTickLength(0.035);
      }

      plottedHistoMean[iRMM][iBunch]->SetTitle("");
      plottedHistoMean[iRMM][iBunch]->GetYaxis()->SetTitle("");

      plottedHistoMean[iRMM][iBunch]->SetMarkerStyle(21);
      plottedHistoMean[iRMM][iBunch]->SetMarkerSize(0.35);
      plottedHistoMean[iRMM][iBunch]->SetMarkerColor(iBunch%4+2);
      plottedHistoMean[iRMM][iBunch]->SetLineWidth(0.35);
      plottedHistoMean[iRMM][iBunch]->SetLineColor(iBunch%4+2);

      if(iBunch%4==3){
        plottedHistoMean[iRMM][iBunch]->SetMarkerColor(kGray+1);
        plottedHistoMean[iRMM][iBunch]->SetLineColor  (kGray+1);
      }

      pad[iBunch]->cd();
      plottedHistoMean[iRMM][iBunch]->Draw(DrawOpt[iBunch].c_str());

      plottedHistoMean[iRMM][iBunch]->SetMinimum(minTimeBunch[iBunch]);
      plottedHistoMean[iRMM][iBunch]->SetMaximum(minTimeBunch[iBunch]+TimeRange);
    }
    mc1[iRMM]->cd();


    for(int iBunch = 0; iBunch < nBunch; iBunch++){
      if(lineFits[iRMM][iBunch])
      std::cout << "line Fit [iRMM = " << iRMM << "][iBunch = " << iBunch << "]" << std::endl;

      double parVal = lineFits[iRMM][iBunch]->GetParameter(0);
      double parErr = lineFits[iRMM][iBunch]->GetParError(0);
      double chi2   = lineFits[iRMM][iBunch]->GetChisquare();
      int NDF       = lineFits[iRMM][iBunch]->GetNDF();
      textbox[iRMM][iBunch] = new TLatex(0.2, 0.10+0.11*iBunch, Form("Mean = %4.2f#pm%4.2f, #chi^{2}/NDF = %4.2f / %d (OOR = %d)", parVal,parErr,chi2,NDF, OutOfRange[iRMM][iBunch]));
      textbox[iRMM][iBunch]->SetTextSize(0.025);
    }

    // Draw a nice Y axis
    TPad *p87 = new TPad("p87","p87",0.05,0.825,0.9,0.850);p87->Draw();p87->cd();
    TLine *l87 = new TLine(0.991,0,0.991,0.99); l87->SetLineWidth(0.1); l87->SetLineColor(1); l87->Draw();
    TLine *cut87_1 = new TLine(0.068,0,0.068,0.3); cut87_1->SetLineWidth(0.1); cut87_1->SetLineColor(1); cut87_1->Draw();
    TLine *cut87_2 = new TLine(0.04,0.1,0.096,0.5); cut87_2->SetLineWidth(0.1); cut87_2->SetLineColor(1); cut87_2->Draw();
    TLine *cut87_3 = new TLine(0.04,0.5,0.096,0.9); cut87_3->SetLineWidth(0.1); cut87_3->SetLineColor(1); cut87_3->Draw();
    TLine *cut87_4 = new TLine(0.068,0.7,0.068,1); cut87_4->SetLineWidth(0.1); cut87_4->SetLineColor(1); cut87_4->Draw();

    mc1[iRMM]->cd();
    TPad *p76 = new TPad("p76","p76",0.0,0.710,0.9,0.745);p76->Draw();p76->cd();
    TLine *l76 = new TLine(0.991,0,0.991,0.99); l76->SetLineWidth(0.1); l76->SetLineColor(1); l76->Draw();
    TLine *cut76_1 = new TLine(0.12,0,0.12,0.3); cut76_1->SetLineWidth(0.1); cut76_1->SetLineColor(1); cut76_1->Draw();
    TLine *cut76_2 = new TLine(0.09,0.1,0.146,0.5); cut76_2->SetLineWidth(0.1); cut76_2->SetLineColor(1); cut76_2->Draw();
    TLine *cut76_3 = new TLine(0.09,0.5,0.146,0.9); cut76_3->SetLineWidth(0.1); cut76_3->SetLineColor(1); cut76_3->Draw();
    TLine *cut76_4 = new TLine(0.12,0.7,0.12,1); cut76_4->SetLineWidth(0.1); cut76_4->SetLineColor(1); cut76_4->Draw();

    mc1[iRMM]->cd();
    TPad *p65label_mask = new TPad("p65lm","p65lm",0.,0.605,0.105,0.645);p65label_mask->Draw();
    TPad *p65 = new TPad("p65","p65",0.05,0.613,0.9,0.638);p65->Draw();p65->cd();
    TLine *l65 = new TLine(0.991,0,0.991,0.99); l65->SetLineWidth(0.1); l65->SetLineColor(1); l65->Draw();
    TLine *cut65_1 = new TLine(0.068,0,0.068,0.3); cut65_1->SetLineWidth(0.1); cut65_1->SetLineColor(1); cut65_1->Draw();
    TLine *cut65_2 = new TLine(0.04,0.1,0.096,0.5); cut65_2->SetLineWidth(0.1); cut65_2->SetLineColor(1); cut65_2->Draw();
    TLine *cut65_3 = new TLine(0.04,0.5,0.096,0.9); cut65_3->SetLineWidth(0.1); cut65_3->SetLineColor(1); cut65_3->Draw();
    TLine *cut65_4 = new TLine(0.068,0.7,0.068,1); cut65_4->SetLineWidth(0.1); cut65_4->SetLineColor(1); cut65_4->Draw();

    mc1[iRMM]->cd();
    TPad *p54label_mask = new TPad("p54lm","p54lm",0.,0.505,0.105,0.55);p54label_mask->Draw();
    TPad *p54 = new TPad("p54","p54",0.05,0.508,0.9,0.533);p54->Draw();p54->cd();
    TLine *l54 = new TLine(0.991,0,0.991,1); l54->SetLineWidth(0.1); l54->SetLineColor(1); l54->Draw();
    TLine *cut54_1 = new TLine(0.068,0,0.068,0.3);  cut54_1->SetLineWidth(0.1); cut54_1->SetLineColor(1); cut54_1->Draw();
    TLine *cut54_2 = new TLine(0.04,0.1,0.096,0.5); cut54_2->SetLineWidth(0.1); cut54_2->SetLineColor(1); cut54_2->Draw();
    TLine *cut54_3 = new TLine(0.04,0.5,0.096,0.9); cut54_3->SetLineWidth(0.1); cut54_3->SetLineColor(1); cut54_3->Draw();
    TLine *cut54_4 = new TLine(0.068,0.7,0.068,1);  cut54_4->SetLineWidth(0.1); cut54_4->SetLineColor(1); cut54_4->Draw();

    mc1[iRMM]->cd();
    TPad *p43label_mask = new TPad("p43lm","p43lm",0.,0.39,0.105,0.43);p43label_mask->Draw();
    TPad *p43 = new TPad("p43","p43",0.05,0.403,0.9,0.428);p43->Draw();p43->cd();
    TLine *l43 = new TLine(0.991,0,0.991,0.99);l43->SetLineWidth(0.1);l43->SetLineColor(1);l43->Draw();
    TLine *cut43_1 = new TLine(0.068,0,0.068,0.3);  cut43_1->SetLineWidth(0.1); cut43_1->SetLineColor(1); cut43_1->Draw();
    TLine *cut43_2 = new TLine(0.04,0.1,0.096,0.5); cut43_2->SetLineWidth(0.1); cut43_2->SetLineColor(1); cut43_2->Draw();
    TLine *cut43_3 = new TLine(0.04,0.5,0.096,0.9); cut43_3->SetLineWidth(0.1); cut43_3->SetLineColor(1); cut43_3->Draw();
    TLine *cut43_4 = new TLine(0.068,0.7,0.068,1);  cut43_4->SetLineWidth(0.1); cut43_4->SetLineColor(1); cut43_4->Draw();

    mc1[iRMM]->cd();
    TPad *p32label_mask = new TPad("p32lm","p32lm",0.0,0.280,0.105,0.35);p32label_mask->Draw();
    TPad *p32 = new TPad("p32","p32",0.05,0.297,0.9,0.322);p32->Draw();p32->cd();
    TLine *l32 = new TLine(0.991,0,0.991,0.99); l32->SetLineWidth(0.1); l32->SetLineColor(1); l32->Draw();
    TLine *cut32_1 = new TLine(0.068,0,0.068,0.3);  cut32_1->SetLineWidth(0.1); cut32_1->SetLineColor(1); cut32_1->Draw();
    TLine *cut32_2 = new TLine(0.04,0.1,0.096,0.5); cut32_2->SetLineWidth(0.1); cut32_2->SetLineColor(1); cut32_2->Draw();
    TLine *cut32_3 = new TLine(0.04,0.5,0.096,0.9); cut32_3->SetLineWidth(0.1); cut32_3->SetLineColor(1); cut32_3->Draw();
    TLine *cut32_4 = new TLine(0.068,0.7,0.068,1);  cut32_4->SetLineWidth(0.1); cut32_4->SetLineColor(1); cut32_4->Draw();


    mc1[iRMM]->cd();
    TPad *p21label_mask = new TPad("p21lm","p21lm",0.0,0.18,0.105,0.225);p21label_mask->Draw();
    TPad *p21 = new TPad("p21","p21",0.05,0.185,0.9,0.217);p21->Draw();p21->cd();
    TLine *l21 = new TLine(0.991,0,0.991,0.99); l21->SetLineWidth(0.1); l21->SetLineColor(1); l21->Draw();
    TLine *cut21_1 = new TLine(0.068,0,0.068,0.3);  cut21_1->SetLineWidth(0.1); cut21_1->SetLineColor(1); cut21_1->Draw();
    TLine *cut21_2 = new TLine(0.04,0.1,0.096,0.5); cut21_2->SetLineWidth(0.1); cut21_2->SetLineColor(1); cut21_2->Draw();
    TLine *cut21_3 = new TLine(0.04,0.5,0.096,0.9); cut21_3->SetLineWidth(0.1); cut21_3->SetLineColor(1); cut21_3->Draw();
    TLine *cut21_4 = new TLine(0.068,0.7,0.068,1);  cut21_4->SetLineWidth(0.1); cut21_4->SetLineColor(1); cut21_4->Draw();
    mc1[iRMM]->cd();

    if(printFitResult)
    for(int iBunch = 0; iBunch < nBunch; iBunch++){
      textbox[iRMM][iBunch]->Draw();
    }
    // Print the title
    TText *titlebox = new TText(0.01,0.96, Form("%s Bunch Timing RMM%02d [ns]", det.c_str(), iRMM));
    titlebox->SetTextSize(0.055);
    titlebox->Draw();

    mc1[iRMM]->SaveAs(Form("%s_bunchtiming_weekly%d.png", det.c_str(), iRMM));


    // Draw bunch width (sigma)
    sc1[iRMM]->cd();

    plottedHistoSigma[iRMM][0]->GetXaxis()->SetTimeDisplay(1);
    plottedHistoSigma[iRMM][0]->GetXaxis()->SetTimeFormat("%d/%m");
    plottedHistoSigma[iRMM][0]->GetXaxis()->SetTimeOffset(0, "gmt");
    plottedHistoSigma[iRMM][0]->GetXaxis()->SetTitle("Date GMT");
    plottedHistoSigma[iRMM][0]->GetYaxis()->SetTitle("Bunch Time [ns]");
    plottedHistoSigma[iRMM][0]->GetXaxis()->SetTitleSize(0.03); // .15
    plottedHistoSigma[iRMM][0]->GetXaxis()->SetLabelSize(0.03); // .15
    plottedHistoSigma[iRMM][0]->GetXaxis()->SetTitleOffset(1);
    //    plottedHistoSigma[iRMM][0]->GetXaxis()->SetLabelOffset(0.05);
    
    plottedHistoSigma[iRMM][0]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
    plottedHistoSigma[iRMM][0]->GetYaxis()->SetLabelSize(0.03); // .15
    plottedHistoSigma[iRMM][0]->GetYaxis()->SetTickLength(0.035);


    // loop bunches
    for(int iBunch = 0; iBunch < nBunch; iBunch++){

      if(iBunch != 0){
        // Remove the X axis
        plottedHistoSigma[iRMM][iBunch]->GetXaxis()->SetLabelOffset(999);
        plottedHistoSigma[iRMM][iBunch]->GetXaxis()->SetLabelSize(0);
        plottedHistoSigma[iRMM][iBunch]->GetXaxis()->SetTitle("");
        plottedHistoSigma[iRMM][iBunch]->GetXaxis()->SetTickLength(0);
        // Set Y axis
        plottedHistoSigma[iRMM][iBunch]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
        plottedHistoSigma[iRMM][iBunch]->GetYaxis()->SetLabelSize(0.3);
        plottedHistoSigma[iRMM][iBunch]->GetYaxis()->SetTickLength(0.02);
      }else{
        plottedHistoSigma[iRMM][0]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
        plottedHistoSigma[iRMM][0]->GetYaxis()->SetLabelSize(0.03); // .15
        plottedHistoSigma[iRMM][0]->GetYaxis()->SetTickLength(0.035);
      }

      plottedHistoSigma[iRMM][iBunch]->SetTitle("");
      plottedHistoSigma[iRMM][iBunch]->GetYaxis()->SetTitle("");

      plottedHistoSigma[iRMM][iBunch]->SetMarkerStyle(21);
      plottedHistoSigma[iRMM][iBunch]->SetMarkerSize(1.0); // .35
      // plottedHistoSigma[iRMM][iBunch]->SetMarkerColor(iBunch%4+2);
      plottedHistoSigma[iRMM][iBunch]->SetMarkerColor( (iBunch<4 ? iBunch+1 : iBunch+2) );
      plottedHistoSigma[iRMM][iBunch]->SetLineWidth(0.35);
      // plottedHistoSigma[iRMM][iBunch]->SetLineColor(iBunch%4+2);
      plottedHistoSigma[iRMM][iBunch]->SetLineColor( (iBunch<4 ? iBunch+1 : iBunch+2) );

      plottedHistoSigma[iRMM][iBunch]->SetMinimum(0.9*minSigma);
      plottedHistoSigma[iRMM][iBunch]->SetMaximum(1.1*maxSigma);
      
      // if(iBunch%4==3){
      //   plottedHistoSigma[iRMM][iBunch]->SetMarkerColor(kGray+1);
      //   plottedHistoSigma[iRMM][iBunch]->SetLineColor  (kGray+1);
      // }

      if(iBunch==0) plottedHistoSigma[iRMM][iBunch]->Draw(DrawOpt[iBunch].c_str());
      else plottedHistoSigma[iRMM][iBunch]->Draw("P same");
    }
    sc1[iRMM]->cd();

    // Print the title
    TText *titleboxsigma = new TText(0.01,0.96, Form("%s Bunch Width RMM%02d [ns]", det.c_str(), iRMM)); // 0.01 0.96
    titleboxsigma->SetNDC();
    titleboxsigma->SetTextSize(0.055); // 0.055
    titleboxsigma->Draw();
    
    sc1[iRMM]->SaveAs(Form("%s_bunchwidth_weekly%d.png", det.c_str(), iRMM));

    // Draw bunch separation
    dc1[iRMM]->cd();

    plottedHistoSep[iRMM][0]->GetXaxis()->SetTimeDisplay(1);
    plottedHistoSep[iRMM][0]->GetXaxis()->SetTimeFormat("%d/%m");
    plottedHistoSep[iRMM][0]->GetXaxis()->SetTimeOffset(0, "gmt");
    plottedHistoSep[iRMM][0]->GetXaxis()->SetTitle("Date GMT");
    plottedHistoSep[iRMM][0]->GetYaxis()->SetTitle("Bunch Time [ns]");
    plottedHistoSep[iRMM][0]->GetXaxis()->SetTitleSize(0.03); // .15
    plottedHistoSep[iRMM][0]->GetXaxis()->SetLabelSize(0.03); // .15
    plottedHistoSep[iRMM][0]->GetXaxis()->SetTitleOffset(1);
    //    plottedHistoSep[iRMM][0]->GetXaxis()->SetLabelOffset(0.05);
    
    plottedHistoSep[iRMM][0]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
    plottedHistoSep[iRMM][0]->GetYaxis()->SetLabelSize(0.03); // .15
    plottedHistoSep[iRMM][0]->GetYaxis()->SetTickLength(0.035);


    // loop bunches
    for(int iBunch = 0; iBunch < nBunch-1; iBunch++){

      if(iBunch != 0){
        // Remove the X axis
        plottedHistoSep[iRMM][iBunch]->GetXaxis()->SetLabelOffset(999);
        plottedHistoSep[iRMM][iBunch]->GetXaxis()->SetLabelSize(0);
        plottedHistoSep[iRMM][iBunch]->GetXaxis()->SetTitle("");
        plottedHistoSep[iRMM][iBunch]->GetXaxis()->SetTickLength(0);
        // Set Y axis
        plottedHistoSep[iRMM][iBunch]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
        plottedHistoSep[iRMM][iBunch]->GetYaxis()->SetLabelSize(0.3);
        plottedHistoSep[iRMM][iBunch]->GetYaxis()->SetTickLength(0.02);
      }else{
        plottedHistoSep[iRMM][0]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
        plottedHistoSep[iRMM][0]->GetYaxis()->SetLabelSize(0.03); // .15
        plottedHistoSep[iRMM][0]->GetYaxis()->SetTickLength(0.035);
      }

      plottedHistoSep[iRMM][iBunch]->SetTitle("");
      plottedHistoSep[iRMM][iBunch]->GetYaxis()->SetTitle("");

      plottedHistoSep[iRMM][iBunch]->SetMarkerStyle(21);
      plottedHistoSep[iRMM][iBunch]->SetMarkerSize(1.0); // .35
      plottedHistoSep[iRMM][iBunch]->SetMarkerColor( (iBunch<3 ? iBunch+2 : iBunch+3) );
      plottedHistoSep[iRMM][iBunch]->SetLineWidth(0.35);
      plottedHistoSep[iRMM][iBunch]->SetLineColor( (iBunch<3 ? iBunch+2 : iBunch+3) );

      plottedHistoSep[iRMM][iBunch]->SetMinimum(0.9*minSep);
      plottedHistoSep[iRMM][iBunch]->SetMaximum(1.1*maxSep);
      
      if(iBunch==0) plottedHistoSep[iRMM][iBunch]->Draw(DrawOpt[iBunch].c_str());
      else plottedHistoSep[iRMM][iBunch]->Draw("P same");
    }
    dc1[iRMM]->cd();

    // Print the title
    TText *titleboxsep = new TText(0.01,0.96, Form("%s Bunch Separation RMM%02d [ns]", det.c_str(), iRMM)); // 0.01 0.96
    titleboxsep->SetNDC();
    titleboxsep->SetTextSize(0.055); // 0.055
    titleboxsep->Draw();
    
    dc1[iRMM]->SaveAs(Form("%s_bunchseparation_weekly%d.png", det.c_str(), iRMM));
    
  }

  // TCanvas *c = new TCanvas();
  // for(int iRMM = 0; iRMM < nRMMs; iRMM++){                    // loop over the RMMs
  //   for(int iBunch = 0; iBunch < nBunch; iBunch++){
  //     if(plottedHistoMean[iRMM][iBunch]){
  // 	plottedHistoMean[iRMM][iBunch]->Draw("AP");
  // 	c->SaveAs(Form("crap%i_%i.png", iRMM, iBunch));
  //     }
  //     //delete c;
  //   }
  // }
  return 1;

}


int main(void){
  plot_beamtiming_update();
  return 0;
}

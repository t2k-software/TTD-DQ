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

const int nRMM = 12;
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
  double minTimeBunch[8] = {2775,3350,3950,4525,5100,5700,6275,6860};

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


  TCanvas* mc1[nRMM];
  TLatex *textbox[nRMM][nBunch];

  for(int ican = 0; ican < nRMM; ican++){
    mc1[ican] = new TCanvas(Form("mc_%d",ican),Form("ECAL RMM%d BunchTiming", ican), 600, 600);
    mc1[ican]->SetTitle("");
  }

  TF1* lineFits[nRMM][nBunch];
  int OutOfRange[nRMM][nBunch];
  std::string DrawOpt[nBunch];
  DrawOpt[0] = "AP";
  for(int iBunch = 1; iBunch < nBunch; iBunch++)
  DrawOpt[iBunch] = "AP same";

  TGraphErrors *fileHistoMean;    // define all the graphs for each file, each RMM and each bunch
  TGraphErrors *fileHistoSigma;   // define all the graphs for each file, each RMM and each bunch

  TGraphErrors *plottedHisto[nRMM][nBunch];

  for(int iRMM = 0; iRMM < nRMM; iRMM++){                    // loop over the RMMs
    mc1[iRMM]->cd();
    std::cout << "RMM : " << iRMM << std::endl;

    // loop over the RMMs
    for(int iBunch = 0; iBunch < nBunch; iBunch++){
      OutOfRange[iRMM][iBunch] = 0;

      plottedHisto[iRMM][iBunch] = new TGraphErrors();

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
            double x,y,ey;
            fileHistoMean->GetPoint(iPoint, x, y);
            ey = fileHistoMean->GetErrorY(iPoint);

            if(x > 0 && y > 0){
              bool flag = false;
              if(minTimeBunch[iBunch] < y && y < minTimeBunch[iBunch]+TimeRange) flag = true;
              if(flag){
                plottedHisto[iRMM][iBunch]->SetPoint     (plottedHisto[iRMM][iBunch]->GetN(), x, y);
                plottedHisto[iRMM][iBunch]->SetPointError(plottedHisto[iRMM][iBunch]->GetN()-1, 0, ey);
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
    plottedHisto[iRMM][0]->GetXaxis()->SetTimeDisplay(1);
    plottedHisto[iRMM][0]->GetXaxis()->SetTimeFormat("%d/%m");
    plottedHisto[iRMM][0]->GetXaxis()->SetTimeOffset(0, "gmt");
    plottedHisto[iRMM][0]->GetXaxis()->SetTitle("Date GMT");
    plottedHisto[iRMM][0]->GetYaxis()->SetTitle("Bunch Time [ns]");
    plottedHisto[iRMM][0]->GetXaxis()->SetTitleSize(0.15);
    plottedHisto[iRMM][0]->GetXaxis()->SetLabelSize(0.15);
    plottedHisto[iRMM][0]->GetXaxis()->SetTitleOffset(1);
    plottedHisto[iRMM][0]->GetXaxis()->SetLabelOffset(0.05);
    double min = plottedHisto[iRMM][0]->GetXaxis()->GetXmin();
    double max = plottedHisto[iRMM][0]->GetXaxis()->GetXmax();
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


      if(!plottedHisto[iRMM][iBunch]){
        std::cout << "all the histo are not instantiated, return" << std::endl;
        return 1;
      }else{

        std::cout << "line Fit starting [iRMM = " << iRMM << "][iBunch = " << iBunch << "]" << std::endl;
        plottedHisto[iRMM][iBunch]->Fit(lineFits[iRMM][iBunch]);
        std::cout << "line Fit done!! [iRMM = " << iRMM << "][iBunch = " << iBunch << "]" << std::endl;

      }

      if(iBunch != 0){
        // Remove the X axis
        plottedHisto[iRMM][iBunch]->GetXaxis()->SetLabelOffset(999);
        plottedHisto[iRMM][iBunch]->GetXaxis()->SetLabelSize(0);
        plottedHisto[iRMM][iBunch]->GetXaxis()->SetTitle("");
        plottedHisto[iRMM][iBunch]->GetXaxis()->SetTickLength(0);
        // Set Y axis
        plottedHisto[iRMM][iBunch]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
        plottedHisto[iRMM][iBunch]->GetYaxis()->SetLabelSize(0.3);
        plottedHisto[iRMM][iBunch]->GetYaxis()->SetTickLength(0.02);
      }else{
        plottedHisto[iRMM][0]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
        plottedHisto[iRMM][0]->GetYaxis()->SetLabelSize(0.15);
        plottedHisto[iRMM][0]->GetYaxis()->SetTickLength(0.035);
      }

      plottedHisto[iRMM][iBunch]->SetTitle("");
      plottedHisto[iRMM][iBunch]->GetYaxis()->SetTitle("");

      plottedHisto[iRMM][iBunch]->SetMarkerStyle(21);
      plottedHisto[iRMM][iBunch]->SetMarkerSize(0.35);
      plottedHisto[iRMM][iBunch]->SetMarkerColor(iBunch%4+2);
      plottedHisto[iRMM][iBunch]->SetLineWidth(0.35);
      plottedHisto[iRMM][iBunch]->SetLineColor(iBunch%4+2);

      if(iBunch%4==3){
        plottedHisto[iRMM][iBunch]->SetMarkerColor(kGray+1);
        plottedHisto[iRMM][iBunch]->SetLineColor  (kGray+1);
      }

      pad[iBunch]->cd();
      plottedHisto[iRMM][iBunch]->Draw(DrawOpt[iBunch].c_str());

      plottedHisto[iRMM][iBunch]->SetMinimum(minTimeBunch[iBunch]);
      plottedHisto[iRMM][iBunch]->SetMaximum(minTimeBunch[iBunch]+TimeRange);
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
    TText *titlebox = new TText(0.01,0.96, Form("Bunch Timing RMM%02d [ns]",iRMM));
    titlebox->SetTextSize(0.055);
    titlebox->Draw();

    mc1[iRMM]->SaveAs(Form("bunchtiming_weekly%d.png", iRMM));
  }

  // TCanvas *c = new TCanvas();
  // for(int iRMM = 0; iRMM < nRMM; iRMM++){                    // loop over the RMMs
  //   for(int iBunch = 0; iBunch < nBunch; iBunch++){
  //     if(plottedHisto[iRMM][iBunch]){
  // 	plottedHisto[iRMM][iBunch]->Draw("AP");
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

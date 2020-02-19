#define BEAMTIMINGDATA_CXX

#include "BeamTimingData.hxx"

#include "TMath.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TText.h"
#include "TLatex.h"
#include "TF1.h"
#include "TLine.h"

#include <algorithm>
#include <iostream>
#include <fstream>


BeamTimingData::BeamTimingData(std::string fileList) {
  
  std::ifstream inputFile(fileList.c_str(), std::ios::in);
  if (!inputFile){
    std::cerr << "Can not open input file: " << fileList << std::endl;
    return;
  }

  std::string inputString;
  
  while (inputFile >> inputString){
    std::cout << "Adding file: " << inputString << std::endl;
    if(exists_file(inputString.c_str()))
      fRootFiles.push_back(new TFile(inputString.c_str()));
    else
      std::cout << "File doesn't exists: " << inputString.c_str() << std::endl;
  } // while

  if (fRootFiles.empty()) {
    std::cerr << "No valid root files found" << std::endl;
    return;
  }

  inputString = fRootFiles.at(0)->GetName();

  if (inputString.find("ecal") != std::string::npos)
    {fDet = "ECal"; std::copy(minTimeBunchECAL, minTimeBunchECAL+8, minTimeBunch); fnRMMs = nRMM_ECAL;}
  else if (inputString.find("p0d") != std::string::npos)
    {fDet = "P0D";  std::copy(minTimeBunchP0D,  minTimeBunchP0D+8,  minTimeBunch); fnRMMs = nRMM_P0D; }
  else if (inputString.find("smrd") != std::string::npos)
    {fDet = "SMRD"; std::copy(minTimeBunchSMRD, minTimeBunchSMRD+8, minTimeBunch); fnRMMs = nRMM_SMRD;}

  else {
    std::cout << "Detector name not found in input file" << std::endl;
    return;
  }
  

  // Do All, then loop over RMMs
  FillGraphs();
  SetGraphStyle();
  DrawGraphs();

  for(Int_t rmm = 0; rmm < fnRMMs; rmm++){
    FillGraphs(rmm);
    SetGraphStyle();
    DrawGraphs(rmm);
  }
  
  return;
}

void BeamTimingData::FillGraphs(Int_t rmm) {
  
  TGraphErrors *fileHistoMean;    // define all the graphs for each file and each bunch
  TGraphErrors *fileHistoSigma;   // define all the graphs for each file and each bunch
  
  OutOfRangeSigma = 0;
  OutOfRangeSep = 0;

  // Loop over the bunches
  for(Int_t iBunch = 0; iBunch < nBunch; iBunch++){
    OutOfRangeMean[iBunch] = 0;
    
    fHistoMean [iBunch] = new TGraphErrors();
    fHistoSigma[iBunch] = new TGraphErrors();
    if (iBunch) fHistoSep[iBunch-1] = new TGraphErrors(); // Don't create for first bunch
    
    char *NameMean;
    char *NameSigma; 

    if(rmm!=-999) {
      NameMean  = Form("timingfit_rmm%02d_b%d_Mean",  rmm, iBunch+1);
      NameSigma = Form("timingfit_rmm%02d_b%d_Sigma", rmm, iBunch+1);
    }
    else {
      NameMean  = Form("timingfit_all_b%d_Mean",  iBunch+1);
      NameSigma = Form("timingfit_all_b%d_Sigma", iBunch+1);
    }

    TKey *keyMean  = fRootFiles.at(0)->FindKey(NameMean );
    TKey *keySigma = fRootFiles.at(0)->FindKey(NameSigma);
    
    if(keyMean == NULL){
      std::cout << "!!MeanHistogram does not exist!!" << std::endl;
      std::cout << "Bunch: " << iBunch+1 << std::endl;
      std::cout << "Tried:" << NameMean << std::endl;
  }

    if(keySigma == NULL){
      std::cout << "!!SigmaHistogram does not exist!!" << std::endl;
      std::cout << "Bunch:" << iBunch+1 << std::endl;
      std::cout << "Tried: " << NameSigma << std::endl;
    }
    
    // Loop over input files
    for(std::vector<TFile*>::iterator itr = fRootFiles.begin(); itr != fRootFiles.end(); ++itr){
      fileHistoMean  = NULL;
      fileHistoSigma = NULL;
      
      fileHistoMean  = (TGraphErrors*)(*itr)->Get(NameMean );
      fileHistoSigma = (TGraphErrors*)(*itr)->Get(NameSigma);
      
      if(!fileHistoMean){
	std::cout << "Mean Histo in file " << (*itr)->GetName() << " for Bunch " << iBunch+1 << " does not exist!" << std::endl;
	std::cout << "Tried: " << NameMean << std::endl;
      }
      
      if(!fileHistoSigma){
	std::cout << "Sigma Histo in file " << (*itr)->GetName() << " for Bunch " << iBunch+1 << " does not exist!" << std::endl;
	std::cout << "Tried: " << NameSigma << std::endl;
      } 

      if(!fileHistoSigma || !fileHistoMean) continue;
      
      for(Int_t iPoint = 0; iPoint < fileHistoMean->GetN(); iPoint++){
	Double_t x,y,ey,s,es,xp,yp,eyp; // x = date/time, y = mean, ey = mean error, s = sigma, es = sigma error, xp = previous date/time, yp = previous mean, eyp = previous mean error
	fileHistoMean->GetPoint(iPoint, x, y);
	ey = fileHistoMean->GetErrorY(iPoint);
	fileHistoSigma->GetPoint(iPoint, x, s);
	es = fileHistoSigma->GetErrorY(iPoint);
	
	if(x > 0 && y > 0){
	  // Bunch mean time
	  if(minTimeBunch[iBunch] < y && y < minTimeBunch[iBunch]+TimeRange) {
	    fHistoMean[iBunch]->SetPoint     (fHistoMean[iBunch]->GetN(), x, y);
	    fHistoMean[iBunch]->SetPointError(fHistoMean[iBunch]->GetN()-1, 0, ey); // -1 because a point has just been added in previous line
	  
	    // Bunch width (sigma)
	    if(minBunchSigma < 2*s && 2*s < maxBunchSigma) {
	      fHistoSigma[iBunch]->SetPoint     (fHistoSigma[iBunch]->GetN(), x, 2*s); // Bunch width is twice the sigma
	      fHistoSigma[iBunch]->SetPointError(fHistoSigma[iBunch]->GetN()-1, 0, 2*es); // Double error too
	    }
	    else
	      OutOfRangeSigma++;
	  
	    // Bunch separation
	    if(iBunch){
	      for(Int_t nPoint = 0; nPoint < fHistoMean[iBunch-1]->GetN(); nPoint++){
		fHistoMean[iBunch-1]->GetPoint(nPoint, xp, yp);
		eyp = fHistoMean[iBunch-1]->GetErrorY(nPoint);
		if(xp == x) { // if x value (time) are the same, subtract previous mean from current mean
		  if(minBunchSep < y-yp && y-yp < maxBunchSep) {
		    fHistoSep[iBunch-1]->SetPoint     (fHistoSep[iBunch-1]->GetN(), x, y-yp);
		    fHistoSep[iBunch-1]->SetPointError(fHistoSep[iBunch-1]->GetN()-1, 0, TMath::Sqrt((ey*ey)+(eyp*eyp)) );
		  }
		  else
		    OutOfRangeSep++;
		} // if(xp==x)
	      } // for(Int_t nPoint...
	    } // if(iBunch)
	  }
	  else
	    OutOfRangeMean[iBunch]++;
	  
	} // if(x > 0 && y > 0)
      } // for(Int_t iPoint...
    } // file iterator
  } // bunches
  
  return;
}

void BeamTimingData::SetGraphStyleMean() {

  fHistoMean[0]->GetXaxis()->SetTimeDisplay(1);
  fHistoMean[0]->GetXaxis()->SetTimeFormat("%d/%m");
  fHistoMean[0]->GetXaxis()->SetTimeOffset(0, "gmt");
  fHistoMean[0]->GetXaxis()->SetTitle("Date GMT");
  fHistoMean[0]->GetYaxis()->SetTitle("Bunch Time [ns]");
  fHistoMean[0]->GetXaxis()->SetTitleSize(0.15); // .15
  fHistoMean[0]->GetXaxis()->SetLabelSize(0.15); // .15
  fHistoMean[0]->GetXaxis()->SetTitleOffset(1);
  fHistoMean[0]->GetXaxis()->SetLabelOffset(0.05);
  fHistoMean[0]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
  fHistoMean[0]->GetYaxis()->SetLabelSize(0.15); // .15
  fHistoMean[0]->GetYaxis()->SetTickLength(0.035);
  
  // loop bunches
  for(Int_t iBunch = 0; iBunch < nBunch; iBunch++){
    
    if(iBunch != 0){
      // Remove the X axis
      fHistoMean[iBunch]->GetXaxis()->SetLabelOffset(999);
      fHistoMean[iBunch]->GetXaxis()->SetLabelSize(0);
      fHistoMean[iBunch]->GetXaxis()->SetTitle("");
      fHistoMean[iBunch]->GetXaxis()->SetTickLength(0);
      // Set Y axis
      fHistoMean[iBunch]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
      fHistoMean[iBunch]->GetYaxis()->SetLabelSize(0.3);
      fHistoMean[iBunch]->GetYaxis()->SetTickLength(0.02);
    }
    
    fHistoMean[iBunch]->SetTitle("");
    fHistoMean[iBunch]->GetYaxis()->SetTitle("");
    
    fHistoMean[iBunch]->SetMarkerStyle(8); // 21
    fHistoMean[iBunch]->SetMarkerSize(1.0); // .35
    // fHistoMean[iBunch]->SetMarkerColor(iBunch%4+2);
    fHistoMean[iBunch]->SetMarkerColor( (iBunch<4 ? iBunch+1 : iBunch+2) );
    fHistoMean[iBunch]->SetLineWidth(0.35);
    // fHistoMean[iBunch]->SetLineColor(iBunch%4+2);
    fHistoMean[iBunch]->SetLineColor( (iBunch<4 ? iBunch+1 : iBunch+2) );
  }

  return;
}

void BeamTimingData::SetGraphStyleSigma() {

  fHistoSigma[0]->GetXaxis()->SetTimeDisplay(1);
  fHistoSigma[0]->GetXaxis()->SetTimeFormat("%d/%m");
  fHistoSigma[0]->GetXaxis()->SetTimeOffset(0, "gmt");
  fHistoSigma[0]->GetXaxis()->SetTitle("Date GMT");
  fHistoSigma[0]->GetYaxis()->SetTitle("Bunch Time [ns]");
  fHistoSigma[0]->GetXaxis()->SetTitleSize(0.03); // .15
  fHistoSigma[0]->GetXaxis()->SetLabelSize(0.03); // .15
  fHistoSigma[0]->GetXaxis()->SetTitleOffset(1);
  fHistoSigma[0]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
  fHistoSigma[0]->GetYaxis()->SetLabelSize(0.03); // .15
  fHistoSigma[0]->GetYaxis()->SetTickLength(0.035);
 
  // loop bunches
  for(Int_t iBunch = 0; iBunch < nBunch; iBunch++){
    
    if(iBunch != 0){
      // Remove the X axis
      fHistoSigma[iBunch]->GetXaxis()->SetLabelOffset(999);
      fHistoSigma[iBunch]->GetXaxis()->SetLabelSize(0);
      fHistoSigma[iBunch]->GetXaxis()->SetTitle("");
      fHistoSigma[iBunch]->GetXaxis()->SetTickLength(0);
      // Set Y axis
      fHistoSigma[iBunch]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
      fHistoSigma[iBunch]->GetYaxis()->SetLabelSize(0.3);
      fHistoSigma[iBunch]->GetYaxis()->SetTickLength(0.02);
    }

    fHistoSigma[iBunch]->SetTitle("");
    fHistoSigma[iBunch]->GetYaxis()->SetTitle("");
    
    fHistoSigma[iBunch]->SetMarkerStyle(8); // 21
    fHistoSigma[iBunch]->SetMarkerSize(1.0); // .35
    // fHistoSigma[iBunch]->SetMarkerColor(iBunch%4+2);
    fHistoSigma[iBunch]->SetMarkerColor( (iBunch<4 ? iBunch+1 : iBunch+2) );
    fHistoSigma[iBunch]->SetLineWidth(0.35);
    // fHistoSigma[iBunch]->SetLineColor(iBunch%4+2);
    fHistoSigma[iBunch]->SetLineColor( (iBunch<4 ? iBunch+1 : iBunch+2) );
    
    fHistoSigma[iBunch]->SetMinimum(minBunchSigma);
    fHistoSigma[iBunch]->SetMaximum(maxBunchSigma);
  }

  return;
}

void BeamTimingData::SetGraphStyleSep() {

  fHistoSep[0]->GetXaxis()->SetTimeDisplay(1);
  fHistoSep[0]->GetXaxis()->SetTimeFormat("%d/%m");
  fHistoSep[0]->GetXaxis()->SetTimeOffset(0, "gmt");
  fHistoSep[0]->GetXaxis()->SetTitle("Date GMT");
  fHistoSep[0]->GetYaxis()->SetTitle("Bunch Time [ns]");
  fHistoSep[0]->GetXaxis()->SetTitleSize(0.03); // .15
  fHistoSep[0]->GetXaxis()->SetLabelSize(0.03); // .15
  fHistoSep[0]->GetXaxis()->SetTitleOffset(1);
  fHistoSep[0]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
  fHistoSep[0]->GetYaxis()->SetLabelSize(0.03); // .15
  fHistoSep[0]->GetYaxis()->SetTickLength(0.035);
  
  // loop bunches
  for(Int_t iBunch = 0; iBunch < nBunch-1; iBunch++){
    
    if(iBunch != 0){
      // Remove the X axis
      fHistoSep[iBunch]->GetXaxis()->SetLabelOffset(999);
      fHistoSep[iBunch]->GetXaxis()->SetLabelSize(0);
      fHistoSep[iBunch]->GetXaxis()->SetTitle("");
      fHistoSep[iBunch]->GetXaxis()->SetTickLength(0);
      // Set Y axis
      fHistoSep[iBunch]->GetYaxis()->SetNdivisions(2,5,0,kTRUE);
      fHistoSep[iBunch]->GetYaxis()->SetLabelSize(0.3);
      fHistoSep[iBunch]->GetYaxis()->SetTickLength(0.02);
    }
    
    fHistoSep[iBunch]->SetTitle("");
    fHistoSep[iBunch]->GetYaxis()->SetTitle("");
    
    fHistoSep[iBunch]->SetMarkerStyle(8); // 21
    fHistoSep[iBunch]->SetMarkerSize(1.0); // .35
    fHistoSep[iBunch]->SetMarkerColor( (iBunch<3 ? iBunch+2 : iBunch+3) );
    fHistoSep[iBunch]->SetLineWidth(0.35);
    fHistoSep[iBunch]->SetLineColor( (iBunch<3 ? iBunch+2 : iBunch+3) );
    
    fHistoSep[iBunch]->SetMinimum(minBunchSep);
    fHistoSep[iBunch]->SetMaximum(maxBunchSep);
  }

  return;
}

void BeamTimingData::DrawGraphMean(Int_t rmm){

  TCanvas* can = new TCanvas("can_mean",Form("%s Bunch Timing", fDet.c_str()), 600, 600);
  can->SetTitle("");

  TPad *pad[nBunch];
  TF1* lineFits[nBunch];
  TLatex *textbox[nBunch];
  
  const float ypadsep = 0.105;
  float ypadlow  = 0.0;
  float ypadhigh = 0.211;

  for(Int_t iBunch = 0; iBunch < nBunch; iBunch++){        // loop over the bunches

    // Setup the pads
    if(iBunch==0) {
      pad[iBunch] = new TPad(Form("pad_%i",iBunch), Form("pad_%i",iBunch), 0.01, 0.010, 0.99, ypadhigh+0.011);
      pad[iBunch]->SetBottomMargin(0.4); pad[iBunch]->SetBorderMode(0); pad[iBunch]->Draw();
    }
    else {
      pad[iBunch] = new TPad(Form("pad_%i",iBunch), Form("pad_%i",iBunch), 0.01, ypadlow, 0.99, ypadhigh);
      pad[iBunch]->SetBottomMargin(0.); pad[iBunch]->SetBorderMode(0); pad[iBunch]->Draw();
    }
    ypadlow = ypadhigh;
    ypadhigh += ypadsep;

    pad[iBunch]->cd();
    fHistoMean[iBunch]->Draw("AP");
    fHistoMean[iBunch]->SetMinimum(minTimeBunch[iBunch]);
    fHistoMean[iBunch]->SetMaximum(minTimeBunch[iBunch]+TimeRange);
    
    can->cd();

    // Setup line fits
    lineFits[iBunch] = new TF1(Form("linefit_%i", iBunch), "[0]", fHistoMean[0]->GetXaxis()->GetXmin(), fHistoMean[0]->GetXaxis()->GetXmax());
    lineFits[iBunch]->SetLineWidth(0.35);
    lineFits[iBunch]->SetLineColor(kBlack);
    
    if(!fHistoMean[iBunch]){
      std::cout << "all the histo are not instantiated, return" << std::endl;
      return;
    }else{
      std::cout << "line Fit starting, bunch: " << iBunch << std::endl;
      fHistoMean[iBunch]->Fit(lineFits[iBunch]);
      std::cout << "line Fit done, bunch:"<< iBunch << std::endl;
    }
    
    Double_t parVal = lineFits[iBunch]->GetParameter(0);
    Double_t parErr = lineFits[iBunch]->GetParError(0);
    Double_t chi2   = lineFits[iBunch]->GetChisquare();
    Int_t NDF       = lineFits[iBunch]->GetNDF();
    textbox[iBunch] = new TLatex(0.2, ypadlow-0.035, Form("Mean = %4.2f #pm %4.2f, #chi^{2}/NDF = %4.2f / %d (OOR = %d)", parVal,parErr,chi2,NDF, OutOfRangeMean[iBunch])); // ypadlow + 0.005
    textbox[iBunch]->SetTextSize(0.025);
    textbox[iBunch]->Draw();
    
  } // for(Int_t iBunch = 0...
  
  std::vector<TPad*> labelMask;
  // Label masks for ECal
  if (fDet.find("ECal") != std::string::npos) {
    labelMask.push_back(new TPad("","",0.0,0.705,0.105,0.745));
    labelMask.push_back(new TPad("","",0.0,0.605,0.105,0.645));
    labelMask.push_back(new TPad("","",0.0,0.505,0.105,0.550));  
    labelMask.push_back(new TPad("","",0.0,0.390,0.105,0.420));
    labelMask.push_back(new TPad("","",0.0,0.280,0.105,0.350));
    labelMask.push_back(new TPad("","",0.0,0.190,0.105,0.225));
  }

  // Label masks for P0D
  if (fDet.find("P0D") != std::string::npos) {
    labelMask.push_back(new TPad("","",0.0,0.705,0.105,0.745));
    labelMask.push_back(new TPad("","",0.0,0.605,0.105,0.645));
    labelMask.push_back(new TPad("","",0.0,0.505,0.105,0.550));  
    labelMask.push_back(new TPad("","",0.0,0.390,0.105,0.420));
    labelMask.push_back(new TPad("","",0.0,0.280,0.105,0.345));
    labelMask.push_back(new TPad("","",0.0,0.190,0.105,0.225));
  }

  // Label masks for SMRD
  if (fDet.find("SMRD") != std::string::npos) {
    labelMask.push_back(new TPad("","",0.0,0.810,0.105,0.841));
    labelMask.push_back(new TPad("","",0.0,0.710,0.105,0.745));
    labelMask.push_back(new TPad("","",0.0,0.500,0.105,0.535));  
    labelMask.push_back(new TPad("","",0.0,0.400,0.105,0.430));
    labelMask.push_back(new TPad("","",0.0,0.185,0.105,0.220));
  }
  
  for(std::vector<TPad*>::iterator it = labelMask.begin(); it != labelMask.end(); ++it)
    (*it)->Draw();

  TPad *maskPads[nBunch-1];
  TLine *vertLine = new TLine(0.991,0,0.991,1.00); 
  vertLine->SetLineWidth(0.1);
  vertLine->SetLineColor(1);

  std::vector<TLine*> cutLine;
  cutLine.push_back(new TLine(0.068,0,0.068,0.3));
  cutLine.push_back(new TLine(0.04,0.1,0.096,0.5));
  cutLine.push_back(new TLine(0.04,0.5,0.096,0.9));
  cutLine.push_back(new TLine(0.068,0.7,0.068,1));
  
  for(std::vector<TLine*>::iterator it = cutLine.begin(); it != cutLine.end(); ++it) {
    (*it)->SetLineWidth(0.1);
    (*it)->SetLineColor(1);
  }
  
  ypadhigh = 0.204;
  
  for(Int_t iBunch = 0; iBunch < nBunch-1; iBunch++){
    maskPads[iBunch] = new TPad("","",0.05,ypadhigh-0.006,0.9,ypadhigh+0.007);
    maskPads[iBunch]->Draw();
    maskPads[iBunch]->cd();
    
    vertLine->Draw();
    for(std::vector<TLine*>::iterator it = cutLine.begin(); it != cutLine.end(); ++it)
      (*it)->Draw();
    
    ypadhigh+=ypadsep;
    can->cd();
  }
  
  // Print the title
  TText *titleboxsigma;
  if(rmm!=-999)
    titleboxsigma = new TText(0.01,0.96,
	    Form("%s RMM %02d Bunch Timing [ns]", fDet.c_str(), rmm)); // 0.01 0.96
  else
    titleboxsigma = new TText(0.01,0.96,
	    Form("%s Bunch Timing [ns]", fDet.c_str(), rmm)); // 0.01 0.96

  titleboxsigma->SetNDC();
  titleboxsigma->SetTextSize(0.055); // 0.055
  titleboxsigma->Draw();

  if(rmm!=-999)
    can->SaveAs(Form("%s_bunchtiming_weekly_rmm%d.png", fDet.c_str(), rmm));
  else
    can->SaveAs(Form("%s_bunchtiming_weekly_all.png", fDet.c_str()));
  
  // clean up
  for(Int_t iBunch = 0; iBunch < nBunch; iBunch++){
    if (iBunch) delete maskPads[iBunch-1];
    delete textbox[iBunch];
    delete lineFits[iBunch];
    delete pad[iBunch];
    delete fHistoMean[iBunch];
  }

  for(std::vector<TLine*>::iterator it = cutLine.begin(); it != cutLine.end(); ++it)
    delete *it;
  cutLine.clear();

  for(std::vector<TPad*>::iterator it = labelMask.begin(); it != labelMask.end(); ++it)
    delete *it;
  labelMask.clear();

  delete titleboxsigma;
  delete vertLine;
  delete can;

  return;
}

void BeamTimingData::DrawGraphSigma(Int_t rmm){

  TCanvas* can = new TCanvas("can_sigma",
	  Form("%s Bunch Width", fDet.c_str()), 600, 600);
  can->SetTitle("");
  
  // loop bunches
  for(Int_t iBunch = 0; iBunch < nBunch; iBunch++){
    if(iBunch==0) fHistoSigma[iBunch]->Draw("AP");
    else fHistoSigma[iBunch]->Draw("P same");
  }

  // Design bunch width line
  TLine *designSigma = new TLine(fHistoSigma[0]->GetXaxis()->GetXmin(), 
	  bunchWidthDesign, fHistoSigma[0]->GetXaxis()->GetXmax(), 
	  bunchWidthDesign);
  designSigma->SetLineColor(2); designSigma->SetLineStyle(2); designSigma->Draw();

  // Print the title
  TText *titleboxsigma;
  if(rmm!=-999)
    titleboxsigma = new TText(0.01,0.96, 
	    Form("%s RMM %02d Bunch Width [ns]", fDet.c_str(), rmm)); // 0.01 0.96
  else
    titleboxsigma = new TText(0.01,0.96, 
	    Form("%s Bunch Width [ns]", fDet.c_str(), rmm)); // 0.01 0.96
  
  titleboxsigma->SetNDC();
  titleboxsigma->SetTextSize(0.055); // 0.055
  titleboxsigma->Draw();
  
  TLatex *textboxsigma = new TLatex(0.2, 0.2, Form("Out of Range = %d", OutOfRangeSigma)); // ypadlow + 0.005
  textboxsigma->SetNDC();
  //  textboxsigma->SetTextSize(0.025);
  textboxsigma->Draw();
  
  if(rmm!=-999)
    can->SaveAs(Form("%s_bunchwidth_weekly_rmm%d.png", fDet.c_str(), rmm));
  else
    can->SaveAs(Form("%s_bunchwidth_weekly_all.png", fDet.c_str()));
  
  // clean up
  for(Int_t iBunch = 0; iBunch < nBunch; iBunch++)
    delete fHistoSigma[iBunch];
    
  delete designSigma;
  delete titleboxsigma;
  delete textboxsigma;
  delete can;

  return;
}

void BeamTimingData::DrawGraphSep(Int_t rmm){

  TCanvas* can = new TCanvas("can_sep",
	  Form("%s Bunch Separation", fDet.c_str()), 600, 600);
  can->SetTitle("");
  
  // loop bunches
  for(Int_t iBunch = 0; iBunch < nBunch-1; iBunch++){
    if(iBunch==0) fHistoSep[iBunch]->Draw("AP");
    else fHistoSep[iBunch]->Draw("P same");
  }

  // Design bunch separation line
  TLine *designSeparation = new TLine(fHistoSep[0]->GetXaxis()->GetXmin(),
	  bunchSeparationDesign, fHistoSep[0]->GetXaxis()->GetXmax(), 
	  bunchSeparationDesign);

  designSeparation->SetLineColor(2); 
  designSeparation->SetLineStyle(2); 
  designSeparation->Draw();
  
  // Print the title
  TText *titleboxsep;
  if(rmm!=-999)
    titleboxsep = new TText(0.01,0.96, 
	    Form("%s RMM %02d Bunch Separation [ns]", fDet.c_str(), rmm)); // 0.01 0.96
  else
    titleboxsep = new TText(0.01,0.96, 
	    Form("%s Bunch Separation [ns]", fDet.c_str(), rmm)); // 0.01 0.96

  titleboxsep->SetNDC();
  titleboxsep->SetTextSize(0.055); // 0.055
  titleboxsep->Draw();

  TLatex *textboxsep = new TLatex(0.2, 0.2, Form("Out of Range = %d", OutOfRangeSep)); // ypadlow + 0.005
  textboxsep->SetNDC();
  //  textboxsep->SetTextSize(0.025);
  textboxsep->Draw();

  if(rmm!=-999)
    can->SaveAs(Form("%s_bunchseparation_weekly_rmm%d.png", fDet.c_str(), rmm));
  else
    can->SaveAs(Form("%s_bunchseparation_weekly_all.png", fDet.c_str()));
  
  // clean up
  for(Int_t iBunch = 0; iBunch < nBunch-1; iBunch++)
    delete fHistoSep[iBunch];

  delete designSeparation;
  delete titleboxsep;
  delete textboxsep;
  delete can;
  
  return;
}

#include "BeamTimingData.hxx"

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

  if (inputString.find("ECAL") != std::string::npos)
    {fDet = "ECAL"; std::copy(minTimeBunchECAL, minTimeBunchECAL+8, minTimeBunch);}
  else if (inputString.find("P0D") != std::string::npos)
    {fDet = "P0D";  std::copy(minTimeBunchP0D, minTimeBunchP0D+8, minTimeBunch);}
  else if (inputString.find("SMRD") != std::string::npos)
    {fDet = "SMRD"; std::copy(minTimeBunchSMRD, minTimeBunchSMRD+8, minTimeBunch);}
  else {
    std::cout << "Detector name not found in input file" << std::endl;
    return;
  }
  
  FillGraphs();
  SetGraphStyle();
  DrawGraphs();
  
  return;
}

void BeamTimingData::FillGraphs() {
  
  TGraphErrors *fileHistoMean;    // define all the graphs for each file and each bunch
  TGraphErrors *fileHistoSigma;   // define all the graphs for each file and each bunch
  
  fMinSigma = 1000.0;
  fMaxSigma = 0.0;
  fMinSep = 1000.0;
  fMaxSep = 0.0;
  
  // Loop over the bunches
  for(int iBunch = 0; iBunch < nBunch; iBunch++){
    OutOfRange[iBunch] = 0;

    fHistoMean [iBunch] = new TGraphErrors();
    fHistoSigma[iBunch] = new TGraphErrors();
    if (iBunch) fHistoSep[iBunch-1] = new TGraphErrors(); // Don't create for first bunch
    
    char *NameMean  = Form("timingfit_rmm_all_b%d_Mean",  iBunch+1);
    char *NameSigma = Form("timingfit_rmm_all_b%d_Sigma", iBunch+1);
    TKey *keyMean  = fRootFiles.at(0)->FindKey(NameMean );
    TKey *keySigma = fRootFiles.at(0)->FindKey(NameSigma);
    
    if(keyMean == NULL){
      std::cout << "!!MeanHistogram does not exist!!" << std::endl;
      std::cout << "Bunch: " << iBunch+1 << std::endl;
      std::cout << "Tried:" << NameMean << std::endl;
      throw 1;
    }

    if(keySigma == NULL){
      std::cout << "!!SigmaHistogram does not exist!!" << std::endl;
      std::cout << "Bunch:" << iBunch+1 << std::endl;
      std::cout << "Tried: " << NameSigma << std::endl;
      throw 1;
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
      
      for(int iPoint = 0; iPoint < fileHistoMean->GetN(); iPoint++){
	double x,y,ey,s,es,xp,yp,eyp; // x = date/time, y = mean, ey = mean error, s = sigma, es = sigma error, xp = previous date/time, yp = previous mean, eyp = previous mean error
	fileHistoMean->GetPoint(iPoint, x, y);
	ey = fileHistoMean->GetErrorY(iPoint);
	fileHistoSigma->GetPoint(iPoint, x, s);
	es = fileHistoSigma->GetErrorY(iPoint);
	
	if(x > 0 && y > 0){
	  if(minTimeBunch[iBunch] < y && y < minTimeBunch[iBunch]+TimeRange) {
	    // Bunch mean time
	    fHistoMean[iBunch]->SetPoint     (fHistoMean[iBunch]->GetN(), x, y);
	    fHistoMean[iBunch]->SetPointError(fHistoMean[iBunch]->GetN()-1, 0, ey); // -1 because a point has just been added in previous line
	    // Bunch width (sigma)
	    fHistoSigma[iBunch]->SetPoint     (fHistoSigma[iBunch]->GetN(), x, 2*s); // Bunch width is twice the sigma
	    fHistoSigma[iBunch]->SetPointError(fHistoSigma[iBunch]->GetN()-1, 0, 2*es); // Double error too
	    if(2*s < fMinSigma) fMinSigma = 2*s;
	    if(2*s > fMaxSigma) fMaxSigma = 2*s;
	    // Bunch separation
	    if(iBunch){
	      for(int nPoint = 0; nPoint < fHistoMean[iBunch-1]->GetN(); nPoint++){
		fHistoMean[iBunch-1]->GetPoint(nPoint, xp, yp);
		eyp = fHistoMean[iBunch-1]->GetErrorY(nPoint);
		if(xp == x) { // if x value (time) are the same, subtract previous mean from current mean
		  fHistoSep[iBunch-1]->SetPoint     (fHistoSep[iBunch-1]->GetN(), x, y-yp);
		  fHistoSep[iBunch-1]->SetPointError(fHistoSep[iBunch-1]->GetN()-1, 0, TMath::Sqrt((ey*ey)+(eyp*eyp)) );
		  if(y-yp < fMinSep) fMinSep = y-yp;
		  if(y-yp > fMaxSep) fMaxSep = y-yp;
		} // if(xp==x)
	      } // for(int nPoint...
	    } // if(iBunch)
	  }
	  else
	    OutOfRange[iBunch]++;
	} // if(x > 0 && y > 0)
      } // for(int iPoint...
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
  for(int iBunch = 0; iBunch < nBunch; iBunch++){
    
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
    
    // fHistoMean[iBunch]->SetMinimum(0.9*fMinSigma);
    // fHistoMean[iBunch]->SetMaximum(1.1*fMaxSigma);
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
  for(int iBunch = 0; iBunch < nBunch; iBunch++){
    
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
    
    fHistoSigma[iBunch]->SetMinimum(0.9*fMinSigma);
    fHistoSigma[iBunch]->SetMaximum(1.1*fMaxSigma);
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
  for(int iBunch = 0; iBunch < nBunch-1; iBunch++){
    
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
    
    fHistoSep[iBunch]->SetMinimum(0.9*fMinSep);
    fHistoSep[iBunch]->SetMaximum(1.1*fMaxSep);
  }

  return;
}

void BeamTimingData::DrawGraphMean(){

  TCanvas* can = new TCanvas("can_mean",Form("%s Bunch Timing", fDet.c_str()), 600, 600);
  can->SetTitle("");

  TPad *pad[nBunch];
  TF1* lineFits[nBunch];
  TLatex *textbox[nBunch];
  
  const float ypadsep = 0.105;
  float ypadlow  = 0.0;
  float ypadhigh = 0.211;

  for(int iBunch = 0; iBunch < nBunch; iBunch++){        // loop over the bunches

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
    
    double parVal = lineFits[iBunch]->GetParameter(0);
    double parErr = lineFits[iBunch]->GetParError(0);
    double chi2   = lineFits[iBunch]->GetChisquare();
    int NDF       = lineFits[iBunch]->GetNDF();
    textbox[iBunch] = new TLatex(0.2, ypadlow-0.035, Form("Mean = %4.2f #pm %4.2f, #chi^{2}/NDF = %4.2f / %d (OOR = %d)", parVal,parErr,chi2,NDF, OutOfRange[iBunch])); // ypadlow + 0.005
    textbox[iBunch]->SetTextSize(0.025);
    textbox[iBunch]->Draw();
    
  } // for(int iBunch = 0...
  
  TPad *maskPads[nBunch-1];
  TLine *vertLine = new TLine(0.991,0,0.991,1.00); vertLine->SetLineWidth(0.1); vertLine->SetLineColor(1);
  TLine *cutLine1 = new TLine(0.068,0,0.068,0.3);  cutLine1->SetLineWidth(0.1); cutLine1->SetLineColor(1);
  TLine *cutLine2 = new TLine(0.04,0.1,0.096,0.5); cutLine2->SetLineWidth(0.1); cutLine2->SetLineColor(1);
  TLine *cutLine3 = new TLine(0.04,0.5,0.096,0.9); cutLine3->SetLineWidth(0.1); cutLine3->SetLineColor(1);
  TLine *cutLine4 = new TLine(0.068,0.7,0.068,1);  cutLine4->SetLineWidth(0.1); cutLine4->SetLineColor(1); 
  
  ypadhigh = 0.204;

  // Set for only ECal
  TPad *labelMask1 = new TPad("","",0.0,0.705,0.105,0.745); labelMask1->Draw();
  TPad *labelMask2 = new TPad("","",0.0,0.605,0.105,0.645); labelMask2->Draw();
  TPad *labelMask3 = new TPad("","",0.0,0.505,0.105,0.550); labelMask3->Draw();  
  TPad *labelMask4 = new TPad("","",0.0,0.390,0.105,0.420); labelMask4->Draw();
  TPad *labelMask5 = new TPad("","",0.0,0.280,0.105,0.350); labelMask5->Draw();
  TPad *labelMask6 = new TPad("","",0.0,0.190,0.105,0.225); labelMask6->Draw();

  for(int iBunch = 0; iBunch < nBunch-1; iBunch++){
    maskPads[iBunch] = new TPad("","",0.05,ypadhigh-0.006,0.9,ypadhigh+0.007);
    maskPads[iBunch]->Draw();
    maskPads[iBunch]->cd();
    
    vertLine->Draw();
    cutLine1->Draw(); cutLine2->Draw(); cutLine3->Draw(); cutLine4->Draw();

    ypadhigh+=ypadsep;
    can->cd();
  }

  // // Draw a nice Y axis
  // TPad *p87 = new TPad("p87","p87",0.05,0.825,0.9,0.850);p87->Draw();p87->cd();
  // TLine *l87 = new TLine(0.991,0,0.991,0.99); l87->SetLineWidth(0.1); l87->SetLineColor(1); l87->Draw();
  // TLine *cut87_1 = new TLine(0.068,0,0.068,0.3); cut87_1->SetLineWidth(0.1); cut87_1->SetLineColor(1); cut87_1->Draw();
  // TLine *cut87_2 = new TLine(0.04,0.1,0.096,0.5); cut87_2->SetLineWidth(0.1); cut87_2->SetLineColor(1); cut87_2->Draw();
  // TLine *cut87_3 = new TLine(0.04,0.5,0.096,0.9); cut87_3->SetLineWidth(0.1); cut87_3->SetLineColor(1); cut87_3->Draw();
  // TLine *cut87_4 = new TLine(0.068,0.7,0.068,1); cut87_4->SetLineWidth(0.1); cut87_4->SetLineColor(1); cut87_4->Draw();
  
  // can->cd();
  // TPad *p76 = new TPad("p76","p76",0.0,0.710,0.9,0.745);p76->Draw();p76->cd();
  // TLine *l76 = new TLine(0.991,0,0.991,0.99); l76->SetLineWidth(0.1); l76->SetLineColor(1); l76->Draw();
  // TLine *cut76_1 = new TLine(0.12,0,0.12,0.3); cut76_1->SetLineWidth(0.1); cut76_1->SetLineColor(1); cut76_1->Draw();
  // TLine *cut76_2 = new TLine(0.09,0.1,0.146,0.5); cut76_2->SetLineWidth(0.1); cut76_2->SetLineColor(1); cut76_2->Draw();
  // TLine *cut76_3 = new TLine(0.09,0.5,0.146,0.9); cut76_3->SetLineWidth(0.1); cut76_3->SetLineColor(1); cut76_3->Draw();
  // TLine *cut76_4 = new TLine(0.12,0.7,0.12,1); cut76_4->SetLineWidth(0.1); cut76_4->SetLineColor(1); cut76_4->Draw();
  
  // can->cd();
  // TPad *p65label_mask = new TPad("p65lm","p65lm",0.,0.605,0.105,0.645);p65label_mask->Draw();
  // TPad *p65 = new TPad("p65","p65",0.05,0.613,0.9,0.638);p65->Draw();p65->cd();
  // TLine *l65 = new TLine(0.991,0,0.991,0.99); l65->SetLineWidth(0.1); l65->SetLineColor(1); l65->Draw();
  // TLine *cut65_1 = new TLine(0.068,0,0.068,0.3); cut65_1->SetLineWidth(0.1); cut65_1->SetLineColor(1); cut65_1->Draw();
  // TLine *cut65_2 = new TLine(0.04,0.1,0.096,0.5); cut65_2->SetLineWidth(0.1); cut65_2->SetLineColor(1); cut65_2->Draw();
  // TLine *cut65_3 = new TLine(0.04,0.5,0.096,0.9); cut65_3->SetLineWidth(0.1); cut65_3->SetLineColor(1); cut65_3->Draw();
  // TLine *cut65_4 = new TLine(0.068,0.7,0.068,1); cut65_4->SetLineWidth(0.1); cut65_4->SetLineColor(1); cut65_4->Draw();
  
  // can->cd();
  // TPad *p54label_mask = new TPad("p54lm","p54lm",0.,0.505,0.105,0.55);p54label_mask->Draw();
  // TPad *p54 = new TPad("p54","p54",0.05,0.508,0.9,0.533);p54->Draw();p54->cd();
  // TLine *l54 = new TLine(0.991,0,0.991,1); l54->SetLineWidth(0.1); l54->SetLineColor(1); l54->Draw();
  // TLine *cut54_1 = new TLine(0.068,0,0.068,0.3);  cut54_1->SetLineWidth(0.1); cut54_1->SetLineColor(1); cut54_1->Draw();
  // TLine *cut54_2 = new TLine(0.04,0.1,0.096,0.5); cut54_2->SetLineWidth(0.1); cut54_2->SetLineColor(1); cut54_2->Draw();
  // TLine *cut54_3 = new TLine(0.04,0.5,0.096,0.9); cut54_3->SetLineWidth(0.1); cut54_3->SetLineColor(1); cut54_3->Draw();
  // TLine *cut54_4 = new TLine(0.068,0.7,0.068,1);  cut54_4->SetLineWidth(0.1); cut54_4->SetLineColor(1); cut54_4->Draw();
  
  // can->cd();
  // TPad *p43label_mask = new TPad("p43lm","p43lm",0.,0.39,0.105,0.43);p43label_mask->Draw();
  // TPad *p43 = new TPad("p43","p43",0.05,0.403,0.9,0.428);p43->Draw();p43->cd();
  // TLine *l43 = new TLine(0.991,0,0.991,0.99);l43->SetLineWidth(0.1);l43->SetLineColor(1);l43->Draw();
  // TLine *cut43_1 = new TLine(0.068,0,0.068,0.3);  cut43_1->SetLineWidth(0.1); cut43_1->SetLineColor(1); cut43_1->Draw();
  // TLine *cut43_2 = new TLine(0.04,0.1,0.096,0.5); cut43_2->SetLineWidth(0.1); cut43_2->SetLineColor(1); cut43_2->Draw();
  // TLine *cut43_3 = new TLine(0.04,0.5,0.096,0.9); cut43_3->SetLineWidth(0.1); cut43_3->SetLineColor(1); cut43_3->Draw();
  // TLine *cut43_4 = new TLine(0.068,0.7,0.068,1);  cut43_4->SetLineWidth(0.1); cut43_4->SetLineColor(1); cut43_4->Draw();
  
  // can->cd();
  // TPad *p32label_mask = new TPad("p32lm","p32lm",0.0,0.280,0.105,0.35);p32label_mask->Draw();
  // TPad *p32 = new TPad("p32","p32",0.05,0.297,0.9,0.322);p32->Draw();p32->cd();
  // TLine *l32 = new TLine(0.991,0,0.991,0.99); l32->SetLineWidth(0.1); l32->SetLineColor(1); l32->Draw();
  // TLine *cut32_1 = new TLine(0.068,0,0.068,0.3);  cut32_1->SetLineWidth(0.1); cut32_1->SetLineColor(1); cut32_1->Draw();
  // TLine *cut32_2 = new TLine(0.04,0.1,0.096,0.5); cut32_2->SetLineWidth(0.1); cut32_2->SetLineColor(1); cut32_2->Draw();
  // TLine *cut32_3 = new TLine(0.04,0.5,0.096,0.9); cut32_3->SetLineWidth(0.1); cut32_3->SetLineColor(1); cut32_3->Draw();
  // TLine *cut32_4 = new TLine(0.068,0.7,0.068,1);  cut32_4->SetLineWidth(0.1); cut32_4->SetLineColor(1); cut32_4->Draw();
  
  //   can->cd();
  // TPad *p21label_mask = new TPad("p21lm","p21lm",0.0,0.18,0.105,0.225);p21label_mask->Draw();
  // TPad *p21 = new TPad("p21","p21",0.05,0.185,0.9,0.217);p21->Draw();p21->cd();
  // TLine *l21 = new TLine(0.991,0,0.991,0.99); l21->SetLineWidth(0.1); l21->SetLineColor(1); l21->Draw();
  // TLine *cut21_1 = new TLine(0.068,0,0.068,0.3);  cut21_1->SetLineWidth(0.1); cut21_1->SetLineColor(1); cut21_1->Draw();
  // TLine *cut21_2 = new TLine(0.04,0.1,0.096,0.5); cut21_2->SetLineWidth(0.1); cut21_2->SetLineColor(1); cut21_2->Draw();
  // TLine *cut21_3 = new TLine(0.04,0.5,0.096,0.9); cut21_3->SetLineWidth(0.1); cut21_3->SetLineColor(1); cut21_3->Draw();
  // TLine *cut21_4 = new TLine(0.068,0.7,0.068,1);  cut21_4->SetLineWidth(0.1); cut21_4->SetLineColor(1); cut21_4->Draw();
  // can->cd();
  
  
  // Print the title
  TText *titleboxsigma = new TText(0.01,0.96, Form("%s Bunch Timing [ns]", fDet.c_str())); // 0.01 0.96
  titleboxsigma->SetNDC();
  titleboxsigma->SetTextSize(0.055); // 0.055
  titleboxsigma->Draw();


  can->SaveAs(Form("%s_bunchtiming_weekly_all.png", fDet.c_str()));
  
  return;
}

void BeamTimingData::DrawGraphSigma(){

  TCanvas* can = new TCanvas("can_sigma",Form("%s Bunch Width", fDet.c_str()), 600, 600);
  can->SetTitle("");
  
  // loop bunches
  for(int iBunch = 0; iBunch < nBunch; iBunch++){
    if(iBunch==0) fHistoSigma[iBunch]->Draw("AP");
    else fHistoSigma[iBunch]->Draw("P same");
  }

  // Print the title
  TText *titleboxsigma = new TText(0.01,0.96, Form("%s Bunch Width [ns]", fDet.c_str())); // 0.01 0.96
  titleboxsigma->SetNDC();
  titleboxsigma->SetTextSize(0.055); // 0.055
  titleboxsigma->Draw();
  
  can->SaveAs(Form("%s_bunchwidth_weekly_all.png", fDet.c_str()));
  
  return;
}

void BeamTimingData::DrawGraphSep(){

  TCanvas* can = new TCanvas("can_sep",Form("%s Bunch Separation", fDet.c_str()), 600, 600);
  can->SetTitle("");
  
  // loop bunches
  for(int iBunch = 0; iBunch < nBunch-1; iBunch++){
    if(iBunch==0) fHistoSep[iBunch]->Draw("AP");
    else fHistoSep[iBunch]->Draw("P same");
  }

  // Print the title
  TText *titleboxsigma = new TText(0.01,0.96, Form("%s Bunch Separation [ns]", fDet.c_str())); // 0.01 0.96
  titleboxsigma->SetNDC();
  titleboxsigma->SetTextSize(0.055); // 0.055
  titleboxsigma->Draw();
  
  can->SaveAs(Form("%s_bunchseparation_weekly_all.png", fDet.c_str()));
  
  return;
}
  
int main(int argc, char* argv[]) {
  
  // Process the options.
  for (;;) {
    int c = getopt(argc, argv, "e:p:s:");
    if (c<0) break;
    switch (c) {
    case 'e': {
      BeamTimingData ecal(optarg);
      break; }
    case 'p': {
      BeamTimingData p0d(optarg);
      break; }
    case 's': {
      BeamTimingData smrd(optarg);
      break; }
    }
  } // Closes process options for loop  
  
  return 0;

}

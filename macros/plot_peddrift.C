{

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
  gStyle->SetPadBorderSize(1);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleSize(0.7);
  gStyle->SetTitleXSize(0.04);
  gStyle->SetTitleYSize(0.04);

  // Say it in black and white!
  gStyle->SetAxisColor(1, "xyz");
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);
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
  gStyle->SetOptStat(00000);  

  //###############################################################  

  // Read input files
  int nlines = 0;
  string line;
  ifstream myfile("files.list");

  while(getline(myfile,line)){ ++nlines; }

  TFile *rootfile[nlines];

  TString fileName("files.list");
  ifstream inputFile(fileName.Data(), std::ios::in);
  
  if (!inputFile) {
    std::cerr << "Can not open input file: " << fileName.Data() << endl;
    return;
  }  

  std::string inputString;
  int i=0;
  while (inputFile >> inputString) {
    cout << "Adding file: " << inputString << endl;
    rootfile[i] = new TFile(inputString.c_str());
    i++;
  }
  //////////////////////////////////////////////////////////////////////

  TH2F *base;
  TH2F *histo[nlines];

  char hname[50];   
  char hnameLo[50];   
  char hnameOR[50];   
  char hnameLoOR[50];   
  char file[50];   
  char text[100]; 
  int rmm;

  TCanvas c0("c0","--c0--",472,0,2*800,900);
  c0->ToggleEventStatus();
  c0->Clear();
  c0->Divide(3,2);

  //Loop over canvas
  for (int y=0; y<4; y++){
 
    // Loop over each RMMs: 3 RMMs per canvas, Low and High pedestals
    for (int i=0; i<3; i++){

      rmm = i+(y*3);
      cout <<"Processing RMM : "<< rmm << endl;
      c0->cd(i+1);
      gPad->SetLogz();

      sprintf(hname,"HiPedDriftRMM%d", rmm);      
      sprintf(hnameLo,"LoPedDriftRMM%d", rmm);      
      sprintf(hnameOR,"HiPedDriftAll");      
      sprintf(hnameLoOR,"LoPedDriftAll");      
      if ((TFile*)rootfile[0]->GetListOfKeys()->Contains(hnameLo))
	for(int k=0;k<nlines;k++) histo[k] = (TH2F*)rootfile[k]->Get(hnameLo);
      else break;
      
      base = (TH2F*)rootfile[0]->Get(hnameLoOR);
      base->SetDirectory(0);
      base->SetTitle(Form("#splitline{Low Ped Drift RMM%d}{ADC counts x 100}", rmm));

      base->GetXaxis()->SetTitle("Date GMT");
      base->GetXaxis()->SetTitleOffset(1.3);
      base->GetXaxis()->SetTitleSize(0.05);
      base->GetXaxis()->SetLabelOffset(0.01);
      base->GetXaxis()->SetLabelSize(0.045);
      base->GetYaxis()->SetLabelSize(0.05);

      TAxis *xaxismin = histo[0]->GetXaxis();
      TAxis *xaxismax = histo[nlines-1]->GetXaxis();

      base->GetXaxis()->SetLimits(xaxismin->GetXmin(),xaxismax->GetXmax());

      TLine* ul=new TLine(xaxismin->GetXmin(),50.,xaxismax->GetXmax(),50.);
      TLine* ll=new TLine(xaxismin->GetXmin(),-50.,xaxismax->GetXmax(),-50.);
      ul->SetLineColor(2);
      ll->SetLineColor(2);

      base->Draw("");
      base->SetMarkerColor(0);

      for(int k=0;k<nlines;k++){
        if(i != 2) histo[k]->Draw("same col");
	else       histo[k]->Draw("same colz");
        histo[k]->GetZaxis()->SetRangeUser(0,5000);
      }

      base->GetXaxis().SetTimeFormat("%d\/%m");

      ul->Draw();
      ll->Draw();

      // TText *t1 = new TText();
      // t1->SetTextFont(62);
      // t1->SetTextColor(1);   
      // t1->SetTextAlign(12);
      // t1->SetTextSize(0.06);
      // t1->DrawTextNDC(0.21,0.95,hnameLo);
      // t1->Draw();

      c0->cd(i+4);
      gPad->SetLogz(); 

      if ((TFile*)rootfile[0]->GetListOfKeys()->Contains(hname))
	for(int k=0;k<nlines;k++) histo[k] = (TH2F*)rootfile[k]->Get(hname);
      else break;

      base = (TH2F*)rootfile[0]->Get(hnameOR);
      base->SetDirectory(0);
      base->SetTitle(Form("#splitline{High Ped Drift RMM%d}{ADC counts x 100}", rmm));

      base->GetXaxis()->SetTitle("Date GMT");
      base->GetXaxis()->SetTitleOffset(1.3);
      base->GetXaxis()->SetTitleSize(0.05);
      base->GetXaxis()->SetLabelOffset(0.01);
      base->GetXaxis()->SetLabelSize(0.045);
      base->GetYaxis()->SetLabelSize(0.05);

      TAxis *xaxismin = histo[0]->GetXaxis();
      TAxis *xaxismax = histo[nlines-1]->GetXaxis();

      base->GetXaxis()->SetLimits(xaxismin->GetXmin(),xaxismax->GetXmax());

      TLine* ul=new TLine(xaxismin->GetXmin(),50.,xaxismax->GetXmax(),50.);
      TLine* ll=new TLine(xaxismin->GetXmin(),-50.,xaxismax->GetXmax(),-50.);
      ul->SetLineColor(2);
      ll->SetLineColor(2);

      base->Draw("");
      base->SetMarkerColor(0);

      for(int k=0;k<nlines;k++){
        if(i != 2) histo[k]->Draw("same col");
	else       histo[k]->Draw("same colz");
        histo[k]->GetZaxis()->SetRangeUser(0,5000);
      }
      
      base->GetXaxis().SetTimeFormat("%d\/%m");


      ul->Draw();
      ll->Draw();

      // TText *t1 = new TText();
      // t1->SetTextFont(62);
      // t1->SetTextColor(1); 
      // t1->SetTextAlign(12);
      // t1->SetTextSize(0.06);
      // t1->DrawTextNDC(0.21,0.95,hname);
      // t1->Draw();

    }
    
    if ((TFile*)rootfile[0]->GetListOfKeys()->Contains(hname)) {NULL;}
    else break;


    sprintf(file,"PedDriftRMM%d.png", y);      
    
    c0->SaveAs(file);
 
  }
  
}

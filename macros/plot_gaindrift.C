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
  gStyle->SetTitleBorderSize(0.);
  gStyle->SetTitleFontSize(0.07);

  // Say it in black and white!
  gStyle->SetAxisColor(1, "xyz");
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);
  gStyle->SetFrameLineColor(1);
  gStyle->SetHistFillColor(0);
  gStyle->SetHistLineColor(1);
  //gStyle->SetPadColor(1);
  gStyle->SetPadColor(kWhite);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadTopMargin(0.11);
  gStyle->SetStatColor(0);
  gStyle->SetStatTextColor(1);
  gStyle->SetTitleColor(1);
  gStyle->SetTitleTextColor(1);
  gStyle->SetLabelColor(1,"xyz");
  // Show functions in red...
  gStyle->SetFuncColor(2);
  gStyle->SetOptStat(00000);  
  // Colour Palette
  gStyle->SetNumberContours(255);
  gStyle->SetPalette(55);

  ///////////////////////////////////////////////////////////////////

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
  int i = 0;
  while(inputFile >> inputString) {
    cout << "Adding file: " << inputString << endl;
    rootfile[i] = new TFile(inputString.c_str());
    i++;
  }

  // Read detector information
  int nRMMs = 0;
  std::string det = "";

  if (inputString.find("ECAL") != std::string::npos)
    {nRMMs = 12; det = "ECal";}
  else if (inputString.find("P0D") != std::string::npos)
    {nRMMs = 6; det = "P0D";}
  else if (inputString.find("SMRD") != std::string::npos)
    {nRMMs = 4; det = "SMRD";}
  else
    std::cout<<"Detector name not found in input file";

  //////////////////////////////////////////////////////////////////////
  
  TH2F *base;
  TH2F *histo[nlines];
  
  char hname[50];
  char hnameOR[50];
  char hfile[50];
  
  //  TCanvas c0("c0","--c0--",472,0,800,900);
  
  //  int rmm;
  
  int canwidth = 800;
  int canheight = 400;
  
  TCanvas c0("c0","--c0--",canwidth,canheight);
  c0->ToggleEventStatus();
  c0->Clear();
  
  gPad->SetLogz();

  for (int rmm = 0; rmm < nRMMs; rmm++) {

    sprintf(hname,   "GainDriftRMM%d",rmm);      
    sprintf(hnameOR, "GainDriftAll");
    
    cout << "Processing RMM : " << rmm << endl;
    
    if ((TFile*)rootfile[0]->GetListOfKeys()->Contains(hname))
      for(int k = 0; k < nlines; k++) histo[k] = (TH2F*)rootfile[k]->Get(hname);
    else break;
    
    TAxis *xaxismin = histo[0]->GetXaxis();
    TAxis *xaxismax = histo[nlines-1]->GetXaxis();
    
    base = (TH2F*)rootfile[0]->Get(hnameOR);
    base->SetDirectory(0);
    base->SetTitle(Form("%s Gain Drift RMM%i: ADC counts x 100", det.c_str(), rmm));
    
    base->GetXaxis()->SetTitle("Date GMT");
    base->GetXaxis()->SetTitleOffset(1.0);
    base->GetXaxis()->SetTitleSize(0.07);
    
    // base->GetYaxis()->SetTitle("ADC counts x 100");
    base->GetXaxis()->SetLabelSize(0.07);
    base->GetYaxis()->SetLabelSize(0.07);
    
    base->GetXaxis()->SetLimits(xaxismin->GetXmin(),xaxismax->GetXmax());
    
    TLine* ul=new TLine(xaxismin->GetXmin(),50.,xaxismax->GetXmax(),50.);
    TLine* ll=new TLine(xaxismin->GetXmin(),-50.,xaxismax->GetXmax(),-50.);
    ul->SetLineColor(2);
    ll->SetLineColor(2);
    
    base->Draw("");
    base->SetMarkerColor(0);
    
    for(int k=0;k<nlines;k++){
      histo[k]->Draw("same colz");
      histo[k]->GetZaxis()->SetRangeUser(0,5000);
    }
    
    base->GetXaxis().SetTimeFormat("%d\/%m");
    
    ul->Draw();
    ll->Draw();
    
    // // Draw Title
    // TText *t1 = new TText();
    // t1->SetTextFont(62);
    // t1->SetTextColor(1); 
    // t1->SetTextAlign(12);
    // t1->SetTextSize(0.06);
    // t1->DrawTextNDC(0.21,0.95,hname);
    // t1->Draw();
    
    // }
  
    sprintf(hfile,"gaindriftnew%s_RMM%d.png",det.c_str(),rmm);      

    if ((TFile*)rootfile[0]->GetListOfKeys()->Contains(hname)) {NULL;}
    else break;
    
    c0->SaveAs(hfile);    
    
  }
  
}

{
  int bunches = 8;
  auto c1 = new TCanvas("c1","c1",600,600);
  
  TGraph *graphs[bunches];

  auto legend = new TLegend(0.05,0.05,0.95,0.95);
  legend->SetFillColor(0);
  legend->SetNColumns(2);  

  for(int i = 0; i < bunches; i++) {
    graphs[i] = new TGraphErrors();
    
    graphs[i]->SetMarkerStyle(8);
    graphs[i]->SetMarkerSize(2.0); //1
    graphs[i]->SetMarkerColor((i<4 ? i+1 : i+2));
    graphs[i]->SetLineWidth(4.0); //0.35
    graphs[i]->SetLineColor((i<4 ? i+1 : i+2));

    legend->AddEntry(graphs[i], Form("Bunch %i", i+1),"lep");  
  }  
  
  legend->Draw();
  c1->SaveAs("BunchMeanWidthLegend.png");
}

{
  int bunches = 7;
  auto c1 = new TCanvas("c1","c1",600,600);
  
  TGraph *graphs[bunches];

  auto legend = new TLegend(0.05,0.05,0.95,0.95);
  legend->SetFillColor(0);
  legend->SetNColumns(2);  

  for(int i = 0; i < bunches; i++) {
    graphs[i] = new TGraphErrors();
    
    graphs[i]->SetMarkerStyle(8);
    graphs[i]->SetMarkerSize(2.0); //1
    graphs[i]->SetMarkerColor((i<3 ? i+2 : i+3));
    graphs[i]->SetLineWidth(4.0); //0.35
    graphs[i]->SetLineColor((i<3 ? i+2 : i+3));

    legend->AddEntry(graphs[i], Form("Bunch %i to %i", i+1, i+2),"lep");  
  }  
  
  legend->Draw();
  c1->SaveAs("BunchSeparationLegend.png");
}

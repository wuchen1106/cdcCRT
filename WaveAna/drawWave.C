//           
// Aurthor: Sam Wong           
// 
// Summary of the code
//
// 1. plotEve()
// Select an event and plot according to hardware board number
//
// 2. SortBoard()
// Plot the accumulative waveform 


const Int_t MAX_CLK = 32;
const Int_t MAX_BOARDS = 8;
const Int_t MAX_CH = MAX_BOARDS*48;
TTree* t;
TFile *_file0;
Int_t tdcNhit[MAX_CH];
Int_t adc[MAX_CH][MAX_CLK];
Int_t clk[MAX_CH][MAX_CLK];
Int_t driftTime[MAX_CH][MAX_CLK];
Int_t triggerNumber;
Int_t num_events;

void readRoot(Int_t runNo){
  //Plot 1 event waveform
  //read root
  char* fileName = Form("../root/run_%06d.root",runNo);
  _file0 = TFile::Open(fileName);
  if(!_file0){
    std::cerr<<"No such file"<<std::endl;
  }
  t = (TTree*)_file0->Get("tree");
  num_events  = t->GetEntries();
  t->SetBranchAddress("tdcNhit",&tdcNhit);
  t->SetBranchAddress("adc",&adc);
  t->SetBranchAddress("clockNumberDriftTime",&clk);
  t->SetBranchAddress("driftTime",&driftTime);
  t->SetBranchAddress("triggerNumber",&triggerNumber);
}
void plotEve(Int_t runNo, Int_t iev)
{ 
  readRoot(runNo);
  //create plotting object
  TCanvas* c[MAX_BOARDS];
  for(Int_t ibd=0;ibd<MAX_BOARDS;ibd++){
    c[ibd] = new TCanvas(Form("c%d",ibd),Form("c%d",ibd),1024,720);
    c[ibd] -> Divide(8,6);
  }
  TGraph* g_wave[MAX_CH];
  TGraph* g_wave_hit[MAX_CH];
  Int_t index = -1;
  for(int i=0;i<t->GetEntries();i++){
    t->GetEntry(i);
    if(iev==triggerNumber){
      index=i;
      break;
    }
  }
  t->GetEntry(index);
  for(Int_t ch=0;ch<MAX_CH;ch++){
    g_wave[ch] = new TGraph(MAX_CLK);
    g_wave[ch] -> SetMarkerStyle(4);
    g_wave[ch] -> SetMarkerSize(0.1);
    g_wave[ch] -> SetMarkerColor(1);
    g_wave[ch] -> SetLineColor(1);
    
    g_wave_hit[ch] = new TGraph(MAX_CLK);
    g_wave_hit[ch] -> SetMarkerStyle(4);
    g_wave_hit[ch] -> SetMarkerSize(0.1);
    g_wave_hit[ch] -> SetMarkerColor(2);
    g_wave_hit[ch] -> SetLineColor(2);    
    for(Int_t k=0;k<MAX_CLK;k++){
      Int_t g_wave_x=k;
      Int_t g_wave_y=adc[ch][k];
      g_wave[ch]->SetPoint(k,g_wave_x,g_wave_y);
      //TDC hit
      for(Int_t w=0;w<MAX_CLK;w++){
	if(clk[ch][w]==k){
	  Int_t g_wave_x=clk[ch][w];
	  Int_t g_wave_y=adc[ch][clk[ch][w]];
	  g_wave_hit[ch]->SetPoint(clk[ch][w],g_wave_x,g_wave_y);
	}
      }
    }
  }

  //draw 
  TH2D* h_frame[MAX_CH];
  for(Int_t ch=0;ch<MAX_CH;ch++){
    Int_t ibd = ch/48;
    h_frame[ch] = new TH2D(Form("h%d",ch),Form("Board%d ch%d",ibd,ch),64,-1,32,100,100,700);
  }

  for(Int_t ch=0;ch<MAX_CH;ch++){
    Int_t ibd=ch/48;
    c[ibd]->cd(ch%48+1);
    h_frame[ch]->Draw();
    h_frame[ch]->GetXaxis()->SetTitle("CLK");
    h_frame[ch]->GetYaxis()->SetTitle("ADC");
    h_frame[ch]->GetYaxis()->SetTitleOffset(1.4);
    g_wave[ch]->Draw("same pl");
    g_wave_hit[ch]->Draw("same p");
  }
  for(Int_t ibd=0;ibd<MAX_BOARDS;ibd++){
    c[ibd]->SaveAs(Form("WF_run%d_b%d_%d.pdf",runNo,ibd,triggerNumber));
  }
}

void plotEve(Int_t runNo, Int_t start, Int_t end)
{ 
  readRoot(runNo);
  //create plotting object
  TCanvas* c[MAX_BOARDS];
  for(Int_t ibd=0;ibd<MAX_BOARDS;ibd++){
    c[ibd] = new TCanvas(Form("c%d",ibd),Form("c%d",ibd),1024,720);
    c[ibd] -> Divide(8,6);
  }
  TGraph* g_wave[MAX_CH];
  TGraph* g_wave_hit[MAX_CH];
  
  for(int iev=start; iev<end; iev++ ){
    t->GetEntry(iev);
    for(Int_t ch=0;ch<MAX_CH;ch++){
      g_wave[ch] = new TGraph(MAX_CLK);
      g_wave[ch] -> SetMarkerStyle(4);
      g_wave[ch] -> SetMarkerSize(0.1);
      g_wave[ch] -> SetMarkerColor(1);
      g_wave[ch] -> SetLineColor(1);
      
      g_wave_hit[ch] = new TGraph(MAX_CLK);
      g_wave_hit[ch] -> SetMarkerStyle(4);
      g_wave_hit[ch] -> SetMarkerSize(0.1);
      g_wave_hit[ch] -> SetMarkerColor(2);
      g_wave_hit[ch] -> SetLineColor(2);    
      for(Int_t k=0;k<MAX_CLK;k++){
	Int_t g_wave_x=k;
	Int_t g_wave_y=adc[ch][k];
	g_wave[ch]->SetPoint(k,g_wave_x,g_wave_y);
	//TDC hit
	for(Int_t w=0;w<MAX_CLK;w++){
	  if(clk[ch][w]==k){
	    Int_t g_wave_x=clk[ch][w];
	    Int_t g_wave_y=adc[ch][clk[ch][w]];
	    g_wave_hit[ch]->SetPoint(clk[ch][w],g_wave_x,g_wave_y);
	  }
	}
      }
    }
    std::cout << "debug : " << __LINE__ << std::endl;
    //draw 
    TH2D* h_frame[MAX_CH];
    for(Int_t ch=0;ch<MAX_CH;ch++){
      Int_t ibd = ch/48;
      h_frame[ch] = new TH2D(Form("h%d",ch),Form("Board%d ch%d",ibd,ch),64,-1,32,100,100,700);
    }
    std::cout << "debug : " << __LINE__ << std::endl;
    for(Int_t ch=0;ch<MAX_CH;ch++){
      Int_t ibd=ch/48;
      c[ibd]->cd(ch%48+1);
      h_frame[ch]->Draw();
      h_frame[ch]->GetXaxis()->SetTitle("CLK");
      h_frame[ch]->GetYaxis()->SetTitle("ADC");
      h_frame[ch]->GetYaxis()->SetTitleOffset(1.4);
      g_wave[ch]->Draw("same pl");
      g_wave_hit[ch]->Draw("same p");
    }
    std::cout << "debug : " << __LINE__ << std::endl;
    for(Int_t ibd=0;ibd<MAX_BOARDS;ibd++){
      c[ibd]->SaveAs(Form("WF_run%d_b%d_%d.pdf",runNo,ibd,triggerNumber));
      c[ibd]->Close();
    }
//     std::cout << "debug : " << __LINE__ << std::endl;
//     //clear 
//     for(Int_t ch=0;ch<MAX_CH;ch++){
//       delete h_frame[ch];
      // delete g_wave_hit[ch];
//       delete g_wave[ch];
//    }
  }
}

void SortBoard(Int_t runNo)
{
  //read root
  readRoot(runNo);
  //create plotting object
  TCanvas* c[MAX_BOARDS];
  for(Int_t ibd=0;ibd<MAX_BOARDS;ibd++){
    c[ibd] = new TCanvas(Form("c%d",ibd),Form("c%d",ibd),1024,720);
    c[ibd] -> Divide(8,6);
  }
  TH2D* h[MAX_CH];
  for(Int_t ch=0;ch<MAX_CH;ch++){
    h[ch] = new TH2D(Form("h%d",ch),Form("h%d",ch),64,-32,32,100,0,700);
  }
  //  event loop
  for(Int_t iev=0;iev<num_events;iev++){
    t->GetEntry(iev);
    for(Int_t ch=0;ch<MAX_CH;ch++){
      if(tdcNhit[ch]>0){
	for(Int_t k=0;k<MAX_CLK;k++){
	  if(driftTime)h[ch]->Fill(k-clk[ch][0],adc[ch][k]);
	}
      }
    }
  }
  //draw 
  for(Int_t ch=0;ch<MAX_CH;ch++){
    Int_t ibd=ch/48;
    c[ibd]->cd(ch%48+1);
    gPad->SetLogz();
    h[ch]->Draw("colz");
    h[ch]->GetXaxis()->SetTitle("CLK");
    h[ch]->GetYaxis()->SetTitle("ADC");
    h[ch]->GetYaxis()->SetTitleOffset(1.4);

  }
  for(Int_t ibd=0;ibd<MAX_BOARDS;ibd++){
    c[ibd]->SaveAs(Form("WF_run%d_b%d.pdf",runNo,ibd));
  }
}

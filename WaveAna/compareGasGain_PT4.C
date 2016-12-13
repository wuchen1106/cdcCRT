{
	new TCanvas(); 

	gStyle->SetPalette(1);
	gStyle->SetOptStat(0);
	gStyle->SetPadTickX(1);
	gStyle->SetPadTickY(1);
	gPad->SetGridx(1);
	gPad->SetGridy(1);

	TFile * ifile = new TFile("../root/h_1003.root");
	TTree * itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h1003(200,0,700)","(nHits-16)<=2");
	double N = h1003->Integral(); h1003->Scale(1/N);
	h1003->SetTitle("ADC (-pedestal) sum");
	h1003->GetXaxis()->SetTitle("ADC");

	ifile = new TFile("../root/h_1004.root");
	itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h1004(200,0,700)","(nHits-16)<=2","SAME");
	N = h1004->Integral(); h1004->Scale(1/N);

	ifile = new TFile("../root/h_1005.root");
	itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h1005(200,0,700)","(nHits-16)<=2","SAME");
	N = h1005->Integral(); h1005->Scale(1/N);

	ifile = new TFile("../root/h_1006.root");
	itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h1006(200,0,700)","(nHits-16)<=2","SAME");
	N = h1006->Integral(); h1006->Scale(1/N);

	ifile = new TFile("../root/h_1008.root");
	itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h1008(200,0,700)","(nHits-16)<=2","SAME");
	N = h1008->Integral(); h1008->Scale(1/N);

	ifile = new TFile("../root/h_1009.root");
	itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h1009(200,0,700)","(nHits-16)<=2","SAME");
	N = h1009->Integral(); h1009->Scale(1/N);

	h1003->SetLineColor(kGreen);
	h1004->SetLineColor(kBlue);
	h1005->SetLineColor(kMagenta);
	h1006->SetLineColor(kYellow);
	h1008->SetLineColor(kOrange);
	h1009->SetLineColor(kRed);

	TLegend * legend = new TLegend(0.3,0.5,0.85,0.85);
	legend->AddEntry(h1003,"#1003, set2");
	legend->AddEntry(h1004,"#1004, set2");
	legend->AddEntry(h1005,"#1005, set3");
	legend->AddEntry(h1006,"#1006, set3");
	legend->AddEntry(h1008,"#1008, set3");
	legend->AddEntry(h1009,"#1009, set3");
	legend->Draw("SAME");
}

{
	new TCanvas(); 

	gStyle->SetPalette(1);
	gStyle->SetOptStat(0);
	gStyle->SetPadTickX(1);
	gStyle->SetPadTickY(1);
	gPad->SetGridx(1);
	gPad->SetGridy(1);

	TFile * ifile = new TFile("../root/h_53.root");
	TTree * itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h53(200,0,700)","(nHits-14)<=1");
	double N = h53->Integral(); h53->Scale(1/N);
	h53->SetTitle("ADC (-pedestal) sum");
	h53->GetXaxis()->SetTitle("ADC");

	ifile = new TFile("../root/h_55.root");
	itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h55(200,0,700)","(nHits-20)<=1","SAME");
	N = h55->Integral(); h55->Scale(1/N);

	ifile = new TFile("../root/h_56.root");
	itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h56(200,0,700)","(nHits-23)<=1","SAME");
	N = h56->Integral(); h56->Scale(1/N);

	ifile = new TFile("../root/h_57.root");
	itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h57(200,0,700)","(nHits-18)<=1","SAME");
	N = h57->Integral(); h57->Scale(4/N);

	ifile = new TFile("../root/h_58.root");
	itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h58(200,0,700)","(nHits-18)<=1","SAME");
	N = h58->Integral(); h58->Scale(4/N);

	ifile = new TFile("../root/h_61.root");
	itree = (TTree*)ifile->Get("t");
	itree->Draw("aa>>h61(200,0,700)","(nHits-20)<=1","SAME");
	N = h61->Integral(); h61->Scale(4/N);

	h53->SetLineColor(kGreen);
	h55->SetLineColor(kBlue);
	h56->SetLineColor(kMagenta);
	h57->SetLineColor(kRed);
	h58->SetLineColor(kOrange);
	h61->SetLineColor(kRed+2);

	TLegend * legend = new TLegend(0.3,0.5,0.85,0.85);
	legend->AddEntry(h53,"#53, set1 0.6 vol");
	legend->AddEntry(h55,"#55, set1 1.1 vol");
	legend->AddEntry(h56,"#56, set1 1.6 vol + set2 0.7 vol + set3 0.3 vol");
	legend->AddEntry(h57,"#57, set1 1.6 vol + set2 0.7 vol + set3 1.8 vol");
	legend->AddEntry(h58,"#58, set1 1.6 vol + set2 0.7 vol + set3 2.1 vol");
	legend->AddEntry(h61,"#61, set1 1.6 vol + set2 0.7 vol + set3 ? vol");
	legend->Draw("SAME");
}

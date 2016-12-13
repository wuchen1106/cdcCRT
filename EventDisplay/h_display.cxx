#include "TText.h"
#include "TEllipse.h"
#include "TLine.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TTree.h"
#include "TFile.h"
#include "TStyle.h"
#include <sstream>
#include <iostream>
#include <stdlib.h>

int main(int argc, char** argv){
	if (argc<2) return -1;
	int runNo = (int)strtol(argv[1],NULL,10);

	// For wire position
	TFile * TFile_wirepos = new TFile("../info/wire-position.root");
	TTree * TTree_wirepos = (TTree*) TFile_wirepos->Get("t");
	int wp_wid;
	int wp_lid;
	double wp_xro;
	double wp_yro;
	double wp_xhv;
	double wp_yhv;
	double wp_length;
	double map_xro[20][400];
	double map_yro[20][400];
	double map_zro[20][400];
	double map_xhv[20][400];
	double map_yhv[20][400];
	double map_zhv[20][400];
	TTree_wirepos->SetBranchAddress("l",&wp_lid);
	TTree_wirepos->SetBranchAddress("w",&wp_wid);
	TTree_wirepos->SetBranchAddress("xhv",&wp_xhv);
	TTree_wirepos->SetBranchAddress("yhv",&wp_yhv);
	TTree_wirepos->SetBranchAddress("xro",&wp_xro);
	TTree_wirepos->SetBranchAddress("yro",&wp_yro);
	TTree_wirepos->SetBranchAddress("Length",&wp_length);
	for (int i = 0; i<TTree_wirepos->GetEntries(); i++){
		TTree_wirepos->GetEntry(i);
		if (wp_lid>=1&&wp_lid<=18){
			map_xro[wp_lid][wp_wid] = wp_xro/10.;
			map_yro[wp_lid][wp_wid] = wp_yro/10.;
			map_zro[wp_lid][wp_wid] = wp_length/10./2;
			map_xhv[wp_lid][wp_wid] = wp_xhv/10.;
			map_yhv[wp_lid][wp_wid] = wp_yhv/10.;
			map_zhv[wp_lid][wp_wid] = -wp_length/10./2;
		}
	}
	double yup = 85;
	double ydown = -85;

	//===================Get ROOT File============================
	//TChain * c = new TChain("t","t");
	TChain * c = new TChain("t","t");
	std::stringstream buf;
	buf.str(""); buf.clear();
	buf<<"../root/h_"<<runNo<<".root";
	c->Add(buf.str().c_str());
	std::cout<<"Adding \""<<buf.str()<<"\""<<std::endl;
	Long64_t triggerNumber;
	std::vector<double> * i_driftT = 0;
	std::vector<int> * i_wireID = 0;
	std::vector<int> * i_layerID = 0;
	int nHits, nLayers;
	c->SetBranchAddress("nHits",&nHits);
	c->SetBranchAddress("nLayers",&nLayers);
	c->SetBranchAddress("wireID",&i_wireID);
	c->SetBranchAddress("layerID",&i_layerID);
	c->SetBranchAddress("driftT",&i_driftT);
	c->SetBranchAddress("triggerNumber",&triggerNumber);

	TEllipse * ewiret[20][400];
	double wx,wy,wz,dd,ddt;
	double wxro,wyro,wzro;
	double wxhv,wyhv,wzhv;
	int lid,wid;
	TCanvas * ca = new TCanvas("ca","ca",896,896);
	TH2D * h0 = new TH2D("h0","h0",512,-85,85,512,-85,85);
	gStyle->SetOptStat(0);
	TText * text[20][400];
	for (int lid = 0; lid<20; lid++){
		for (int wid = 0; wid<400; wid++){
			text[lid][wid] = 0;
			ewiret[lid][wid] = 0;
		}
	}

	h0->Draw();
	TEllipse * einner = new TEllipse(0,0,50,50);
	TEllipse * eouter = new TEllipse(0,0,82.6,82.6);
	einner->SetLineColor(kRed);
	eouter->SetLineColor(kRed);
	einner->SetFillStyle(0);
	eouter->SetFillStyle(0);
	einner->Draw("SAME");
	eouter->Draw("SAME");
//	TTree_wirepos->SetMarkerStyle(20);
//	TTree_wirepos->SetMarkerSize(0.5);
//	TTree_wirepos->Draw("yc/10.:xc/10.","","SAME");
	std::string suffix = ".pdf";
	for ( int i = 0 ; i<c->GetEntries(); i++){
//	for ( int i = 0 ; i<100; i++){
		if (i%100==0) printf("%lf%...\n",(double)i/c->GetEntries()*100);
		c->GetEntry(i);
		if (nLayers<10||nHits>20) continue;
		buf.str("");
		buf.clear();
		//FIXME
		buf.str("");
		buf.clear();
		buf<<"Entry#"<<i<<", TriggerNumber#"<<triggerNumber;
		h0->SetTitle(buf.str().c_str());
		// FIXME
//		if (iHit[83]<0) continue;
		int nHits = 0;
		for (int lid = 0; lid<20; lid++){
			for (int wid = 0; wid<400; wid++){
				if (ewiret[lid][wid]){
					delete ewiret[lid][wid];
					ewiret[lid][wid] = 0;
				}
				if (text[lid][wid]){
					delete text[lid][wid];
					text[lid][wid] = 0;
				}
			}
		}
		for (int ihit = 0; ihit<i_driftT->size(); ihit++){
			lid = (*i_layerID)[ihit];
			wid = (*i_wireID)[ihit];
			if (lid<0) continue;
			wxro = map_xro[lid][wid];
			wyro = map_yro[lid][wid];
			wxhv = map_xhv[lid][wid];
			wyhv = map_yhv[lid][wid];
		//	wy = (wyro+wyhv)/2.;
		//	wz = ((yup-wy)*zdown+(wy-ydown)*zup)/(yup-ydown);
		//	wx = ((wzro-wz)*wxhv+(wz-wzhv)*wxro)/(wzro-wzhv);
		//	wy = ((wzro-wz)*wyhv+(wz-wzhv)*wyro)/(wzro-wzhv);
			wx = wxro; // for this moment use the readout side
			wy = wyro; // for this moment use the readout side
			dd = (*i_driftT)[ihit]/560.; // for the moment use constant velocity;
			ewiret[lid][wid] = new TEllipse(wx,wy,dd,dd);
			ewiret[lid][wid]->SetFillStyle(0);
			ewiret[lid][wid]->SetLineColor(kRed);
			ewiret[lid][wid]->Draw("SAME"); // Draw hits.
//			text[lid][wid]= new TText(wx,wy,Form("%d,%d",lid,wid));
//			text[lid][wid]->SetTextSize(0.02);
//			text[lid][wid]->Draw("SAME");
		}
		buf.str("");
		buf.clear();
		//FIXME
//		buf<<i<<"_before.pdf";
		buf<<i<<suffix;
//		buf<<i<<"_after.pdf";
		ca->SaveAs(buf.str().c_str());
//		ca->WaitPrimitive();
	//	ca->Update();
//		while(1){}
	}
	return 0;
}

#include <stdlib.h>
#include <iostream>
#include <vector>

#include "TStyle.h"
#include "TTree.h"
#include "TAxis.h"
#include "TGaxis.h"
#include "TFile.h"
#include "TGraph.h"
#include "TLatex.h"
#include "TText.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TEllipse.h"

#define NCHS 48
#define NCHT 384 // shall be consistant with the value set in getP
#define NBD 8
#define NLY 20

int getGlobalChanID(int bid,int chid,int runid = 0);
int hbid2bid(int hbid,int runid = 0);

int main(int argc, char ** argv){
    int iRun = atoi(argv[1]);

	//===================Get wire position============================
	// For wire position
	TFile * TFile_wirepos = new TFile("../info/chanmap_20160814.root");
	TTree * TTree_wirepos = (TTree*) TFile_wirepos->Get("t");
	int wp_bid;
	int wp_wid;
	int wp_lid;
	int wp_ch;
	int map_wid[NCHT];
	int map_lid[NCHT];
	int widmax[NLY];
	for ( int il = 0; il<NLY; il++ ){
		widmax[il] = 0;
	}
	for ( int ch = 0; ch<NCHT; ch++ ){
		map_lid[ch] = -1;
		map_wid[ch] = -1;
	}
	TTree_wirepos->SetBranchAddress("LayerID",&wp_lid);
	TTree_wirepos->SetBranchAddress("BoardID",&wp_bid);
	TTree_wirepos->SetBranchAddress("ChanID",&wp_ch);
	TTree_wirepos->SetBranchAddress("CellID",&wp_wid);
	int gl_ch = 0;
	for (int i = 0; i<TTree_wirepos->GetEntries(); i++){
		TTree_wirepos->GetEntry(i);
		gl_ch = getGlobalChanID(wp_bid,wp_ch,iRun);
		if (gl_ch<0) continue; // not connected;
		map_wid[gl_ch] = wp_wid;
		map_lid[gl_ch] = wp_lid;
		if (widmax[wp_lid]<wp_wid) widmax[wp_lid] = wp_wid;
	}
	TFile_wirepos->Close();

    TFile * ifile = new TFile(Form("../root/run_%0.6d.root",iRun)); 
    TTree * itree = (TTree*)ifile->Get("tree");
    TFile * ifile2 = new TFile(Form("../root/p_%d.root",iRun)); 
    TTree * itree2 = (TTree*)ifile2->Get("t");
    int nhit[336];
    int clk[336][32];
    int tdc[336][32];
    int adc[336][32];
    int tri;
    double ped[384];
    itree->SetBranchAddress("triggerNumber",&tri);
    itree->SetBranchAddress("tdcNhit",&nhit);
    itree->SetBranchAddress("driftTime",tdc);
    itree->SetBranchAddress("adc",adc);
    itree->SetBranchAddress("clockNumberDriftTime",clk);
    itree2->SetBranchAddress("ped",ped);
    long int iEvent = atol(argv[2]);
    itree->GetEntry(iEvent);
    itree2->GetEntry(iEvent);
	ifile->Close();
	ifile2->Close();

    int index[32];
    double indexD[32];
    double adcScaled[32];
    for (int i = 0; i<32; i++){
        index[i] = i;
        indexD[i] = i;
    }
    for (int iBoard = 0; iBoard<7; iBoard++){
        TCanvas * c = new TCanvas("c","c",1440,960);
        gStyle->SetLineWidth(0.7);
        gStyle->SetLineScalePS(0.3);
        TLatex * text = new TLatex();
        text->SetTextSize(0.02);
        text->SetText(0.1,0.96,Form("Run %d, Entry %d, TriggerNumber %d",iRun,iEvent,tri));
        text->Draw("SAME");
        for (int iASD = 0; iASD<6; iASD++){
            for (int iCH = 0; iCH<8; iCH++){
                c->cd();
                TPad * p = new TPad("p","p",iCH*1./8,0.95*(1-(iASD+1)*1./6),(iCH+1)*1./8,0.95*(1-(iASD)*1./6));
                p->Draw();
                p->cd();
                int gCH = iBoard*48+iASD*8+iCH;
                TGraph * g = new TGraph(32,index,&(adc[gCH][0]));
                g->GetYaxis()->SetRangeUser(0,700);
                g->GetXaxis()->SetRangeUser(0,32);
                g->SetMarkerStyle(20);
                g->SetMarkerSize(0.25);
                g->SetLineWidth(0.7);
                g->SetTitle(Form("Brd %d, Chn %d, Lay %d, Cel %d",iBoard,iASD*8+iCH,map_lid[gCH],map_wid[gCH]));
                g->Draw("APL");
                double maxADC = 0;
                double minADC = 999;
                for (int iSample = 0; iSample<32; iSample++){
                    if (maxADC<adc[gCH][iSample]){
                        maxADC = adc[gCH][iSample];
                    }
                    if (minADC>adc[gCH][iSample]){
                        minADC = adc[gCH][iSample];
                    }
                }
                minADC--;
                maxADC++;
                for (int iSample = 0; iSample<32; iSample++){
                    adcScaled[iSample] = (adc[gCH][iSample]-minADC)*700/(maxADC-minADC);
                }
                TGraph * g2 = new TGraph(32,indexD,adcScaled);
                g2->SetLineWidth(0.7);
                g2->SetLineColor(kGray+1);
                g2->Draw("LSAME");
                TGaxis *axis = new TGaxis(32,0,32,700,minADC-ped[gCH],maxADC-ped[gCH],510,"+L");
//                axis->SetLabelColor(kGray+2);
                axis->Draw("SAME");
                for (int iHit = 0; iHit<nhit[gCH]; iHit++){
                    TEllipse * p = new TEllipse(clk[gCH][iHit],adc[gCH][clk[gCH][iHit]],0.3,6);
                    p->SetFillColor(kRed);
                    p->SetLineWidth(0.);
                    p->Draw("SAME");
                    TText *t = new TText(clk[gCH][iHit],adc[gCH][clk[gCH][iHit]]+6,Form("%d",tdc[gCH][iHit]));
                    t->SetTextColor(kRed);
                    t->SetTextSize(0.05);
                    t->Draw("SAME");
                }
            }
        }
        c->SaveAs(Form("%d.%d.pdf",iEvent,iBoard));
    }
    return 0;
}

int getGlobalChanID(int bid,int chid,int runid){
	int hbid = 0; // the index of board in real setup
	for (;hbid<NBD;hbid++){
		if (hbid2bid(hbid,runid)==bid) break;
	}

	if (hbid==NBD) return -1; // cannot find the index of this board
	return hbid*NCHS+chid;
}

int hbid2bid(int hbid, int runid){
	int bid = -1;
	if (runid<61){ // CDC, original setup
		if (hbid==1) bid = 95;
		else if (hbid==2) bid = 77;
		else if (hbid==3) bid = 59;
		else if (hbid==4) bid = 41;
		else if (hbid==5) bid = 50;
		else if (hbid==6) bid = 68;
		else if (hbid==7) bid = 86;
	}
	else if (runid<1000){ // CDC, new setup in 2016 Nov
		if (hbid==1) bid = 86;
		else if (hbid==2) bid = 68;
		else if (hbid==3) bid = 50;
		else if (hbid==4) bid = 32;
		else if (hbid==5) bid = 16;
		else if (hbid==6) bid = 0;
	}

	return bid;
}

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "TTree.h"
#include "TBranch.h"
#include "TFile.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TLine.h"
#include "TLatex.h"

#define NSAM 32
#define NCHS 48
#define NCHT 384 // shall be consistant with the value set in getP
#define NBD 8
#define NLY 20
//#define MIN_ADC 50
//#define MAX_ADC 750
#define MIN_ADC -50
#define MAX_ADC 750
#define MAX_ADCS 7500
#define NBINS  256

#define DEBUG

int power2_15 = pow(2,15);
double tscale = 0.96;

void print_usage(char* prog_name);
int getGlobalChanID(int bid,int chid,int runid = 0);
int hbid2bid(int hbid,int runid = 0);
int main(int argc, char** argv){
	if (argc<2){
		print_usage(argv[0]);
		return 1;
	}
	int runNo = (int)strtol(argv[1],NULL,10);
	int nEventMax = 0;
	if (argc>=3) nEventMax = (int)strtol(argv[2],NULL,10);
	std::string suffix = "";
	if (argc>=4){
		suffix  = argv[3];
		suffix=suffix+".";
	}

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
		gl_ch = getGlobalChanID(wp_bid,wp_ch,runNo);
		if (gl_ch<0) continue; // not connected;
		map_wid[gl_ch] = wp_wid;
		map_lid[gl_ch] = wp_lid;
		if (widmax[wp_lid]<wp_wid) widmax[wp_lid] = wp_wid;
	}

	//===================Get raw input ROOT file============================
	TChain * c_raw = new TChain("tree","tree");
	char inputName[384];
	sprintf(inputName,"../root/run_%0.6d.root",runNo);
	c_raw->Add(inputName);
	int adc[NCHT][NSAM];
	c_raw->SetBranchAddress("adc",adc);

	//===================Get peak input ROOT file============================
	TChain * c_peak = new TChain("t","t");
	sprintf(inputName,"../root/p_%d.root",runNo);
	c_peak->Add(inputName);
	std::vector<std::vector<int> > * i_tdc = 0;
	std::vector<std::vector<int> > * i_peak = 0;
	std::vector<std::vector<int> > * i_clk = 0;
	std::vector<std::vector<int> > * i_width = 0;
	std::vector<std::vector<double> > * i_sum = 0;
	int i_np[NCHT][50];
	int i_nh[50];
	double i_ped[NCHT];
	double i_aa[NCHT];
	int triggerNumberMax = 0;
	int triggerNumber;
	c_peak->SetBranchAddress("triggerNumber",&triggerNumber);
	c_peak->SetBranchAddress("nh",i_nh);
	c_peak->SetBranchAddress("ped",i_ped);
	c_peak->SetBranchAddress("aa",i_aa);
	c_peak->SetBranchAddress("np",i_np);
	c_peak->SetBranchAddress("clk",&i_clk);
	c_peak->SetBranchAddress("sum",&i_sum);
	c_peak->SetBranchAddress("tdc",&i_tdc);
	c_peak->SetBranchAddress("peak",&i_peak);
	c_peak->SetBranchAddress("width",&i_width);

	//===================Get run info============================
	TFile * if_run = new TFile("../info/run-info.root");
	TTree * t_run = (TTree*) if_run->Get("t");
	int i_runNo, gasID, runGr, HV, THR;
	char runDu[384];
	double t00, t01, t02, t03, t04, t05, t06, t07, aacut0, sumcut0, aacut, sumcut;
	t_run->SetBranchAddress("run_number",&i_runNo);
	t_run->SetBranchAddress("gas_mixture_id",&gasID);
	t_run->SetBranchAddress("hv",&HV);
	t_run->SetBranchAddress("thr",&THR);
	t_run->SetBranchAddress("duration",&runDu);
	t_run->SetBranchAddress("run_grade",&runGr);
	t_run->SetBranchAddress("t00",&t00);
	t_run->SetBranchAddress("t01",&t01);
	t_run->SetBranchAddress("t02",&t02);
	t_run->SetBranchAddress("t03",&t03);
	t_run->SetBranchAddress("t04",&t04);
	t_run->SetBranchAddress("t05",&t05);
	t_run->SetBranchAddress("t06",&t06);
	t_run->SetBranchAddress("t07",&t07);
	t_run->SetBranchAddress("aa0",&aacut0);
	t_run->SetBranchAddress("sum0",&sumcut0);
	t_run->SetBranchAddress("aa",&aacut);
	t_run->SetBranchAddress("sum",&sumcut);
	for(int i = 0; i<t_run->GetEntries(); i++){
		t_run->GetEntry(i);
		if (i_runNo == runNo) break;
	}
	double npair = 17.96;
	TString gastype = "He:C_{2}H_{4}(50:50)";
	if (gasID==1){
		gastype = "He:iC_{4}H_{10}(90:10)";
		npair = 27.96;
	}
	else if (gasID==2){
		gastype = "He:CH_{4}(80:20)";
		npair = 56.10;
	}
	TString duration = runDu;
	const char *sep = ":";
	char * durationSep = strtok(runDu,sep);
	double durationTime = 0;
	double timeunit = 3600;
	while(durationSep){
		durationTime += timeunit*strtol(durationSep,NULL,10);
		timeunit/=60;
		durationSep = strtok(NULL,sep);
	}
	double t0[NBD];
	t0[0] = t00;
	t0[1] = t01;
	t0[2] = t02;
	t0[3] = t03;
	t0[4] = t04;
	t0[5] = t05;
	t0[6] = t06;
	t0[7] = t07;
	std::cout<<"runNo#"<<runNo<<": "<<gastype<<", "<<runGr<<", "<<duration<<", "<<HV<<" V, "<<durationTime<<"sec"<<std::endl;

	//===================Prepare output ROOT file============================
	char outputName[384];
	int o_nHits; // number of hits in this event
	int o_nLayers; // number of hitten layers in this event
	std::vector<int> * o_layerID = 0;
	std::vector<int> * o_wireID = 0;
	std::vector<int> * o_type = 0; // cell type: 0, normal cell; -2, dummy layer cell; -1, guard layer (HV/2) cell; 1, boardary cell;
	std::vector<int> * o_np = 0; // The number of peaks in this hit
	std::vector<int> * o_ip = 0; // The index of the peak chosen as main peak in this hit
	std::vector<int> * o_clk = 0; // The index of the ADC sample corresponding to the TDC tag of the chosen peak.
	std::vector<int> * o_width = 0; // width of the chosen peak
	std::vector<int> * o_peak = 0; // ADC height (minus pedestal) of this chosen peak
	std::vector<int> * o_ped = 0; // pedestal
	std::vector<double> * o_sum = 0; // ADC sum (minus pedestal) of this chosen peak
	std::vector<double> * o_aa = 0; // ADC sum (minus pedestal) of all peaks in this hit
	std::vector<double> * o_driftT = 0; // drift time of this hit
	sprintf(outputName,("../root/h_%d."+suffix+"root").c_str(),runNo);
	TFile * f = new TFile(outputName,"RECREATE");
	TTree * t = new TTree("t","t");
	t->Branch("triggerNumber",&triggerNumber);
	t->Branch("nHits",&o_nHits);
	t->Branch("nLayers",&o_nLayers);
	t->Branch("layerID",&o_layerID);
	t->Branch("wireID",&o_wireID);
	t->Branch("type",&o_type);
	t->Branch("np",&o_np);
	t->Branch("ip",&o_ip);
	t->Branch("clk",&o_clk);
	t->Branch("width",&o_width);
	t->Branch("peak",&o_peak);
	t->Branch("ped",&o_ped);
	t->Branch("sum",&o_sum);
	t->Branch("aa",&o_aa);
	t->Branch("driftT",&o_driftT);

	//===================Prepare Histograms============================
	TCanvas * canvas;
	TPad * pad[9];
	gStyle->SetPalette(1);
	gStyle->SetOptStat(0);
	gStyle->SetPadTickX(1);
	gStyle->SetPadTickY(1);

	TLine *line_ht[NCHS];
	TLatex *text_ht[NCHS];
	TLine *line_ht2[NCHS];
	TLatex *text_ht2[NCHS];
	for (Int_t i=0; i<NCHS; i++) {
		line_ht[i] = new TLine();
		line_ht[i]->SetLineColor(kRed);
		line_ht[i]->SetLineWidth(0.5);
		text_ht[i] = new TLatex(0,0,"");
		text_ht[i]->SetTextSize(0.04);
		text_ht[i]->SetTextColor(kRed);
		line_ht2[i] = new TLine();
		line_ht2[i]->SetLineColor(kBlack);
		line_ht2[i]->SetLineWidth(0.5);
		text_ht2[i] = new TLatex(0,0,"");
		text_ht2[i]->SetTextSize(0.04);
		text_ht2[i]->SetTextColor(kBlack);
	}
	TLatex * text = new TLatex();
	text->SetTextSize(0.02);
	TString name;
	TString title;

	TH2D* hwf[NCHT];
	TH2D* hst[NCHT];
	TH2D* hat[NCHT];
	TH1D* htdc[NCHT];
	TH1D* ha[NCHT];
	TH1D* hs[NCHT];

	TH2D* hbwf[NBD];
	TH2D* hbst[NBD];
	TH2D* hbat[NBD];
	TH1D* hbtdc[NBD];
	TH1D* hba[NBD];
	TH1D* hbs[NBD];

	for (int i = 0; i<NCHT; i++){
		if (map_lid[i]==-1){
			title = Form("ch#%d",i%NCHS);
		}
		else{
			title = Form("ch#%d, layer#%d, wire#%d",i%NCHS,map_lid[i],map_wid[i]);
		}
		//hst
		name = Form("hst_%d",i);
		hst[i] = new TH2D(name,title,NBINS,-1100,0,NBINS,MIN_ADC,MAX_ADCS);
		hst[i]->GetXaxis()->SetTitle("TDC");
		hst[i]->GetYaxis()->SetTitle("ADC (-pedestal) Sum (of Single Peak) [ADC]");
		//hat
		name = Form("hat_%d",i);
		hat[i] = new TH2D(name,title,NBINS,-1100,0,NBINS,MIN_ADC,MAX_ADCS);
		hat[i]->GetXaxis()->SetTitle("TDC");
		hat[i]->GetYaxis()->SetTitle("ADC (-pedestal) Sum (of All Peaks) [ADC]");
		//wf
		name = Form("hwf_%d",i);
		hwf[i] = new TH2D(name,title,NSAM+10,-10,NSAM,MAX_ADC-MIN_ADC,MIN_ADC,MAX_ADC);
		hwf[i]->GetXaxis()->SetTitle("SampleID - SampleID_{first hit}");
		hwf[i]->GetYaxis()->SetTitle("ADC");
		//tdc
		name = Form("htdc_%d",i);
		htdc[i] = new TH1D(name,title,256,-1100,0);
		htdc[i]->GetXaxis()->SetTitle("TDC");
		//adc sum
		name = Form("has_%d",i);
		ha[i] = new TH1D(name,title,1000,MIN_ADC,MAX_ADCS);
		ha[i]->GetXaxis()->SetTitle("ADC (-pedestal) Sum (of All Peaks) [ADC]");
		//adc peak
		name = Form("hap_%d",i);
		hs[i] = new TH1D(name,title,(MAX_ADCS-MIN_ADC)/2,MIN_ADC,MAX_ADCS);
		hs[i]->GetXaxis()->SetTitle("ADC (-pedestal) Sum (of Single Peak) [ADC]");

		if (map_lid[i]==-1){
			hst[i]->GetXaxis()->SetAxisColor(kRed);
			hst[i]->GetYaxis()->SetAxisColor(kRed);
			hst[i]->GetXaxis()->SetLabelColor(kRed);
			hst[i]->GetYaxis()->SetLabelColor(kRed);

			hat[i]->GetXaxis()->SetAxisColor(kRed);
			hat[i]->GetYaxis()->SetAxisColor(kRed);
			hat[i]->GetXaxis()->SetLabelColor(kRed);
			hat[i]->GetYaxis()->SetLabelColor(kRed);

			hwf[i]->GetXaxis()->SetAxisColor(kRed);
			hwf[i]->GetYaxis()->SetAxisColor(kRed);
			hwf[i]->GetXaxis()->SetLabelColor(kRed);
			hwf[i]->GetYaxis()->SetLabelColor(kRed);

			htdc[i]->GetXaxis()->SetAxisColor(kRed);
			htdc[i]->GetYaxis()->SetAxisColor(kRed);
			htdc[i]->GetXaxis()->SetLabelColor(kRed);
			htdc[i]->GetYaxis()->SetLabelColor(kRed);

			ha[i]->GetXaxis()->SetAxisColor(kRed);
			ha[i]->GetYaxis()->SetAxisColor(kRed);
			ha[i]->GetXaxis()->SetLabelColor(kRed);
			ha[i]->GetYaxis()->SetLabelColor(kRed);

			hs[i]->GetXaxis()->SetAxisColor(kRed);
			hs[i]->GetYaxis()->SetAxisColor(kRed);
			hs[i]->GetXaxis()->SetLabelColor(kRed);
			hs[i]->GetYaxis()->SetLabelColor(kRed);
		}
		else if (map_lid[i]==0){
			hst[i]->GetXaxis()->SetAxisColor(kGreen);
			hst[i]->GetYaxis()->SetAxisColor(kGreen);
			hst[i]->GetXaxis()->SetLabelColor(kGreen);
			hst[i]->GetYaxis()->SetLabelColor(kGreen);

			hat[i]->GetXaxis()->SetAxisColor(kGreen);
			hat[i]->GetYaxis()->SetAxisColor(kGreen);
			hat[i]->GetXaxis()->SetLabelColor(kGreen);
			hat[i]->GetYaxis()->SetLabelColor(kGreen);

			hwf[i]->GetXaxis()->SetAxisColor(kGreen);
			hwf[i]->GetYaxis()->SetAxisColor(kGreen);
			hwf[i]->GetXaxis()->SetLabelColor(kGreen);
			hwf[i]->GetYaxis()->SetLabelColor(kGreen);

			htdc[i]->GetXaxis()->SetAxisColor(kGreen);
			htdc[i]->GetYaxis()->SetAxisColor(kGreen);
			htdc[i]->GetXaxis()->SetLabelColor(kGreen);
			htdc[i]->GetYaxis()->SetLabelColor(kGreen);

			ha[i]->GetXaxis()->SetAxisColor(kGreen);
			ha[i]->GetYaxis()->SetAxisColor(kGreen);
			ha[i]->GetXaxis()->SetLabelColor(kGreen);
			ha[i]->GetYaxis()->SetLabelColor(kGreen);

			hs[i]->GetXaxis()->SetAxisColor(kGreen);
			hs[i]->GetYaxis()->SetAxisColor(kGreen);
			hs[i]->GetXaxis()->SetLabelColor(kGreen);
			hs[i]->GetYaxis()->SetLabelColor(kGreen);
		}
	}

	for (int i = 0; i<NBD; i++){
		title = Form("board#%d",i);
		//hbst
		name = Form("hbst_%d",i);
		hbst[i] = new TH2D(name,title,NBINS,-1100,0,NBINS,MIN_ADC,MAX_ADCS);
		hbst[i]->GetXaxis()->SetTitle("TDC");
		hbst[i]->GetYaxis()->SetTitle("ADC (-pedestal) Sum (of Single Peak) [ADC]");
		//hbat
		name = Form("hbat_%d",i);
		hbat[i] = new TH2D(name,title,NBINS,-1100,0,NBINS,MIN_ADC,MAX_ADCS);
		hbat[i]->GetXaxis()->SetTitle("TDC");
		hbat[i]->GetYaxis()->SetTitle("ADC (-pedestal) Sum (of All Peaks) [ADC]");
		//wf
		name = Form("hbwf_%d",i);
		hbwf[i] = new TH2D(name,title,NSAM+10,-10,NSAM,MAX_ADC-MIN_ADC,MIN_ADC,MAX_ADC);
		hbwf[i]->GetXaxis()->SetTitle("SampleID - SampleID_{first hit}");
		hbwf[i]->GetYaxis()->SetTitle("ADC");
		//tdc
		name = Form("hbtdc_%d",i);
		hbtdc[i] = new TH1D(name,title,256,-1100,0);
		hbtdc[i]->GetXaxis()->SetTitle("TDC");
		//adc sum
		name = Form("hbas_%d",i);
		hba[i] = new TH1D(name,title,1000,MIN_ADC,MAX_ADCS);
		hba[i]->GetXaxis()->SetTitle("ADC (-pedestal) Sum (of All Peaks) [ADC]");
		//adc peak
		name = Form("hbap_%d",i);
		hbs[i] = new TH1D(name,title,(MAX_ADCS-MIN_ADC)/2,MIN_ADC,MAX_ADCS);
		hbs[i]->GetXaxis()->SetTitle("ADC (-pedestal) Sum (of Single Peak) [ADC]");

		hbst[i]->GetXaxis()->SetAxisColor(kGreen);
		hbst[i]->GetYaxis()->SetAxisColor(kGreen);
		hbst[i]->GetXaxis()->SetLabelColor(kGreen);
		hbst[i]->GetYaxis()->SetLabelColor(kGreen);

		hbat[i]->GetXaxis()->SetAxisColor(kGreen);
		hbat[i]->GetYaxis()->SetAxisColor(kGreen);
		hbat[i]->GetXaxis()->SetLabelColor(kGreen);
		hbat[i]->GetYaxis()->SetLabelColor(kGreen);

		hbwf[i]->GetXaxis()->SetAxisColor(kGreen);
		hbwf[i]->GetYaxis()->SetAxisColor(kGreen);
		hbwf[i]->GetXaxis()->SetLabelColor(kGreen);
		hbwf[i]->GetYaxis()->SetLabelColor(kGreen);

		hbtdc[i]->GetXaxis()->SetAxisColor(kGreen);
		hbtdc[i]->GetYaxis()->SetAxisColor(kGreen);
		hbtdc[i]->GetXaxis()->SetLabelColor(kGreen);
		hbtdc[i]->GetYaxis()->SetLabelColor(kGreen);

		hba[i]->GetXaxis()->SetAxisColor(kGreen);
		hba[i]->GetYaxis()->SetAxisColor(kGreen);
		hba[i]->GetXaxis()->SetLabelColor(kGreen);
		hba[i]->GetYaxis()->SetLabelColor(kGreen);

		hbs[i]->GetXaxis()->SetAxisColor(kGreen);
		hbs[i]->GetYaxis()->SetAxisColor(kGreen);
		hbs[i]->GetXaxis()->SetLabelColor(kGreen);
		hbs[i]->GetYaxis()->SetLabelColor(kGreen);
	}

	// Loop in events
	Long64_t N = c_raw->GetEntries();
	if (nEventMax&&nEventMax<N) N = nEventMax;
	std::cout<<"Processing "<<N<<" events..."<<std::endl;
	int tdcNhitwire;
	bool layerhit[NLY];
	int ip = -1;
	double avped[NCHT];
	for ( int ch = 0; ch<NCHT; ch++ ){
		avped[ch] = 0;
	}
	//N=1;
	for (Long64_t i = 0;i<N; i++){
		if (i%1000==0) std::cout<<(double)i/N*100<<"%..."<<std::endl;
		c_peak->GetEntry(i);
		c_raw->GetEntry(i);
		if (triggerNumberMax<triggerNumber) triggerNumberMax = triggerNumber;
		o_nHits = 0;
		o_nLayers = 0;
		if(o_layerID==0) delete o_layerID; o_layerID= new std::vector<int>;
		if(o_wireID==0) delete o_wireID; o_wireID = new std::vector<int>;
		if(o_type==0) delete o_type; o_type = new std::vector<int>;
		if(o_ip==0) delete o_ip; o_ip = new std::vector<int>;
		if(o_np==0) delete o_np; o_np = new std::vector<int>;
		if(o_clk==0) delete o_clk; o_clk = new std::vector<int>;
		if(o_width==0) delete o_width; o_width = new std::vector<int>;
		if(o_peak==0) delete o_peak; o_peak = new std::vector<int>;
		if(o_ped==0) delete o_ped; o_ped = new std::vector<int>;
		if(o_sum==0) delete o_sum; o_sum = new std::vector<double>;
		if(o_aa==0) delete o_aa; o_aa = new std::vector<double>;
		if(o_driftT==0) delete o_driftT; o_driftT = new std::vector<double>;
		for (int il = 0; il < NLY; il++){
			layerhit[il] = false;
		}
		for(int ch = 0; ch<NCHT; ch++){
			int brd = ch/NCHS;
			tdcNhitwire = i_np[ch][0];
			if (!isnan(i_ped[ch])&&!isinf(i_ped[ch]))
				avped[ch]+= i_ped[ch];

			// Get the peak;
			int thepeak = 0;
			for(;thepeak<tdcNhitwire; thepeak++){
				if ((*i_sum)[ch][thepeak]>=sumcut) break;
			}
			if (thepeak==tdcNhitwire){// didn't find the peak, choose the first one;
				thepeak=0;
				ip = -1;
			}
			else{
				ip = thepeak;
			}
			if (tdcNhitwire==0){
				thepeak = -1;
				continue;
			}

			// Fill histograms
			if (i_aa[ch]){
				ha[ch]->Fill(i_aa[ch]);
				hba[brd]->Fill(i_aa[ch]);
			}
			int offset;
			if (thepeak<0) offset = 0;
			else offset = (*i_clk)[ch][thepeak];
			for ( int clk = 0; clk<NSAM; clk++ ){
				hwf[ch]->Fill(clk-offset,adc[ch][clk]);
				hbwf[brd]->Fill(clk-offset,adc[ch][clk]);
			}
			if (thepeak>=0){
				hs[ch]->Fill((*i_sum)[ch][0]);
				hst[ch]->Fill((*i_tdc)[ch][thepeak],(*i_sum)[ch][thepeak]);
				hat[ch]->Fill((*i_tdc)[ch][thepeak],i_aa[ch]);
				htdc[ch]->Fill((*i_tdc)[ch][thepeak]);
				hbs[brd]->Fill((*i_sum)[ch][0]);
				hbst[brd]->Fill((*i_tdc)[ch][thepeak],(*i_sum)[ch][thepeak]);
				hbat[brd]->Fill((*i_tdc)[ch][thepeak],i_aa[ch]);
				hbtdc[brd]->Fill((*i_tdc)[ch][thepeak]);
			}

			// cut and output the tree
			if (i_aa[ch]<aacut) continue;
			if (map_lid[ch]<0) continue;
			int bid = ch/NCHS;
			/*
			if (map_lid[ch]==0) o_type->push_back(-1);
			else if (map_lid[ch]==NLY-1) o_type->push_back(1);
			else{
				if (map_wid[ch]==0)  o_type->push_back(2);
				else if (map_wid[ch]==widmax[ch])  o_type->push_back(3);
				else{
					o_type->push_back(0);
					layerhit[map_lid[ch]] = true;
					o_nHits++;
				}
			}
			*/
			o_type->push_back(0); layerhit[map_lid[ch]] = true; o_nHits++; // currently all cells are normal cells.
			o_ip->push_back(ip);
			o_driftT->push_back(((*i_tdc)[ch][thepeak]-t0[bid])/tscale);
			o_layerID->push_back(map_lid[ch]);
			o_wireID->push_back(map_wid[ch]);
			o_np->push_back(tdcNhitwire);
			o_clk->push_back((*i_clk)[ch][thepeak]);
			o_width->push_back((*i_width)[ch][thepeak]);
			o_peak->push_back((*i_peak)[ch][thepeak]);
			o_ped->push_back((i_ped)[ch]);
			o_sum->push_back((*i_sum)[ch][thepeak]);
			o_aa->push_back(i_aa[ch]);
		}
		for (int il = 0; il < NLY; il++ ){
			if (layerhit[il]) o_nLayers++;
		}
		t->Fill();
	}
	t->Write();
	for ( int ch = 0; ch<NCHT; ch++ ){
		avped[ch] = avped[ch]/N;
	}

	// Prepare canvas
	canvas = new TCanvas("c","c",1024,768);
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			int index = j*3+i;
			if (index==8) continue;
			pad[index] = new TPad(Form("p%d_%d",i,j),Form("p%d_%d",i,j),1./3*i,0.95/3*(2-j),1./3*(i+1),0.95/3*(3-j));
			pad[index]->Draw();
			pad[index]->SetGridx(1);
			pad[index]->SetGridy(1);
		}
	}

	// run summary
	TLatex * text2 = new TLatex();
	text2->SetTextSize(0.02);
//	text2->SetText(0.1,0.96,Form("run#%d ",runNo)+gastype+Form(", %d V,%d mV, Grade#%d",HV,THR,runGr)+", "+duration+Form(", %d events, Eff_{daq} = %2.2lf%%, Rate_{tri} = %1.1lfkHz",N,((double)N)/(triggerNumberMax+1)*100,(triggerNumberMax+1)/durationTime/1000));
	text2->SetText(0.1,0.96,Form("run#%d ",runNo)+gastype+Form(", %d V, Grade#%d",HV,runGr)+", "+duration+Form(", %d events, Eff_{daq} = %2.2lf%%, Rate_{tri} = %1.1lfkHz",N,((double)N)/(triggerNumberMax+1)*100,(triggerNumberMax+1)/durationTime/1000));

	// Draw sum VS dt
	canvas->cd();
	text->SetText(0.1,0.98,Form("ADC (-pedestal) Sum (of Single Peak) VS TDC"));
	text->Draw("SAME");
	text2->Draw("SAME");
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			int index = j*3+i;
			if (index==8) continue;
			pad[index]->cd();
			hbst[index]->Draw("COLZ");
			line_ht[index]->SetX1(-1000);
			line_ht[index]->SetY1(sumcut);
			line_ht[index]->SetX2(0);
			line_ht[index]->SetY2(sumcut);
			line_ht[index]->Draw("SAME");
			text_ht[index]->SetText(-100,sumcut+5,Form("%.2lf",sumcut));
			text_ht[index]->Draw("SAME");
		}
	}
	canvas->SaveAs(Form("run%d.st.pdf",runNo));
	canvas->SaveAs(Form("run%d.st.png",runNo));

	// Draw aa VS dt
	canvas->cd();
	text->SetText(0.1,0.98,"ADC (-pedestal) Sum (of All Peaks) VS TDC (Board #0)");
	text->Draw("SAME");
	text2->Draw("SAME");
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			int index = j*3+i;
			if (index==8) continue;
			pad[index]->cd();
			hbat[index]->Draw("COLZ");
			line_ht[index]->SetX1(-1000);
			line_ht[index]->SetY1(aacut);
			line_ht[index]->SetX2(0);
			line_ht[index]->SetY2(aacut);
			line_ht[index]->Draw("SAME");
			text_ht[index]->SetText(-100,aacut+5,Form("%.2lf",aacut));
			text_ht[index]->Draw("SAME");
		}
	}
	canvas->SaveAs(Form("run%d.at.pdf",runNo));
	canvas->SaveAs(Form("run%d.at.png",runNo));

	// Draw waveform
	canvas->cd();
	text->SetText(0.1,0.98,"Waveform (Board #0)");
	text->Draw("SAME");
	text2->Draw("SAME");
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			int index = j*3+i;
			if (index==8) continue;
			pad[index]->cd();
			pad[index]->SetLogz(1);
			hbwf[index]->Draw("COLZ");
			double ped = avped[index];
			line_ht2[index]->SetX1(-10);
			line_ht2[index]->SetY1(ped);
			line_ht2[index]->SetX2(32);
			line_ht2[index]->SetY2(ped);
			line_ht2[index]->Draw("SAME");
			text_ht2[index]->SetText(25,ped+5,Form("%.2lf",ped));
			text_ht2[index]->Draw("SAME");
		}
	}
	canvas->SaveAs(Form("run%d.wf.pdf",runNo));
	canvas->SaveAs(Form("run%d.wf.png",runNo));

	// Draw TDC
	canvas->cd();
	text->SetText(0.1,0.98,"TDC (Board #0)");
	text->Draw("SAME");
	text2->Draw("SAME");
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			int index = j*3+i;
			if (index==8) continue;
			pad[index]->cd();
			hbtdc[index]->Draw("");
			int bd = index/48;
			line_ht[index]->SetX1(t0[bd]);
			line_ht[index]->SetY1(0);
			line_ht[index]->SetX2(t0[bd]);
			int max = hbtdc[index]->GetBinContent(hbtdc[index]->GetMaximumBin());
			line_ht[index]->SetY2(max);
			line_ht[index]->Draw("SAME");
			text_ht[index]->SetText(t0[bd],max*0.1,Form("%.2lf",t0[bd]));
			text_ht[index]->Draw("SAME");
		}
	}
	canvas->SaveAs(Form("run%d.tdc.pdf",runNo));
	canvas->SaveAs(Form("run%d.tdc.png",runNo));

	// Draw ADC (-pedestal) Sum (of Single Peak)
	canvas->cd();
	text->SetText(0.1,0.98,"ADC (-pedestal) Sum (of Single Peak) (Board #0)");
	text->Draw("SAME");
	text2->Draw("SAME");
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			int index = j*3+i;
			if (index==8) continue;
			pad[index]->cd();
			pad[index]->SetLogy();
			hbs[index]->Draw("");
			line_ht[index]->SetX1(sumcut);
			line_ht[index]->SetY1(0);
			line_ht[index]->SetX2(sumcut);
			int max = hbs[index]->GetBinContent(hbs[index]->GetMaximumBin());
			line_ht[index]->SetY2(max);
			line_ht[index]->Draw("SAME");
			text_ht[index]->SetText(sumcut,max*0.2,Form("%.2lf",sumcut));
			text_ht[index]->Draw("SAME");
		}
	}
	canvas->SaveAs(Form("run%d.sum.pdf",runNo));
	canvas->SaveAs(Form("run%d.sum.png",runNo));

	// Draw ADC sum
	canvas->cd();
	text->SetText(0.1,0.98,"ADC (-pedestal) Sum (of All Peaks) (Board #0)");
	text->Draw("SAME");
	text2->Draw("SAME");
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			int index = j*3+i;
			if (index==8) continue;
			pad[index]->cd();
			pad[index]->SetLogy();
			hba[index]->Draw("");
			line_ht[index]->SetX1(aacut);
			line_ht[index]->SetY1(0);
			line_ht[index]->SetX2(aacut);
			int max = hba[index]->GetBinContent(hba[index]->GetMaximumBin());
			line_ht[index]->SetY2(max);
			line_ht[index]->Draw("SAME");
			text_ht[index]->SetText(aacut,max*0.2,Form("%.2lf",aacut));
			text_ht[index]->Draw("SAME");
		}
	}
	canvas->SaveAs(Form("run%d.aa.pdf",runNo));
	canvas->SaveAs(Form("run%d.aa.png",runNo));

	for (int i = 0; i<NCHT; i++) hst[i]->Write();;
	for (int i = 0; i<NCHT; i++) hat[i]->Write();;
	for (int i = 0; i<NCHT; i++) hwf[i]->Write();;
	for (int i = 0; i<NCHT; i++) htdc[i]->Write();;
	for (int i = 0; i<NCHT; i++) ha[i]->Write();;
	for (int i = 0; i<NCHT; i++) hs[i]->Write();;
	for (int i = 0; i<NBD; i++) hbst[i]->Write();;
	for (int i = 0; i<NBD; i++) hbat[i]->Write();;
	for (int i = 0; i<NBD; i++) hbwf[i]->Write();;
	for (int i = 0; i<NBD; i++) hbtdc[i]->Write();;
	for (int i = 0; i<NBD; i++) hba[i]->Write();;
	for (int i = 0; i<NBD; i++) hbs[i]->Write();;

	return 0;
}

void print_usage(char* prog_name)
{
	fprintf(stderr,"\t%s [runNo] <[nEventMax]>\n",prog_name);
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

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
#include "TROOT.h"

#define NSAM 32
#define NCHS 48
#define NCHT 384
#define NBD 8
#define MIN_ADC 50
#define MAX_ADC 750
#define NBINS  256

#define DEBUG

int power2_15 = pow(2,15);

void print_usage(char* prog_name);
int main(int argc, char** argv){
	if (argc<2){
		print_usage(argv[0]);
		return 1;
	}
	int runNo = (int)strtol(argv[1],NULL,10);
	int nEventMax = 0;
	if (argc>=3) nEventMax = (int)strtol(argv[2],NULL,10);

	//===================Get input ROOT file============================
	TChain * c = new TChain("tree","tree");
	char inputName[384];
	sprintf(inputName,"../root/run_%0.6d.root",runNo);
	c->Add(inputName);
	int tdcNhit[NCHT]; // number of TDC tags of each global channel
	int clockNumberDriftTime[NCHT][NSAM]; // The corresponding ADC index of each TDC tag in each global channel
	int adc[NCHT][NSAM]; // ADC samples in each channel
	int driftTime[NCHT][NSAM]; // TDC tags of each global channel
	int triggerNumber; // trigger number given by FCT (emmulator) of this event
	int triggerNumberMax = 0;
	c->SetBranchAddress("triggerNumber",&triggerNumber);
	c->SetBranchAddress("tdcNhit",tdcNhit);
	c->SetBranchAddress("clockNumberDriftTime",clockNumberDriftTime);
	c->SetBranchAddress("driftTime",driftTime);
	c->SetBranchAddress("adc",adc);

	//===================Prepare output ROOT file============================
	gROOT->ProcessLine(".L loader.C+");
	char outputName[384];
	std::vector<std::vector<int> > * o_tdc = 0; // TDC tags of each channel
	std::vector<std::vector<int> > * o_peak = 0; // ADC height (minus pedestal) of peaks in each channel
	std::vector<std::vector<int> > * o_clk = 0; // ADC index of each TDC tag in each channel
	std::vector<std::vector<int> > * o_width = 0; // width of peaks in each channel
	std::vector<std::vector<double> > * o_sum = 0; // ADC sum (minus pedestal) of peaks in each channel
	int o_nPeaks[NCHT][50]; // Number of peaks in each channel
	int o_nHits[50]; // Number of hits in this events with at least Iteration$ peaks
	double o_pedestal[NCHT]; // Pedestal of each channel
	double o_areaall[NCHT]; // ADC sum (minus pedestal) of all peaks in each channel
	TFile * f = new TFile(Form("../root/p_%d.root",runNo),"RECREATE");
	TTree * t = new TTree("t","t");
	t->Branch("triggerNumber",&triggerNumber);
	t->Branch("nh",&o_nHits,"nh[50]/I");
	t->Branch("ped",o_pedestal,Form("ped[%d]/D",NCHT));
	t->Branch("aa",o_areaall,Form("aa[%d]/D",NCHT));
	t->Branch("np",o_nPeaks,Form("np[%d][50]/I",NCHT));
	t->Branch("clk",&o_clk);
	t->Branch("sum",&o_sum);
	t->Branch("tdc",&o_tdc);
	t->Branch("peak",&o_peak);
	t->Branch("width",&o_width);
//	t->Branch("adc",adc,Form("aa[%d][%d]/I",NCHT,NSAM));
	double prepedestal[NCHT];
	for (int ch = 0; ch<NCHT; ch++){
		prepedestal[ch] = 210;
	}

	// Loop in events
	Long64_t N = c->GetEntries();
	if (nEventMax&&nEventMax<N) N = nEventMax;
	std::cout<<"Processing "<<N<<" events..."<<std::endl;
	for (Long64_t i = 0;i<N; i++){
		//FIXME
		if (i%1000==0) std::cout<<(double)i/N*100<<"%..."<<std::endl;
		c->GetEntry(i);

		for(int iH = 0; iH<50; iH++){
			o_nHits[iH] = 0;
		}
		// prepare vectors
		if (o_clk) delete o_clk; o_clk = new std::vector<std::vector<int> >;
		if (o_tdc) delete o_tdc; o_tdc = new std::vector<std::vector<int> >;
		if (o_sum) delete o_sum; o_sum = new std::vector<std::vector<double> >;
		if (o_peak) delete o_peak; o_peak = new std::vector<std::vector<int> >;
		if (o_width) delete o_width; o_width = new std::vector<std::vector<int> >;
		for(int ch = 0; ch<NCHT; ch++){
			for ( int iH = 0; iH<50; iH++ ){
				o_nPeaks[ch][iH] = 0;
			}
			// reset
			int tdcNhitwire = tdcNhit[ch];
			o_pedestal[ch]=0;
			o_areaall[ch]=0;
			std::vector<int> temp_clk;
			std::vector<int> temp_tdc;
			std::vector<double> temp_sum;
			std::vector<int> temp_peak;
			std::vector<int> temp_width;
			temp_clk.resize(tdcNhitwire);
			temp_tdc.resize(tdcNhitwire);
			temp_sum.resize(tdcNhitwire);
			temp_peak.resize(tdcNhitwire);
			temp_width.resize(tdcNhitwire);

			// Get height and etc
			int clk;
			for(clk = 0; clk<(tdcNhitwire<=0?NSAM:clockNumberDriftTime[ch][0]-1); clk++){
				o_pedestal[ch]+=adc[ch][clk];
			}
			if (clk==0){
				o_pedestal[ch] = prepedestal[ch];
			}
			else {
				o_pedestal[ch]/=clk;
				prepedestal[ch] = o_pedestal[ch];
			}
			for ( int ihit = 0; ihit<tdcNhitwire; ihit++){
				if (driftTime[ch][ihit]>0) driftTime[ch][ihit]-=power2_15;
				temp_tdc[ihit] = driftTime[ch][ihit];
				temp_clk[ihit] = clockNumberDriftTime[ch][ihit];
				temp_peak[ihit] = 0;;
				temp_sum[ihit] = 0;
				int clk;
				for(clk = clockNumberDriftTime[ch][ihit]; clk<(ihit+1>=tdcNhitwire?NSAM:clockNumberDriftTime[ch][ihit+1]); clk++){
					if (adc[ch][clk]>temp_peak[ihit]){
						temp_peak[ihit]=adc[ch][clk];
					}
					if (clk!=clockNumberDriftTime[ch][ihit]&&adc[ch][clk]<o_pedestal[ch]) break;
					temp_sum[ihit] += adc[ch][clk]-o_pedestal[ch];
				}
				temp_width[ihit] = clk-clockNumberDriftTime[ch][ihit];
				o_areaall[ch] += temp_sum[ihit];
				for(int iH = 0; iH<50; iH++){
					if (!iH||temp_peak[ihit]>=o_pedestal[ch]+iH-1) o_nPeaks[ch][iH]++;
				}
				temp_peak[ihit]-=o_pedestal[ch];
			}
			for(int iH = 0; iH<50; iH++){
				if (o_nPeaks[ch][iH]) o_nHits[iH]++;
			}
			o_clk->push_back(temp_clk);
			o_peak->push_back(temp_peak);
			o_sum->push_back(temp_sum);
			o_tdc->push_back(temp_tdc);
			o_width->push_back(temp_width);
		}
		t->Fill();
	}
	t->Write();

	return 0;
}

void print_usage(char* prog_name)
{
	fprintf(stderr,"\t%s [runNo] <[nEventMax]>\n",prog_name);
}

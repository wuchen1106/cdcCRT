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
	std::cout<<__LINE__<<std::endl;
	TFile * f = new TFile("");
	std::cout<<(void*)f<<std::endl;
	std::cout<<__LINE__<<std::endl;
	if (f) f->ls();
	std::cout<<__LINE__<<std::endl;
	TString a = "1+1";
	TString b = "2";
	if (a==b){
		std::cout<<a<<"="<<b<<std::endl;
	}
	else{
		std::cout<<a<<"!="<<b<<std::endl;
		a=b;
	}
//	std::vector<int> * triggerNumber = 0;
//	std::vector<std::string> * ostring = 0;
//	TFile * f = new TFile("output.root","RECREATE");
//	TTree * t = new TTree("t","t");
//	t->Branch("triggerNumber",&triggerNumber);
//	t->Branch("name",&ostring);
//
//	for (Long64_t i = 0;i<10; i++){
//		triggerNumber=new std::vector<int>;
//		ostring=new std::vector<std::string>;
//		for (int j = 0; j<i; j++){
//			triggerNumber->push_back(0);
//			ostring->push_back(Form("%d",j));
//		}
//		t->Fill();
//	}
//	t->Write();
//	f->Close();

	return 0;
}

#include <iostream>
#include "TFile.h"
#include "TTUBE.h"

int main(int argc, char** argv){
	TFile * f = new TFile("hitmap.root","RECREATE");
	TTUBE * t = new TTUBE("tube","Tube","ChamberGas",10,20);
	t->Write();
	f->Close();
	return 0;
}

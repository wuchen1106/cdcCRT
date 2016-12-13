{
	TChain * chain = new TChain("t","t");
	chain->Add("../root/h_61.root");
	int triggerNumber;
	std::vector<double> * i_aa = 0;
	std::vector<int> * i_np = 0;
	std::vector<int> * i_layerID = 0;
	std::vector<int> * i_wireID = 0;
	chain->SetBranchAddress("triggerNumber",&triggerNumber);
	chain->SetBranchAddress("aa",&i_aa);
	chain->SetBranchAddress("np",&i_np);
	chain->SetBranchAddress("layerID",&i_layerID);
	chain->SetBranchAddress("wireID",&i_wireID);

	TFile * ofile = new TFile("output.root","RECREATE");
	TTree * otree = new TTree("t","t");
	int nNoisyHits = 0;
	int nNoisyHits7 = 0;
	int nNoisyHits6 = 0;
	int nNoisyHits3 = 0;
	int nNoisyHits2 = 0;
	int nLargeHits = 0;
	int nLargeHits7 = 0;
	int nLargeHits6 = 0;
	int nLargeHits3 = 0;
	int nLargeHits2 = 0;
	otree->Branch("t",&triggerNumber);
	otree->Branch("nn",&nNoisyHits);
	otree->Branch("nn7",&nNoisyHits7);
	otree->Branch("nn6",&nNoisyHits6);
	otree->Branch("nn3",&nNoisyHits3);
	otree->Branch("nn2",&nNoisyHits2);
	otree->Branch("nl",&nLargeHits);
	otree->Branch("nl7",&nLargeHits7);
	otree->Branch("nl6",&nLargeHits6);
	otree->Branch("nl3",&nLargeHits3);
	otree->Branch("nl2",&nLargeHits2);
	for (int i = 0; i<chain->GetEntries();i++){
		chain->GetEntry(i); 
		nNoisyHits = 0;
		nNoisyHits7 = 0;
		nNoisyHits6 = 0;
		nNoisyHits3 = 0;
		nNoisyHits2 = 0;
		nLargeHits = 0;
		nLargeHits7 = 0;
		nLargeHits6 = 0;
		nLargeHits3 = 0;
		nLargeHits2 = 0;
		for (int iHit = 0;iHit<i_aa->size(); iHit++){
			if ((*i_aa)[iHit]>2500&&(*i_np)[iHit]>=9){
				nNoisyHits++;
				if ((*i_wireID)[iHit]>100){
					if ((*i_layerID)[iHit]==19) nNoisyHits7++;
					else nNoisyHits6++;
				}
				else{
					if ((*i_layerID)[iHit]==19) nNoisyHits3++;
					else nNoisyHits2++;
				}
			}
			if ((*i_aa)[iHit]>5000){
				nLargeHits++;
				if ((*i_wireID)[iHit]>100){
					if ((*i_layerID)[iHit]==19) nLargeHits7++;
					else nLargeHits6++;
				}
				else{
					if ((*i_layerID)[iHit]==19) nLargeHits3++;
					else nLargeHits2++;
				}
			}
		}
		if (nNoisyHits>0||nLargeHits>0){
			otree->Fill();
		}
	}
	otree->Write();
	ofile->Close();
}

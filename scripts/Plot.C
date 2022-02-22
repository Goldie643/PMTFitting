#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <ctime>
#include <iomanip>
#include <stdexcept>

#include <cmath>
#include <vector>
#include <numeric>
#include <functional>
//#include <vector>   //make code slower
//#include <algorithm>//make code slower

#include <stdlib.h>
#include <stdio.h>
#include <TVirtualFitter.h>
#include <TMath.h>
#include <TStopwatch.h>
#include <TMatrixD.h>
#include <TVectorD.h>

#include <TRandom.h>
#include <TComplex.h>

using std::ifstream;
using std::ofstream;
using std::istringstream;
using std::ostringstream;
using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::invalid_argument;
using std::setw; 
using std::fixed; 
using std::ios; 
using namespace std;
streamsize precision(20);
float StillSignal(float x, std::vector<float> *wy, int m, int i){
        float sum = 0;
        if (m+i >= 0 && m+i < wy->size()) {
                sum += (x - wy->at(m+i));
        }
        //if (sum > 0 ){ return sum;} else return 0; 
        return sum;
}
void ProcessWaveform(std::vector<float> *wx, std::vector<float> *wy, float &totCharge, float &timePeak, float hz, float adcFactor){
        float widthNS = 40;
        int width = (int)(widthNS*(hz/1e9));
        int buffer = 5; 
        int halfSigWidth = 4;

        int minPos = 0;
        float minCharge = 10000;
        float maxCharge = -10000;
        float minTime = -10000;
        float fullCharge = 0;

        float sumCharge = 0;
        int sumInt = 0;

        float currentWindow = 0;
        float minimumWindow = 0;
        int minimumStart = 0;

        for (int i = 0 + buffer; i < wy->size() - buffer; i++){ // find minimum
                if (wy->at(i) < minCharge){minCharge = wy->at(i); minPos = i; minTime = wx->at(i);}
        }
        int lowInt = minPos - halfSigWidth;
        int highInt = minPos + halfSigWidth;

        if (lowInt < 0) lowInt = 0;
        if (highInt > wy->size()) highInt = wy->size();

        for (int i = 0; i < wy->size(); i++){

                float totCharge = 0;
                if (i < lowInt || i > highInt){
                        sumCharge += wy->at(i);
                        sumInt++;
                }
        }

        bool keepPoint = true;

        float baseCharge = sumCharge/sumInt;
        float countCharge = 0;
        bool stop = false;

        countCharge += StillSignal(baseCharge, wy, minPos, 0);
        countCharge += StillSignal(baseCharge, wy, minPos, -1);
        if ( StillSignal(baseCharge, wy, minPos, -1) != 0 ) {  countCharge += StillSignal(baseCharge, wy, minPos, -2); }

        countCharge += StillSignal(baseCharge, wy, minPos, +1);
        if ( StillSignal(baseCharge, wy, minPos, +1) != 0 ) { countCharge += StillSignal(baseCharge, wy, minPos, +2);  }


        totCharge = countCharge*adcFactor; // voltage in [mV]
        timePeak = 1e9*minTime/hz; // time in [ns]


}

 void Plot(){
	 
	TFile *input = new TFile ("/mnt/lustre/groups/nms_epapg/k20043567/PMT_OD/Data/pmt_data_202112/NNVT034077/NNVT034077_1100V_10m_DR_A_cooldown26h.root", "READ");
	TTree *tDevice = (TTree*) input->Get("device");
	TTree *tWaves = (TTree*) input->Get("data");

	float frequency;
	float maxSamples;
        int resolution;
        float voltLow;
        float voltHigh;
	std::vector<float> *wavex = 0;
	std::vector<float> *wavey = 0;
	int time;

	tDevice->Branch("frequency",&frequency);
	tDevice->Branch("maxSamples",&maxSamples);
        tDevice->SetBranchAddress("resolution",&resolution);
        tDevice->SetBranchAddress("voltLow",&voltLow);
        tDevice->SetBranchAddress("voltHigh",&voltHigh);

	//tWaves->Branch("wavex", &wavex, &bwavex);
	//tWaves->Branch("wavey", &wavey, &bwavey);
	tWaves->SetBranchAddress("wavex", &wavex);
	tWaves->SetBranchAddress("wavey", &wavey);
	tWaves->SetBranchAddress("time", &time);
	
	tDevice->GetEntry(0); // frequency and maxSamples only set once
	float adcFactor = (voltHigh - voltLow)*1000/resolution;

//	std::cout << "AAAA" << std::endl;
	TFile *output = new TFile ("NNVT034077_1100V_10m_DR_A_cooldown26h_waveform.root", "RECREATE");
	output->cd();
	// Now we loop over the events
	for (int c = 0; c < tWaves->GetEntries(); c++){
//		std::cout << "BBBB" << std::endl;
		//if (c >= tWaves->GetEntries()) continue; // make sure not to go outside of limit of tree
//		std::cout << "CCCC" << std::endl;
//		std::cout << "DDDD" << std::endl;

		std::cout << tWaves->GetEntries() << std::endl;

		tWaves->GetEntry(c);
//		std::cout << "EEEE" << std::endl;
                float totCharge = 0;
                float timePeak = 0;

                ProcessWaveform(wavex, wavey, totCharge, timePeak, frequency, adcFactor);
                if (totCharge > 14){ 

		TGraph *g = new TGraph;
		g->GetYaxis()->SetRangeUser(0,100);
		//g->SetName( std::to_string(c).c_str() );
		std::cout << "size:\t" << wavex->size() << std::endl;
		std::cout << c << "th event" << std::endl;
		for (int i = 0; i < wavex->size(); i++){
			g->SetPoint (i, wavex->at(i), wavey->at(i));		
		}
		// budget way to show it in a nice scale
//		g->SetPoint(wavex->size(), 150, 0);
//		g->SetPoint(wavex->size()+1, 151, 100);
		g->Write();
		}	    
	}
    
	
	output->Close();
	//tWaves->ResetBranchAddresses(); 
 }




#include <fstream>


void GetMeanStd(std::vector<double> v, double &m, double &d){

	double mean = 0;
	for (int i = 0; i < v.size()-1; i++){
		mean += v[i];
	}
	mean /= (double)v.size();

	double dev = 0;
	for (int i = 0; i < v.size()-1; i++){
		dev+= (v[i] - mean)*(v[i] - mean);
	}

	m = mean;
	d = sqrt(dev/v.size());

}

// void DarkRate(std::string inFile, double pe = 10.0, double thresh = 1.0){
void DarkRate(std::string inFile, double pe, double thresh){
	std::ofstream outfile;

  	outfile.open("darkrates.txt", std::ios_base::app); // append instead of overwrite

	TFile *input = new TFile (inFile.c_str(), "READ"); //Get the input root file

	TTree *tDevice = (TTree*) input->Get("device");
	TTree *tWaves = (TTree*) input->Get("data");

	float frequency;
	int maxSamples;
	int resolution;
	float voltLow;
	float voltHigh;
	std::vector<float> *wavex = 0;
	std::vector<float> *wavey = 0;
	int time;

	//tDevice stores some properties of the digitizer and tWaves stores the waveform, i.e. voltage vs time

	tDevice->SetBranchAddress("frequency",&frequency);
	tDevice->SetBranchAddress("maxSamples",&maxSamples);
	tDevice->SetBranchAddress("resolution",&resolution);
	tDevice->SetBranchAddress("voltLow",&voltLow);
	tDevice->SetBranchAddress("voltHigh",&voltHigh);

	tWaves->SetBranchAddress("wavex", &wavex);
	tWaves->SetBranchAddress("wavey", &wavey);
	tWaves->SetBranchAddress("time", &time);
	
	tDevice->GetEntry(0); // frequency and maxSamples only set once

	float adcFactor = (voltHigh - voltLow)*1000/resolution; // Multiply by adc counts to get voltage [mV]



	int width = 15;//?
	int dark = 0;
	double timef = 0;
	
	std::cout << "Entries:\t" << tWaves->GetEntries() << std::endl;
	
	std::vector<double> darkVals;//?
	// Now we loop over the events
	for (int c = 0; c < tWaves->GetEntries(); c++){
		//for c-th waveform - get the total time info (get the length of time axis and multiple 1ns)
		tWaves->GetEntry(c);
		int size = wavex->size();
		timef += (double)(1e-9)*size;
	
		//for each 100000 entries do the dark/timef to get the dark noise	
		if (c % 100000 == 0 || c == tWaves->GetEntries() -1){
			darkVals.push_back((double)dark/timef);
			std::cout << "dark count " << dark << "while time is " << timef << ";" << std::endl;
			std::cout << "darkVals: " << (double)dark/timef << std::endl;
			dark = 0;
			timef = 0;
		}
		
		bool belowThreshold = true; //set a flag to start and stop count the dark rate
		

		if (c == 0) {std::cout << "Size:\t" << size << std::endl;}
		//calculate the baseline - add all bins and get mean
		double baseline = 0;

		for (int i = 0; i < size; i++){
			baseline += wavey->at(i);
		}
		baseline /= (double)(size);

//set a height of volatge method
/*
		for (int i = 0; i < size; i++){
			double height = wavey->at(i) - baseline;
			height*=adcFactor;
			if (belowThreshold){
				if (height >= 2*pe*thresh/width) { dark++; belowThreshold = false; }
			}
			else{
				if (height < 2*pe*thresh/width) {belowThreshold = true;}
			}
		}
*/
//intergal method to count DR

		for (int i = 0; i < size - width; i++){

			double sum = 0;
			//sum over every continous width bins - sum
			for (int j = 0; j < width; j++){
				sum += wavey->at(i+j) - baseline;
			}
			//trans to the voltage uni
			sum*=adcFactor;

			// the counting method doing in this way 
			// when sum goes larger than the threshold (0.4pe or some other value)
			// count the peak and then set the flag to false to make sure the peak would not be counted several times
			// Then when the intergal goes lower than the threshold which mean the peak is end
			// - set the flag to true to restart to searching the next peak
			if (belowThreshold){
				if ( sum >= pe*thresh ) { dark++; belowThreshold = false; }
			} else {
				if (sum < pe*thresh){belowThreshold = true;}
			}
			
		}

			    
	}


	double rateMean = 0;
	double rateSTD = 0;
	GetMeanStd(  darkVals, rateMean, rateSTD);
	std::cout << "DR = " << rateMean << "+-" << rateSTD << std::endl;
	outfile << inFile << "\t" << rateMean << "\t" << rateSTD << "\n";  

 }


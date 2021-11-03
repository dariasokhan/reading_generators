/********************************************************************************/
/*                                                                              */
/*   Macro to read in a list of generated LUND files for dvmp on proton or      */
/*   neutron in deuteron and write out a root file with the particle four-      */
/*   momenta, the cross-section and the beam energy.                            */
/*                                                                              */
/*   To run, make a list of all LUND files you want to read in, eg:             */
/*   ls *.dat > filelist.txt                                                    */
/*                                                                              */
/*   Run through ROOT:                                                          */
/*     root -l                                                                  */
/*     [] .L root_from_lund.C                                                   */
/*     [] root_from_lund((char*)"filelist.txt",(char*)"outrootfile.root")       */
/*                                                                              */
/*   where outrootfile.root is the output file.                                 */
/*                                                                              */
/*   You can also run without the (char*) above, but you'll get a harmless.     */
/*   warning.                                                                   */
/*                                                                              */
/*                                                                              */
/*   Daria Sokhan, Saclay, Nov 2021                                             */
/********************************************************************************/

 
int ce = 0;        // event counter for "good" events
int ce_read = 0;   // event counter for all events read in  

// variables to write out to output file, for each event (made global to avoid headache of passing arrays):

int ivar[10];         // int in header line
double dvar[10];      // double in header line
int ipar[8][10];      // int in particle lines (should be 5 of them for each event but in case of photons, etc, set max to 8)
double dpar[8][10];   // double in particle lines

TFile *Outfile;       // output ROOT file

TTree *GenEvent;      // tree to hold the generated info (only for standard DVMP where pi0 decays to two photons)

// Variables for the output ROOT tree:
double xsec;
double beamE;
TLorentzVector *electron;
TLorentzVector *spectator;
TLorentzVector *recoil;
TLorentzVector *photon1;
TLorentzVector *photon2;

// Functions used by the macro:
void set_up_objects(char*);
void process_file(char*);


void root_from_lund(char *listname, char* outrootfile){   // takes as argument name of filelist to read in and name of ROOt file to write out
  
  set_up_objects(outrootfile);   // create the tree and output file

  cout << " Reading from list: " << listname << "\n" << endl;
  
  ifstream filelist;
  
  char file_name[1000];      // make sure this char array is large enough so that the file name doesn't get cut off!
  char last_file[1000];
  
  filelist.open(listname);
  
  int L = 0;  // to count lines in the list
  
  if(filelist.is_open()){
    
    while(!filelist.eof()) {
      if (!filelist.good()) break;
      
      filelist >> file_name;  // read in the name of the file from the list into the filename variable. Assumes one file name per line and no punctuation.
      
      if (strcmp(last_file,file_name) != 0){   // makes sure the last file doesn't get counted twice.
	
	cout << " Reading from file: " << file_name << endl;
	
	process_file(file_name);  // reads in the data from the actual file and does any other processing on it
	
	sprintf(last_file,"%s",file_name);  // save name of the current file into "last_file"
        
	L++;
	
      }
    }
  }
  else cout << "Crap, no " <<  listname << " found!" << endl;
  
  filelist.close();
  
  cout << "\n Number of total events read in: " << ce_read << endl;
  cout << "Number of good events saved to the ROOT file: " << ce << endl;
  
  GenEvent->Write();
  Outfile->Write();
  Outfile->Close();
  
}

void process_file(char *filename){
  
  // zero variables which will hold the LUND data at the start of each file:
  
  for (int i=0; i<10; i++){
    ivar[i] = 0;
    dvar[i] = 0.;
    for (int j=0; j<5; j++){
      ipar[j][i] = 0;
      dpar[j][i] = 0.;
    }
  }
  
  // set local counters for each file:
  int lce = 0;   // local counter for "good" events
  int lce_read = 0;   // local counter for events read in 
  int p = 0;     // counter for particle lines in each event
  
  int bad_event = 0;    // flag for a bad event

  ifstream file;
  
  file.open(filename);
  
  if(file.is_open()){
    
    while(!file.eof()) {
      if (!file.good()) break;
      
      if (p == ivar[0]){  // first time this gets read is first line of file, ivar[0] = 0. After that, it's read when all of previous event's particles have been read in.
	
	file >> ivar[0] >> ivar[1] >> ivar[2] >> ivar[3] >> ivar[4] >> ivar[5] >> dvar[0] >> ivar[6] >> ivar[7] >> dvar[1];
	
	//if (ivar[0] != 5) cout << "Caught you! Event " << lce << " has " << ivar[0] << " particles!" << endl;    // expect 5 particles per normal event
	
	p = 0;           // reset the counter of particles per event
	bad_event = 0;   // reset flag for this event
	
	// set event variable for the ROOT output file:
	beamE = dvar[0];
	xsec = dvar[1];
	
      }
      else {  // lines which have info for each particle
	
	file >> ipar[p][0] >> ipar[p][1] >> ipar[p][2] >> ipar[p][3] >> ipar[p][4] >> ipar[p][5] >> dpar[p][0] >> dpar[p][1] >> dpar[p][2] >> dpar[p][3] >> dpar[p][4] >> dpar[p][5] >> dpar[p][6] >> dpar[p][7];
	
	if (ipar[p][0] == 1){
	  if (ipar[p][3] == 11) electron->SetPxPyPzE(dpar[p][0],dpar[p][1],dpar[p][2],dpar[p][3]);
	  else {
	    bad_event = 1;
	    cout << "Odd-balls: first particle in event number " << lce << " isn't an electron. It's a " << ipar[p][3] << ". Humpf!" << endl;
	  }
	}
	
	else if (ipar[p][0] == 2){
          if (ipar[p][3] == 2112 || ipar[p][3] == 2212) spectator->SetPxPyPzE(dpar[p][0],dpar[p][1],dpar[p][2],dpar[p][3]);
          else{
	    bad_event = 1;
	    cout << "Odd-balls: second particle in event number " << lce << " isn't a nucleon. It's a " << ipar[p][3] << ". Humpf!" << endl;
	  }
	}
	
	else if (ipar[p][0] == 3){   // particle with index 3 in event is the active nucleon
	  if (ipar[p][3] != ivar[6]){
	    bad_event = 1;
	    cout << "Oups, event " << lce << "has one specified target particle and another active nucleon!" << endl;
	  }
	  else {
	    if (ipar[p][3] == 2212 || ipar[p][3] == 2112) recoil->SetPxPyPzE(dpar[p][0],dpar[p][1],dpar[p][2],dpar[p][3]);
	    else{
	      bad_event = 1;
	      cout << "Odd-balls: third particle in event number " << lce << " isn't a nucleon. It's a " << ipar[p][3] << ". Humpf!"  << endl;
	    }
	  }
	}
	else if (ipar[p][0] == 4){
	  if (ipar[p][3] == 22) photon1->SetPxPyPzE(dpar[p][0],dpar[p][1],dpar[p][2],dpar[p][3]);
	  else{
	    bad_event = 1;
	    // cout << "Odd-balls: fourth particle in event number " << lce << " isn't a photon. It's a " << ipar[p][3] << ". Humpf!" << endl;
	  }
	}
	else if (ipar[p][0] == 5){
	  if (ipar[p][3] == 22) photon2->SetPxPyPzE(dpar[p][0],dpar[p][1],dpar[p][2],dpar[p][3]);
	  else{
	    bad_event = 1;
	    // cout << "Odd-balls: fourth particle in event number " << lce << " isn't a photon. It's a " << ipar[p][3] << ". Humpf!" << endl;
	  }
	}
	
	// Once you've read in the final particle for the event, write the event to the tree:
	if (ipar[p][0] == ivar[0]){
	  lce_read++;    // counter for total events read in, local to the file
	  ce_read++;    // counter for total events read in overall
	  if (bad_event == 0){
	    GenEvent->Fill();
	    ce++;   // increment global event counter
	    lce++;  // also the local, events-per-file counter
	  }
	}
	p++;  // increment particle counter per event
      }
    }
  }
  else cout << "Crap, no " <<  filename << " found!" << endl;
  
  file.close();
  
  cout << "\n In this file, number of events: " << lce_read << endl;
  cout << "\t Of these, good events saved to the ROOT file: " << lce << endl;
  
}


void set_up_objects(char *rootfilename){
  
  Outfile = new TFile(rootfilename,"RECREATE","Generated DVMP events read from LUND");
  
  // create new tree and its branches:
  GenEvent = new TTree("TCSevent","generated TCS events");
  
  GenEvent->Branch("beamE",&beamE,"beamE/D");
  GenEvent->Branch("xsec",&xsec,"xsec/D");
  GenEvent->Branch("electron","TLorentzVector",&electron);
  GenEvent->Branch("spectator","TLorentzVector",&spectator);
  GenEvent->Branch("recoil","TLorentzVector",&recoil);
  GenEvent->Branch("photon1","TLorentzVector",&photon1);
  GenEvent->Branch("photon2","TLorentzVector",&photon2);
  
}





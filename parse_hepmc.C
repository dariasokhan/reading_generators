/*****************************************************************/
/*                                                               */
/*   Macro to parse TCS HEPMC3 files,                            */
/*   generated either with EpIC or with the                      */
/*   ToyMC for TCS, and save all the                             */
/*   generated particles as four-momenta                         */
/*   in an output ROOT file, the name of                         */
/*   which is chosen by the user.                                */     
/*                                                               */
/*   IMPORTANT:                                                  */
/*   The electron helicity is set manually                       */
/*   currently -- adjust it according to how                     */
/*   the files are read in through the list.                     */
/*   Assumes each file has a constant                            */
/*   helicity.                                                   */
/*                                                               */
/*   FLAGS TO SET:                                               */
/*   * Whether you're running it on a ToyMC file (default is     */
/*   EpIC), whether the file has been put through the after-     */
/*   burner to add crossing-angles (default is no).              */
/*   * The debug flag lets everything that's been read in be     */
/*   be printed to screen.                                       */
/*   * If you only want to read in a certain number of events,   */
/*   you can  set the fag for this and specify the number.       */
/*                                                               */
/*   The code has been set up for files where the quasi-real     */
/*   photon had its code manually changed to 3 (from 1) in       */
/*   EpIC files. Once there's a formal change in EpIC, update    */
/*   this feature.                                               */
/*                                                               */
/*   To run, make a list of all HEPMC files you want to          */
/*   read in, eg:                                                */
/*   ls *.txt > filelist.txt                                     */
/*                                                               */
/*   Then, run through ROOT:                                     */
/*    root -l                                                    */ 
/*    [] .L parse_hepmc.C                                        */
/*    [] parse_hepmc("filelist.txt","output.root")               */
/*                                                               */
/*   or, to suppress the char warning, do this instead:          */
/*    [] parse_hepmc((char*)"filelist.txt",(char*)"output.root") */     
/*                                                               */
/*   Daria Sokhan, Saclay, Oct 2021                              */
/*                                                               */
/*****************************************************************/

/************ CUSTOMISE! *******************/
// Flags (0 is "off", 1 is "on". No other values should be used):

int debug = 1;             // will print everything it reads to screen if set to 1
int burn = 0;              // this is for the EpIC hepmc files after the afterburner, set to 1 if needed
int toyMC = 1;             // this flag is for parsing toyMC output, set to 1 if needed

int event_limit = 0;       // flat to stop processing events after the max_events number is reached -- set to 1 if you want this
int max_events = 200000;   // set the max. number of events you want to read in. Value is only used if event_limit flag is set to 1.
/********************************************/


TFile *Outfile;

// branches of the output tree:
TTree *TCSevent;
TLorentzVector *ebeam;
TLorentzVector *pbeam;
TLorentzVector *escattered;  // scattered electron 
TLorentzVector *q;
TLorentzVector *recoil;      // scattered nucleon
TLorentzVector *qprime;
TLorentzVector *lep_minus;   // this is the actual electron produced in the lepton pair
TLorentzVector *lep_plus;    // e+ 
int helicity;                // electron helicity

TTree *TCSinfo;
double xsec_total;           // total cross-section for all the files read in (only quoted in EpIC unburned files)
double xsec_total_err;       // uncertainty on the total cross-section for all read-in files.

int process_file(char*);
void set_up_objects(char*);


// The main function loops through the files in the list and runs the process_file function on each one, then saves the total tree to the output file:

void parse_hepmc(char *listname, char *outfilename){   // takes as argument name of filelist and the name of the output ROOT file you want created
  
  set_up_objects(outfilename);   // tree branches, output file...

  xsec_total = 0.;        // initialise these to zero
  xsec_total_err = 0.;
  
  /*****************************************/
  // standard code to read in list of files         
  // open and parse the file-list:
  
  cout << "\n Reading from list: " << listname << "\n" << endl;
  
  ifstream filelist;
  char file_name[1000];      // make sure this char array is large enough so that the file name doesn't get cut off!                                             
  char last_file[1000];
  
  filelist.open(listname);
  
  int N = 0;          // will count the number of files in your list   
  int ce = 0;         // overall event counter.
  int temp_lce = 0;   // just to hold the return variable of the file processing
  
  if(filelist.is_open()){
    
    while(!filelist.eof()) {
      if (!filelist.good()) break;
      
      filelist >> file_name;  // read in the name of the file from the list into the filename variable. Assumes one file name per line and no punctuation.
      
      if (strcmp(last_file,file_name) != 0){   // makes sure the last file doesn't get counted twice.    
	
	cout << "\n Reading from file: " << file_name << endl;

	/*********** CUSTOMISEE! Set helicity value per file here!! *********/	

	if (N == 0 || N == 1 || N == 2 || N == 3 || N == 4) helicity = 0;
	else if (N == 5 || N == 6 || N == 7 || N == 8 || N == 9) helicity = 1;
	else cout << "Unknown N!" << endl;

	/*********************************************************************/

	temp_lce = process_file(file_name);  // reads in the data from the actual file, returns number of events read in that file
	
	ce = ce + temp_lce;   // the overall event counter
	
        sprintf(last_file,"%s",file_name);     // save name of the current file into "last_file"                                                                 
	
        N++;
      }
    }
  }
  else cout << "Crap, no " <<  listname << " found!" << endl;
  
  filelist.close();
  
  cout << "\n Total no of files in list: " << N << endl;
  
  /*****************************************/
  
  cout << "\n Number of total events: " << ce << endl;
  
  printf("\n Integrated cross-section: %.8f +/- %.8f \n\n\n",xsec_total,xsec_total_err);

  TCSinfo->Fill();   // fill the tree with integrated cross-section info once all the files have been processed
  
  // Save the created tree (and any histograms if you create them) to the output file:
  
  TCSevent->Write();
  TCSinfo->Write();
  Outfile->Write();
  Outfile->Close();
  
}


// This function runs on each file and does the actual parsing of the data in it, 
// picking out the relevant information and creating four-momenta for each event:

int process_file(char *filename){
  
  int lce = 0;  // local event counter for this file                                                                                                                     
  
  // event variables you're interested in:
  int pid = 0;
  double px = 0.;
  double py = 0.;
  double pz = 0.;
  double E = 0.;
  int part_num = -10;
  int code = 0;

  double xsec_int;    // this is the integrated cross-section for the file and its uncertainty
  double xsec_int_err;  

  ifstream file;
  
  // flags to navigate the hepmc format:  
  int file_start = 0;    // flag for start of file
  int new_event = 0;     // flag for start of new event
  
  // dummy variables to hold things from the hepmc file which you don't need, kept separate for debugging:
  double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12;
  char c1[100], c2[100], c3[100], c4[100], c5[100], c6[100];
  
  file.open(filename);
  
  if(file.is_open()){
    
    while(!file.eof()) {
      if (!file.good()) break;
      
      char letter[1]; // single letter at start of some lines, used to ID the info on them. Need to re-create it for each new instance, otherwise weird bugs appear.

      if (file_start == 0){       // start of file, first two lines (three entries) are not needed
	file >> c1 >> c2 >> c3;
	file_start = 1;
	if (debug == 1){
	  cout << "first lines: " << endl;
	  cout << c1 << " " << c2 << " " << c3 << endl;
	}
	if (burn == 1){
	  for (int i=0; i<19; i++){  // 19 extra lines added by the afterburner, also not needed
	    file >> c1 >> c2 >> d1;
	    if (debug == 1) cout << c1 << " " << c2 << " " << d1 << endl;
	  }
	}
      } // closes file-start loop

      else { // after parsing the lines at the start of file, progress to this loop
	if (new_event == 0){  // start of a new event

	  file >> letter;  // normal events will start with a line beginning with "E", end of file text will start with "T"

	  if (strcmp(letter,"E") == 0){   // normal event
	    
	    if (burn == 0) file >> d1 >> d2 >> d3;
	    else if (burn == 1) file >> d1 >> d2 >> d3 >> c1 >> d4 >> d5 >> d6 >> d7;   // after-burner adds more stuff to the first line in the event
	    else cout << "Burn flag unknown! Check." << endl;
	    file >> c2 >> c3 >> c4;
	    file >> c5  >> d8 >> c6 >> d9 >> d10 >> d11 >> d12;   // line with cross-section, but cross-section isn't saved. Change if needed!
	    new_event = 1;
	  
	    if (debug == 1){
	      cout << "Starting event number " << lce+1 << endl;    // lce is only incremented at the end of the event
	      if (burn == 0) cout << letter << " " << d1 << " " << d2 << " " << d3 << endl;
	      else if (burn == 1) cout << letter << " " << d1 << " " << d2 << " " << d3 << " " << c1 << " " << d4 << " " << d5 << " " << d6 << " " << d7 << endl;
	      cout << c2 << " " << c3 << " " << c4 << endl;
	      cout << c5 << " " << d8 << " " << c6 << " " << d9 << " " << d10 << " " << d11 << " " << d12 << endl;
	    }
	  }  // start of each normal event has been parsed in

	  else if (strcmp(letter,"T") == 0){   // end of file, this is only present in the non-burned EpIC files
	    // In the after-burned files there's just one string and then that's the end of the file.
	    
	    file >> c1 >> c2 >> c3 >> c4 >> c5 >> c6;
	    if (debug == 1) cout << "\n" << letter << " " << c1 << " " << c2 << " " << c3 << " " << c4 << " " << c5 << " " << c6 << endl;
	    file >> c1 >> c2 >> d1;
	    if (debug == 1) cout << c1 << " "<< c2 << " " << d1 << endl;
	    file >> c1 >> c2 >> c3 >> c4 >> d1 >> c5 >> c6;
	    if (debug == 1) cout << c1 << " " << c2 << " " << c3 << " " << c4 << " " << d1 << " " << c5 << " " << c6 << endl;
	    file >> c1 >> c2 >> xsec_int_err;
	    file >> c3 >> c4 >> xsec_int;
	    if (debug == 1){
	      cout << c1 << " " << c2 << " " << xsec_int_err << endl;
	      cout << c3 << " " << c4 << " " << xsec_int << endl;
	    }
	    file >> c1 >> c2 >> c3 >> c4;   // end of file
	    if (debug == 1){
	      cout << c1 << " " << c2 << " " << c3 << endl;
	      cout << c4 << endl;
	    }
	  } // end of file loop
	  
	  else {  // that's the case for the after-burned file and ToyMC -- there's only one string at the very end.
	    file >> c1;
	    if (debug == 1) cout << letter << c1 << endl;
	  }
	  
	} // start of each new event has been parsed in
	
	// This is the start of every subsequent line in the event:
	file >> letter >> part_num;  // read in the first two items of the line. Should be "P" followed by particle number if it's a particle. 
	
	if (debug == 1){
	  cout << "Particle line: " << endl;
	  cout << letter << " " << part_num << endl;
	}
	
	if (strcmp(letter,"V") == 0){    // this is the line with vertex info. It's the only one that's different from the particle lines. None of it is needed. 
	  if (burn == 0) file >> d1 >> c1;
	  else if (burn == 1) file >> d1 >> c1 >> c2 >> d2 >> d3 >> d4 >> d5;
	  if (debug == 1){
	    if (burn == 0) cout << d1 << " " << c1 << endl;
	    else if (burn == 1) cout << d1 << " " << c1 << " " << c2 << " " << d2 << " " << d3 << " " << d4 << " " << d5 << endl;
	  }
        }
	else if (strcmp(letter,"P") == 0){   // particle line
	  
	  file >> d1 >> pid >> px >> py >> pz >> E >> d2 >> code;
	  
	  if (debug == 1){
	    cout << d1 << " " << pid << " " << px << " " << py << " " << pz << " " << E << " " << d2 << " " << code << endl;	    
	  }
	  
	  /******** Here the particle information is read in and, depending on what particle it is, the four-momenta are created ******/
	  // Customise as needed for your hepmc set-up
	  
	  if (part_num == 1){
	    lce++;  // increment the counter for the new event only once the first particle has been read in. So you know it's a genuine event.
	    if (pid != 11 || (toyMC == 0 && code != 4) || (toyMC == 1 && code != 21)){
	      cout << "Weird! First particle doesn't seem to be a beam electron." << endl;
	      cout << "Pid: " << pid << endl;
	      cout << "Code: " << code <<endl;
	    }
	    ebeam->SetPxPyPzE(px,py,pz,E);
	  }
	  else if (part_num == 2){
            if ((toyMC == 0 && (pid != 11 || code != 1)) || (toyMC == 1 && (pid != 22 || code != 21))){
              if (toyMC == 0) cout << "Weird! Second particle doesn't seem to be a scattered electron." << endl;
	      else if (toyMC == 1) cout << "Weird! Second particle doesn't seem to be the quasi-real photon." << endl;
	      cout << "Pid: " << pid <<endl;
	      cout << "Code: " << code <<endl;
            }
            if (toyMC == 0) escattered->SetPxPyPzE(px,py,pz,E);
	    else if (toyMC == 1) q->SetPxPyPzE(px,py,pz,E);
          }
          else if (part_num == 3){
            if ((toyMC == 0 && (pid != 22 || code != 3)) || (toyMC == 1 && (pid != 11 || code != 1))){
	      if (toyMC == 0) cout << "Weird! Third particle doesn't seem to be the quasi-real photon!" <<endl;
	      else if (toyMC == 1) cout << "Weird! Third particle doesn't seem to be a scattered electron!" <<endl;
	      cout << "Pid: " << pid << endl;
              cout << "Code: " << code <<endl;
            }
            if (toyMC == 0) q->SetPxPyPzE(px,py,pz,E);
	    else if (toyMC == 1) escattered->SetPxPyPzE(px,py,pz,E);
          }
          else if (part_num == 4){
            if (pid != 2212 || (toyMC == 0 && code != 4) || (toyMC == 1 && code != 21)){
              cout << "Weird! Fourth particle doesn't seem to be the beam proton" <<endl;
              cout << "Pid: " << pid << endl;
              cout << "Code: " << code <<endl;
            }
            pbeam->SetPxPyPzE(px,py,pz,E);
          }
          else if (part_num == 5){
            if ((toyMC == 0 && (pid != 22 || code != 3)) || (toyMC == 1 && (pid != 2212 || code != 1))){
              if (toyMC == 0) cout << "Weird! Fifth particle doesn't seem to be the virtual photon" <<endl;
              else if (toyMC == 1) cout << "Weird! Fifth particle doesn't seem to be the recoil proton" <<endl;
	      cout << "Pid: " << pid << endl;
              cout << "Code: " << code <<endl;
            }
            if (toyMC == 0) qprime->SetPxPyPzE(px,py,pz,E);
	    else if (toyMC == 1) recoil->SetPxPyPzE(px,py,pz,E);
          }
          else if (part_num == 6){
            if ((toyMC == 0 && (pid != 2212 || code != 1)) || (toyMC == 1 && (pid != 22 || code != 21))){
              if (toyMC == 0) cout << "Weird! Sixth particle doesn't seem to be the recoil proton" <<endl;
	      else if (toyMC == 1) cout << "Weird! Sixth particle doesn't seem to be the virtual photon" <<endl;
	      cout << "Pid: " << pid << endl;
              cout << "Code: " << code <<endl;
            }
            if (toyMC == 0) recoil->SetPxPyPzE(px,py,pz,E);
	    else if (toyMC == 1) qprime->SetPxPyPzE(px,py,pz,E);
          }
	  else if (part_num == 7){
            if (pid != 11 || code != 1){
              cout << "Weird! Seventh particle doesn't seem to be the produced e-" <<endl;
              cout << "Pid: " << pid << endl;
              cout << "Code: " << code <<endl;
            }
            lep_minus->SetPxPyPzE(px,py,pz,E);
          }
          else if (part_num == 8){     // assumed that this is the last particle in the event
            if (pid != -11 || code != 1){
              cout << "Weird! Eighths particle doesn't seem to be the produced e+" <<endl;
              cout << "Pid: " << pid << endl;
              cout << "Code: " << code <<endl;
            }
            lep_plus->SetPxPyPzE(px,py,pz,E);
	    new_event = 0;  // 8th particle is the last one, reset this flag for the next event
	    	    
	    TCSevent->Fill();  // fill the tree with this event before progressing to the new one
	    
	    // For debugging:
	    if (debug == 1){
	      cout << "ebeam: " << ebeam->Px() << ", " << ebeam->Py() << ", " << ebeam->Pz() << ", " << ebeam->E() << endl;
	      cout << "pbeam: " << pbeam->Px() << ", " << pbeam->Py() << ", " << pbeam->Pz() << ", " << pbeam->E() << endl;
	      cout << "escattered: " << escattered->Px() << ", " << escattered->Py() << ", " << escattered->Pz() << ", " << escattered->E() << endl;
	      cout << "q: " << q->Px() << ", " << q->Py() << ", " << q->Pz() << ", " << q->E() << endl;
	      cout << "recoil: " << recoil->Px() << ", " << recoil->Py() << ", " << recoil->Pz() << ", " << recoil->E() << endl;
	      cout << "qprime: " << qprime->Px() << ", " << qprime->Py() << ", " << qprime->Pz() << ", " << qprime->E() << endl;
	      cout << "lep_minus: " << lep_minus->Px() << ", " << lep_minus->Py() << ", " << lep_minus->Pz() << ", " << lep_minus->E() << endl;
	      cout << "lep_plus: " << lep_plus->Px() << ", " << lep_plus->Py() << ", " << lep_plus->Pz() << ", " << lep_plus->E() << endl;
	    }

	    if (lce % 10000 == 0) cout << "Done events: " << lce << endl;
	    if (event_limit == 1 && lce == max_events) break;      // only process a certain number of events if required
	    
          } // end of loop for particle 8 (assuming it's the last one in the event)
	} // end of loop looking to particle lines starting with "P"
      } // end of loop around everything other than the start of the file
    }  // end of file loop

    file.close();
    
    cout << "Number of events in the file: " << lce << endl;
    cout << "-------------------" << endl;

    // add the integrated cross-section from this file to the total and re-calculate the uncertainty:
    xsec_total = xsec_total + xsec_int;                   
    xsec_total_err = sqrt(pow(xsec_total_err,2) + pow(xsec_int_err,2));

  }
  
  return lce;
}



// This function is called right at the start and just creates the output file and the output trees.
// Customise as needed

void set_up_objects(char *rootfilename){
  
  Outfile = new TFile(rootfilename,"RECREATE","Generated TCS events read from hepmc");
  
  // create new tree and its branches:                                                                                                                                                               
  TCSevent = new TTree("TCSevent","generated TCS events");
  
  TCSevent->Branch("ebeam","TLorentzVector",&ebeam);
  TCSevent->Branch("pbeam","TLorentzVector",&pbeam);
  TCSevent->Branch("escattered","TLorentzVector",&escattered);
  TCSevent->Branch("q","TLorentzVector",&q);
  TCSevent->Branch("recoil","TLorentzVector",&recoil);
  TCSevent->Branch("qprime","TLorentzVector",&qprime);
  TCSevent->Branch("lep_minus","TLorentzVector",&lep_minus);
  TCSevent->Branch("lep_plus","TLorentzVector",&lep_plus);
  TCSevent->Branch("helicity",&helicity,"helicity/I");

  TCSinfo = new TTree("TCSinfo","Info for the whole file");

  TCSinfo->Branch("xsec_total",&xsec_total,"xsec_total/D");
  TCSinfo->Branch("xsec_total_err",&xsec_total_err,"xsec_total_err/D");

}

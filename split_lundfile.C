/****************************************************/
/*                                                  */
/*   Macro to read in a list of generated files for */
/*   dvcs on proton and neutron in deuteron and     */
/*   write out two new sets of files which contain  */
/*   only dvcs on the proton or only on the neutron.*/
/*                                                  */
/*   The output file names are hard-coded:          */
/*                                                  */
/*        dvcsD_neut_N.dat                          */
/*        dvcsD_prot_N.dat                          */
/*                                                  */
/*   where N is the number of the file. The files   */
/*   will get numbered from 0 in the order in which */
/*   the input files are read in. For example, if   */
/*   you run this macro on two deuteron LUND files  */
/*   called:                                        */
/*          deut_764.dat and                        */
/*          deut_825.dat,                           */
/*   in that order, the output will be:             */
/*        dvcsD_neut_0.dat and dvcs_prot_0.dat,     */
/*   both corresponding to deut_764.dat, and        */
/*        dvcsD_neut_1.dat and dvcs_prot_1.dat,     */
/*   both corresponding to deut_825.dat.            */
/*                                                  */
/*   To run, make a list of all LUND files          */
/*   you want to read in, eg:                       */
/*   ls *.dat > filelist.txt                        */
/*                                                  */
/*   Run through ROOT:                              */
/*     root -l                                      */ 
/*     [] .L split_lundfile.C                       */
/*     [] split_lundfile((char*)"filelist.txt")     */
/*                                                  */
/*   You can also run without the (char*) above,    */
/*   but you'll get a harmless warning.             */
/*                                                  */
/*                                                  */
/*   Daria Sokhan, Saclay, Nov 2021                 */
/****************************************************/

#include <iomanip>

// proton/neutron/event counters:                                                                                                                                  
int count_p = 0;
int count_n = 0;
int ce = 0;

// variables to write out to output file, for each event (made global to avoid headache of passing arrays):                                                                                                
int ivar[10];         // int header line                                                                                                                 
double dvar[10];      // doubles header line                                                                                                             
int ipar[8][10];      // int particle lines (should be 5 of them for each event but in case of photons, etc, set max to 8)
double dpar[8][10];   // double particle lines                                                                                                           


void process_file(char*, int);
void write_event(char*, int, int);

void split_lundfile(char *listname){   // takes as argument name of filelist and number of files in it
  
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
	
	process_file(file_name, L);  // reads in the data from the actual file and does any other processing on it
	
	sprintf(last_file,"%s",file_name);  // save name of the current file into "last_file"
        
	L++;
	
      }
    }
  }
  else cout << "Crap, no " <<  listname << " found!" << endl;
  
  filelist.close();
  
  cout << "\n Number of total events: " << ce << endl;
  cout << "Number of total events with active proton: " << count_p << endl;
  cout << "Number of total events with active neutron: " << count_n << endl;
  
}
  
/**********************/

void process_file(char *filename, int N){
  

  // Open and close the output files just so that they are re-created -- otherwise the events will be added to the end of the file if it already exists!

  char fullname[100];
  ofstream outfile;
  sprintf(fullname,"dvcsD_prot_%d.dat",N);
  outfile.open(fullname);
  outfile.close();

  sprintf(fullname,"dvcsD_neut_%d.dat",N);
  outfile.open(fullname);
  outfile.close();


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
  int cp = 0;    // local counter for active protons
  int cn = 0;    // local counter for active neutrons
  int lce = 0;   // local counter for events
  int p = 0;     // counter for particle lines in each event

  int neutron_flag = -1;    // flag to specify an active neutron in the event

  ifstream file;
  
  file.open(filename);
  
  if(file.is_open()){
    
    while(!file.eof()) {
      if (!file.good()) break;
      
      if (p == ivar[0]){  // first time this gets read is first line of file, ivar[0] = 0. After that, it's read when all of previous event's particles have been read in.

	file >> ivar[0] >> ivar[1] >> ivar[2] >> ivar[3] >> ivar[4] >> ivar[5] >> dvar[0] >> ivar[6] >> ivar[7] >> dvar[1];
	
	//if (ivar[0] != 5) cout << "Caught you! Event " << lce << "has " << ivar[0] << " particles!" << endl;    // expect 5 particles per normal event
	
	p = 0; // reset the counter of particles per event
	neutron_flag = -1;    // reset the neutron flag for this event
      }
      else {  // lines which have info for each particle
	
	file >> ipar[p][0] >> ipar[p][1] >> ipar[p][2] >> ipar[p][3] >> ipar[p][4] >> ipar[p][5] >> dpar[p][0] >> dpar[p][1] >> dpar[p][2] >> dpar[p][3] >> dpar[p][4] >> dpar[p][5] >> dpar[p][6] >> dpar[p][7];
	
	if (ipar[p][0] == 3){   // particle with index 3 in event is the active nucleon
	  if (ipar[p][3] != ivar[6]) cout << "Oups, event " << lce << "has one specified target particle and another active nucleon!" << endl;
	  else {
	    if (ipar[p][3] == 2212){
	      count_p++;    // global proton counter
	      cp++;         // local proton counter
	      neutron_flag = 0; 
	    }
	    else if (ipar[p][3] == 2112){
	      count_n++;    // global neutron counter
	      cn++;         // local neutron counter
	      neutron_flag = 1;     // set flag for active neutron event!
	    }
	  }
	}
	// Once you've read in the final particle for the event, write the events out, depending on active particle:
	if (ipar[p][0] == ivar[0]){
	  if (neutron_flag == 0) write_event((char*)"dvcsD_prot",N,ivar[0]);        // pass the number of particles in the event to the function      
	  else if (neutron_flag == 1) write_event((char*)"dvcsD_neut",N,ivar[0]);
	  else cout << "Weird, active nucleon seems to be neither the proton nor the neutorn! Event number: " << lce << endl;
	  ce++;   // increment global event counter
	  lce++;  // also the local, events-per-file counter
	} 
	p++;  // increment particle counter per event
      }
      
    }
  }
  else cout << "Crap, no " <<  filename << " found!" << endl;
  
  file.close();
  
  cout << "\t In this file, number of events: " << lce << endl;
  cout << "\t Number of events with active proton: " << cp << endl;
  cout << "\t Number of events with active neutron: " << cn << "\n" << endl;
  
}

/****************************************/

void write_event(char *outname, int N, int npar){
  
  char fullname[100];
  sprintf(fullname,"%s_%d.dat",outname,N);   // append the file number to the end of the file name
  
  ofstream outfile;
  outfile.open(fullname, std::fstream::app);     // opens existing file and add anything new to the end of it
  
  outfile << std::setprecision(6) << std::fixed << ivar[0] << " " <<  ivar[1] << " " << ivar[2] << " " << ivar[3] << " " << ivar[4] << " " << ivar[5] << " " << dvar[0] << " " << ivar[6] << " " << ivar[7] << " " << dvar[1] << endl;
  
  for (int p=0; p<npar; p++){
    
    outfile << std::setprecision(8) << std::fixed << ipar[p][0] << " " << ipar[p][1] << " " << ipar[p][2] << " " << ipar[p][3] << " " << ipar[p][4] << "  " << ipar[p][5] << "   " << dpar[p][0] << "    " << dpar[p][1] << "    " << dpar[p][2] << "    " << dpar[p][3] << "    " << dpar[p][4] << "   " << dpar[p][5] << "   " << dpar[p][6] << "   " << dpar[p][7] << endl;
    
  }
  
  outfile.close();

}

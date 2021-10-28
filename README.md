# parsehepmc


Macro to parse TCS HEPMC3 files, generated either with EpIC or with the ToyMC for TCS, and save all the generated particles as four-momenta in an output ROOT file, the name of which is chosen by the user.

IMPORTANT: 
The electron helicity is set manually currently -- adjust it according to how the files are read in through the list. Assumes each file has a constant helicity.

FLAGS TO SET:
* Whether you're running it on a ToyMC file (default is EpIC), whether the file has been put through the afterburner to add crossing-angles (default is no).
* The debug flag lets everything that's been read in be printed to screen.
* If you only want to read in a certain number of events, you can set the flag for this and specify the number.

The code has been set up for files where the quasi-real photon had its code manually changed to 3 (from 1) in EpIC files. Once there's a formal change in EpIC, update    */
this feature.

To run, make a list of all HEPMC files you want to read in, eg: 
ls *.txt > filelist.txt

Then, run through ROOT:

     root -l    
     [] .L parse_hepmc.C   
     [] parse_hepmc("filelist.txt","output.root") 

or, to suppress the char warning, do this instead:

     [] parse_hepmc((char*)"filelist.txt",(char*)"output.root")    

Daria Sokhan, Paris-Saclay, Oct 2021 



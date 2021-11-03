# parse_hepmc


Macro to parse TCS HEPMC3 files, generated either with EpIC or with the ToyMC for TCS, and save all the generated particles as four-momenta in an output ROOT file, the name of which is chosen by the user.

IMPORTANT: 
The electron helicity is set manually currently -- adjust it according to how the files are read in through the list. Assumes each file has a constant helicity.

FLAGS TO SET:
* Whether you're running it on a ToyMC file (default is EpIC), whether the file has been put through the afterburner to add crossing-angles (default is no).
* The debug flag lets everything that's been read in be printed to screen.
* If you only want to read in a certain number of events, you can set the flag for this and specify the number.

The code has been set up for files where the quasi-real photon had its code manually changed to 3 (from 1) in EpIC files. Once there's a formal change in EpIC, update this feature.

To run, make a list of all HEPMC files you want to read in, eg: 
ls *.txt > filelist.txt

Then, run through ROOT:

     root -l    
     [] .L parse_hepmc.C   
     [] parse_hepmc("filelist.txt","output.root") 

or, to suppress the char warning, do this instead:

     [] parse_hepmc((char*)"filelist.txt",(char*)"output.root")    

Daria Sokhan, Paris-Saclay, Oct 2021 


# split_lundfile


Macro to read in a list of generated files for dvcs on proton and neutron in deuteron and write out two new sets of files which contain only dvcs on the proton or only on the neutron.

The output file names are hard-coded:

        dvcsD_neut_N.dat                         
        dvcsD_prot_N.dat                         
                                                 
where N is the number of the file. The files will get numbered from 0 in the order in which the input files are read in. For example, if you run this macro on two deuteron LUND files called: 
 deut_764.dat and
deut_825.dat, 
in that order, the output will be: 
 dvcsD_neut_0.dat and dvcs_prot_0.dat,
both corresponding to deut_764.dat, and
dvcsD_neut_1.dat and dvcs_prot_1.dat,
both corresponding to deut_825.dat.

To run, make a list of all LUND files you want to read in, eg: 
 ls *.dat > filelist.txt 

Run through ROOT:   
        
        root -l   
       [] .L split_lundfile.C
       [] split_lundfile((char*)"filelist.txt")

You can also run without the (char*) above, but you'll get a harmless warning.

Daria Sokhan, Saclay, Nov 2021 



# root_from_lund

Macro to read in a list of generated LUND files for dvmp pi0-production on proton or neutron in deuteron and write out a ROOT file with the four-momenta of all the particles, the beam energy and the cross-section per event. You pick the name of the output root file and pass it to the function. 

Set up specifically for pi0 production on deuteron -- to use it for another channel will require edits to the code.

Run through ROOT:   
        
        root -l   
       [] .L root_from_lund.C
       [] root_from_lund((char*)"filelist.txt",(char*)"outrootfile.root")

You can also run without the (char*) above, but you'll get a harmless warning.

Daria Sokhan, Saclay, Nov 2021 

#pragma ident "$Id$"

//============================================================================
//
// pppgnss Positioning
//
// This program shows how to use GNSS Data Structures (GDS) and other classes
// to build a reasonable complete application that computes "Precise Point
// Positioning" (pppgnss).
//
// For details on the pppgnss algorithm please consult:
//
//    Kouba, J. and P. Heroux. "Precise Point Positioning using IGS Orbit
//       and Clock Products". GPS Solutions, vol 5, pp 2-28. October, 2001.
//
// Copyright 
//
// Dagoberto Salazar - gAGE ( http://www.gage.es ). 2008, 2009
//
// Shoujian Zhang, Wuhan University, 2015
//
// Modifications
//
// 2010/10/01
//
// Create this program
//
// 2015/11/18
//
// if rinex header is not valid, then skip the rinex files, and then continue
// processing the other files.
//
// 2016/06/15
//
// add BDS/GAL/GLO system by wei wang
//
//============================================================================



// Basic input/output C++ classes
#include <iostream>
#include <iomanip>
#include <fstream>


   // Basic framework for programs in the GPSTk. 'process()' method MUST
   // be implemented
#include "BasicFramework.hpp"

   // Class for handling observation RINEX files
#include "Rinex3ObsStream.hpp"

   // Class for handling navigation RINEX files
#include "Rinex3NavStream.hpp"

   // Class to store Glonass navigation data
#include "GloEphemerisStore.hpp"

   // Class to store satellite precise navigation data
#include "SP3EphemerisStore.hpp"

   // Class to store a list of processing objects
#include "ProcessingList.hpp"

   // Class in charge of basic GNSS signal modelling
#include "BasicModel.hpp"

   // Class to model the tropospheric delays
#include "TropModel.hpp"

   // Class defining the GNSS data structures
#include "DataStructures.hpp"

   // Class to filter out satellites without required observables
#include "RequireObservables.hpp"

   // Class to filter out observables grossly out of limits
#include "SimpleFilter.hpp"

   // Class for easily changing reference base from ECEF to NEU
#include "XYZ2NEU.hpp"

   // Class to detect cycle slips using LI combination
#include "LICSDetector.hpp"

   // Class to detect cycle slips using the Melbourne-Wubbena combination
#include "MWCSDetector.hpp"

   // Class to compute the effect of solid tides
#include "SolidTides.hpp"

   // Class to compute the effect of ocean loading
#include "OceanLoading.hpp"

   // Class to compute the effect of pole tides
#include "PoleTides.hpp"

   // Class to correct observables
#include "CorrectObservables.hpp"

   // Classes to deal with Antex antenna parameters
#include "Antenna.hpp"
#include "AntexReader.hpp"

   // Class to compute the effect of wind-up
#include "ComputeWindUp.hpp"

   // Class to compute the effect of satellite antenna phase center
#include "ComputeSatPCenter.hpp"

   // Class to compute the tropospheric data
#include "ComputeTropModel.hpp"

   // Class to compute linear combinations
#include "ComputeLinear.hpp"

   // This class pre-defines several handy linear combinations
#include "LinearCombinations.hpp"

   // Class to compute Dilution Of Precision values
#include "ComputeDOP.hpp"

   // Class to keep track of satellite arcs
#include "SatArcMarker.hpp"

   // Class to keep track of satellite arcs
#include "SatArcMarker2.hpp"

   // Class to compute gravitational delays
#include "GravitationalDelay.hpp"

   // Class to align phases with code measurements
#include "PhaseCodeAlignment.hpp"

   // Used to delete satellites in eclipse
#include "EclipsedSatFilter.hpp"

   // Used to decimate data. This is important because RINEX observation
   // data is provided with a 30 s sample rate, whereas SP3 files provide
   // satellite clock information with a 900 s sample rate.
#include "Decimate.hpp"

   // Class to compute the Precise Point Positioning (pppgnss) solution in
   // forwards-only mode.
#include "SolverPPPGNSS.hpp"

   // Class to filter the MW combination
#include "MWFilter.hpp"

   // Class to read configuration files.
#include "ConfDataReader.hpp"

   // Class to compute the elevation weights
#include "ComputeElevWeights.hpp"

   // Class to store satellite precise navigation data
#include "MSCStore.hpp"

   // Class to convert the CC to NONCC.
#include "CC2NONCC.hpp"

  // Class to read and store the receiver type.
#include "RecTypeDataReader.hpp"


using namespace std;
using namespace gpstk;
using namespace gpstk::StringUtils;


   // A new class is declared that will handle program behaviour
   // This class inherits from BasicFramework
class pppgnss : public gpstk::BasicFramework
{
public:

      // Constructor declaration
   pppgnss(char* arg0);


protected:


      // Method that will take care of processing
   virtual void process();

      // Method that hold code to be run BEFORE processing
   virtual void spinUp();


private:


      // This field represents an option at command line interface (CLI)
   CommandOptionWithArg confFile;

      // Option for rinex file list
   CommandOptionWithAnyArg rnxFileListOpt;

      // Option for sp3 file list
   CommandOptionWithAnyArg sp3FileListOpt;

      // Option for Glonass navigation file list
   CommandOptionWithAnyArg GloNavFileListOpt;

      // Option for clk file list
   CommandOptionWithAnyArg clkFileListOpt;

      // Option for eop file list
   CommandOptionWithAnyArg eopFileListOpt;

      // Option for monitor coordinate file
   CommandOptionWithAnyArg mscFileOpt;

      // Option for p1c1 dcb file
   CommandOptionWithAnyArg dcbFileListOpt;

      // Option for monitor coordinate file
   CommandOptionWithAnyArg outputFileListOpt;

      // If you want to share objects and variables among methods, you'd
      // better declare them here
   
   string rnxFileListName;
   string sp3FileListName;
   string GloNavFileListName;
   string clkFileListName;
   string eopFileListName;
   string mscFileName;
   string dcbFileListName;
   string outputFileListName;

      // Configuration file reader
   ConfDataReader confReader;


      // Declare our own methods to handle output


      // Method to print solution values
   void printSolution( ofstream& outfile,
                       const  SolverLMS& solver,
                       const  CommonTime& time,
                       const  ComputeDOP& cDOP,
                       bool   useNEU,
                       bool   usingGPS,
                       bool   usingGlonass,
                       bool   usingBeiDou,
                       bool   usingGalileo,
                       int    numSats,
                       double dryTropo,
                       int    precision = 3 );


      // Method to print model values
   void printModel( ofstream& modelfile,
                    const gnssRinex& gData,
                    int   precision = 4 );


}; // End of 'pppgnss' class declaration



   // Let's implement constructor details
pppgnss::pppgnss(char* arg0)
   :
   gpstk::BasicFramework(  arg0,
"\nThis program reads parameters from a configuration file, \n"
"reads GPS receiver data and ephemeris data from command line, \n"
"then process the data using the pppgnss strategy.\n\n"
"Please consult the default configuration file, 'pppgnss.conf', for \n"
"further details.\n\n"
"The output file format is as follows:\n"
"\n 1) Year"
"\n 2) doy"
"\n 3) Seconds of day"
"\n 4) dx/dLat (m)"
"\n 5) dy/dLon (m)"
"\n 6) dz/dH (m)"
"\n 7) Zenital Tropospheric Delay (zpd) (m)"
"\n 8) Number of satellites"
"\n 9) GDOP"
"\n10) PDOP\n"),
      // Option initialization. "true" means a mandatory option
   confFile( CommandOption::stdType,
             'c',
             "conffile",
   "Name of configuration file ('pppgnss.conf' by default).",
             false),
      // Option for rinex file list reading
   rnxFileListOpt( 'r',
                   "rnxFileList",
   "file storing a list of rinex file name ",
                   true),
   sp3FileListOpt( 's',
                   "sp3FileList",
   "file storing a list of rinex SP3 file name ",
                   true),
   GloNavFileListOpt( 'g',
                   "GloNavFileList",
   "file storing a list of Glonass navigation file name ",
                   false),
   clkFileListOpt( 'k',
                   "clkFileList",
   "file storing a list of rinex clk file name ",
                   false),
   eopFileListOpt( 'e',
                   "eopFileList",
   "file storing a list of IGS erp file name ",
                   true),
   outputFileListOpt( 'o',
                   "outputFileList",
   "file storing the list of output file name ",
                   false),
   mscFileOpt( 'm',
               "mscFile",
   "file storing monitor station coordinates ",
               true),
   dcbFileListOpt( 'D',
               "dcbFile",
   "file storing P1-C1 DCB ",
               false)
{

      // This option may appear just once at CLI
   confFile.setMaxCount(1);

}  // End of 'pppgnss::pppgnss'



   // Method to print solution values
void pppgnss::printSolution( ofstream& outfile,
                              const SolverLMS& solver,
                              const CommonTime& time,
                              const ComputeDOP& cDOP,
                              bool  useNEU,
                              bool  useGPS,
                              bool  useGlonass,
                              bool  useBeiDou,
                              bool  useGalileo,
                              int   numSats,
                              double dryTropo,
                              int   precision )
{

      // Prepare for printing
   outfile << fixed << setprecision( precision );


      // Print results
   outfile << static_cast<YDSTime>(time).year        << "  ";    // Year           - #1
   outfile << setw(5) << static_cast<YDSTime>(time).doy         << "  ";    // DayOfYear      - #2
   outfile << setw(12)<< static_cast<YDSTime>(time).sod   << "  ";    // SecondsOfDay   - #3

   if( useNEU )
   {

      outfile<< setw(8) << solver.getSolution(TypeID::dLat) << "  ";       // dLat  - #4
      outfile<< setw(8) << solver.getSolution(TypeID::dLon) << "  ";       // dLon  - #5
      outfile<< setw(8) << solver.getSolution(TypeID::dH) << "  ";         // dH    - #6

         // We add 0.1 meters to 'wetMap' because 'NeillTropModel' sets a
         // nominal value of 0.1 m. Also to get the total we have to add the
         // dry tropospheric delay value
                                                                 // ztd - #7
      outfile<< setw(8) << solver.getSolution(TypeID::wetMap) + 0.1 + dryTropo << "  ";


   }
   else
   {

      outfile << solver.getSolution(TypeID::dx) << "  ";         // dx    - #4
      outfile << solver.getSolution(TypeID::dy) << "  ";         // dy    - #5
      outfile << solver.getSolution(TypeID::dz) << "  ";         // dz    - #6

         // We add 0.1 meters to 'wetMap' because 'NeillTropModel' sets a
         // nominal value of 0.1 m. Also to get the total we have to add the
         // dry tropospheric delay value
                                                                 // ztd - #7
      outfile << solver.getSolution(TypeID::wetMap) + 0.1 + dryTropo << "  ";


   }

   if (useGPS && useBeiDou)
   {
      double ISB_BDS = 0.0;
      try
      {
          ISB_BDS = solver.getSolution(TypeID::ISB_BDS);
      }   
      catch (...)
      {
          // do nothing 
      } 
           // now the unit of ISB is ns
      outfile<< setw(8) << ISB_BDS*3.3352 << "  ";
   }

   if (useGPS && useGalileo)
   {
      double ISB_GAL = 0.0;
      try
      {
          ISB_GAL = solver.getSolution(TypeID::ISB_GAL);
      }   
      catch (...)
      {
          // do nothing 
      } 

      outfile<< setw(8) << ISB_GAL*3.3352 << "  ";
   }

   outfile << numSats << "  ";    // Number of satellites - #12
   outfile << solver.getConverged() << "  "; 

   outfile << cDOP.getGDOP()        << "  ";  // GDOP - #13
   outfile << cDOP.getPDOP()        << "  ";  // PDOP - #14

      // Add end-of-line
   outfile << endl;


   return;


}  // End of method 'pppgnss::printSolution()'



   // Method to print model values
void pppgnss::printModel( ofstream& modelfile,
                           const gnssRinex& gData,
                           int   precision )
{

      // Prepare for printing
   modelfile << fixed << setprecision( precision );

      // Get epoch out of GDS
   CommonTime time(gData.header.epoch);

      // Iterate through the GNSS Data Structure
   for ( satTypeValueMap::const_iterator it = gData.body.begin();
         it!= gData.body.end();
         it++ )
   {

         // Print epoch
      modelfile << static_cast<YDSTime>(time).year         << "  ";    // Year           #1
      modelfile << static_cast<YDSTime>(time).doy          << "  ";    // DayOfYear      #2
      modelfile << static_cast<YDSTime>(time).sod          << "  ";    // SecondsOfDay   #3

         // Print satellite information (Satellite system and ID number)
      modelfile << (*it).first << " ";             // System         #4
                                                   // ID number      #5

         // Print model values
      for( typeValueMap::const_iterator itObs  = (*it).second.begin();
           itObs != (*it).second.end();
           itObs++ )
      {
            // Print type names and values
         modelfile << (*itObs).first << " ";
         modelfile << (*itObs).second << " ";

      }  // End of 'for( typeValueMap::const_iterator itObs = ...'

      modelfile << endl;

   }  // End for (it = gData.body.begin(); ... )

}  // End of method 'pppgnss::printModel()'





   // Method that will be executed AFTER initialization but BEFORE processing
void pppgnss::spinUp()
{

      // Check if the user provided a configuration file name
   if ( confFile.getCount() > 0 )
   {

         // Enable exceptions
      confReader.exceptions(ios::failbit);

      try
      {

            // Try to open the provided configuration file
         confReader.open( confFile.getValue()[0] );

      }
      catch(...)
      {

         cerr << "Problem opening file "
              << confFile.getValue()[0]
              << endl;
         cerr << "Maybe it doesn't exist or you don't have proper "
              << "read permissions." << endl;

         exit (-1);

      }  // End of 'try-catch' block

   }
   else
   {

      try
      {
            // Try to open default configuration file
         confReader.open( "pppgnss.conf" );
      }
      catch(...)
      {

         cerr << "Problem opening default configuration file 'ppp.conf'"
              << endl;
         cerr << "Maybe it doesn't exist or you don't have proper read "
              << "permissions. Try providing a configuration file with "
              << "option '-c'."
              << endl;

         exit (-1);

      }  // End of 'try-catch' block

   }  // End of 'if ( confFile.getCount() > 0 )'

      // If a given variable is not found in the provided section, then
      // 'confReader' will look for it in the 'DEFAULT' section.
   confReader.setFallback2Default(true);

      // Now, Let's parse the command line
   if(rnxFileListOpt.getCount())
   {
      rnxFileListName = rnxFileListOpt.getValue()[0];
   }
   if(sp3FileListOpt.getCount())
   {
      sp3FileListName = sp3FileListOpt.getValue()[0];
   }
   if(GloNavFileListOpt.getCount())
   {
      GloNavFileListName = GloNavFileListOpt.getValue()[0];
   }
   if(clkFileListOpt.getCount())
   {
      clkFileListName = clkFileListOpt.getValue()[0];
   }
   if(eopFileListOpt.getCount())
   {
      eopFileListName = eopFileListOpt.getValue()[0];
   }
   if(outputFileListOpt.getCount())
   {
      outputFileListName = outputFileListOpt.getValue()[0];
   }
   if(mscFileOpt.getCount())
   {
      mscFileName = mscFileOpt.getValue()[0];
   }
   if(dcbFileListOpt.getCount())
   {
      dcbFileListName = dcbFileListOpt.getValue()[0];
   }

}  // End of method 'pppgnss::spinUp()'



   // Method that will really process information
void pppgnss::process()
{
      //**********************************************
      // Now, Let's read SP3 Files
      //**********************************************

      // Declare a "SP3EphemerisStore" object to handle precise ephemeris
   SP3EphemerisStore SP3EphList;

      // Set flags to reject satellites with bad or absent positional
      // values or clocks
   SP3EphList.rejectBadPositions(true);
   SP3EphList.rejectBadClocks(true);
      // Set clock max interval ( 5min )
   SP3EphList.setClockMaxInterval(300.0);

      // Now read sp3 files from 'sp3FileList'
   ifstream sp3FileListStream;

   sp3FileListStream.open(sp3FileListName.c_str(), ios::in);
   if(!sp3FileListStream)
   {
         // If file doesn't exist, issue a warning
      cerr << "SP3 file List Name'" << sp3FileListName << "' doesn't exist or you don't "
           << "have permission to read it. Skipping it." << endl;

      exit(-1);
   }

   string sp3File;
   while( sp3FileListStream >> sp3File )
   {
      try
      {
         SP3EphList.loadFile( sp3File );
      }
      catch (FileMissingException& e)
      {
            // If file doesn't exist, issue a warning
         cerr << "SP3 file '" << sp3File << "' doesn't exist or you don't "
              << "have permission to read it. Skipping it." << endl;
         continue;
      }
   }
      // Close file
   sp3FileListStream.close();

      //***********************
      // Let's read Glonass navigation files
      //***********************
   
   Rinex3NavStream rNavStream;
   rNavStream.exceptions(ios::failbit); // Enable exception

   Rinex3NavData rNavData;
       // Container to store the Glonass satellite frequency number 
   std::map<RinexSatID,int>  FreqNumber;
 
   if(GloNavFileListOpt.getCount())
   {
         // Now read glonass navigation files 
      ifstream GloNavFileListStream;

         // Open GloNavFileList File
      GloNavFileListStream.open(GloNavFileListName.c_str(), ios::in);
      if(!GloNavFileListStream)
      {
            // If file doesn't exist, issue a warning
         cerr << "glonass navigation file List Name'" << GloNavFileListName << "' doesn't exist or you don't "
              << "have permission to read it. Skipping it." << endl;

         exit(-1);
      }

      string GloNavFile;
      while( GloNavFileListStream >> GloNavFile )
      {
         try
         {     
             
            rNavStream.open(GloNavFile.c_str(),std::ios::in);
         }
         catch (...)
         {
               // If file doesn't exist, issue a warning
            cerr << "Problem opening file '" << GloNavFile <<"'."<< endl;
            cerr << "Maybe it doesn't exist or you don't "
                 << "have permission to read it." << endl;

            exit(-1);
         }
                  // read Glonass Nav File
          while (rNavStream >> rNavData)
          {
                  // initilize the Glophemeis
               GloEphemeris GloEph(rNavData);   
                  // get PRN ID
               int prn(GloEph.getPRNID());
                  // get frequency number
               int freqNo(GloEph.getfreqNum());
                  // insert into map
               RinexSatID sat(prn,SatID::systemGlonass);

               FreqNumber[sat] = freqNo;
           }

           rNavStream.close();
      }

         // Close file
      GloNavFileListStream.close();

   }  // End of 'if(...)'

      //***********************
      // Let's read clock files
      //***********************

      // If rinex clock file list is given, then use rinex clock
   if(clkFileListOpt.getCount())
   {
         // Now read clk files from 'clkFileList'
      ifstream clkFileListStream;

         // Open clkFileList File
      clkFileListStream.open(clkFileListName.c_str(), ios::in);
      if(!clkFileListStream)
      {
            // If file doesn't exist, issue a warning
         cerr << "clock file List Name'" << clkFileListName << "' doesn't exist or you don't "
              << "have permission to read it. Skipping it." << endl;

         exit(-1);
      }

      string clkFile;
      while( clkFileListStream >> clkFile )
      {
         try
         {
            SP3EphList.loadRinexClockFile( clkFile );
         }
         catch (FileMissingException& e)
         {
               // If file doesn't exist, issue a warning
            cerr << "rinex CLK file '" << clkFile << "' doesn't exist or you don't "
                 << "have permission to read it. Skipping it." << endl;
            continue;
         }
      }

         // Close file
      clkFileListStream.close();

   }  // End of 'if(...)'

      //***********************
      // Let's read ocean loading BLQ data files
      //***********************

      // BLQ data store object
   BLQDataReader blqStore;

      // Read BLQ file name from the configure file
   string blqFile = confReader.getValue( "oceanLoadingFile", "DEFAULT");

   try
   {
      blqStore.open( blqFile );
   }
   catch (FileMissingException& e)
   {
         // If file doesn't exist, issue a warning
      cerr << "BLQ file '" << blqFile << "' doesn't exist or you don't "
           << "have permission to read it. Skipping it." << endl;
      exit(-1);
   }

      //***********************
      // Let's read eop files
      //***********************

      // Declare a "EOPDataStore" object to handle earth rotation parameter file
   EOPDataStore eopStore;

      // Now read eop files from 'eopFileList'
   ifstream eopFileListStream;

      // Open eopFileList File
   eopFileListStream.open(eopFileListName.c_str(), ios::in);
   if(!eopFileListStream)
   {
         // If file doesn't exist, issue a warning
      cerr << "erp file List Name'" << eopFileListName << "' doesn't exist or you don't "
           << "have permission to read it. Skipping it." << endl;

      exit(-1);
   }

   string eopFile;
   while( eopFileListStream >> eopFile )
   {
      try
      {
         eopStore.loadIGSFile( eopFile );
      }
      catch (FileMissingException& e)
      {
            // If file doesn't exist, issue a warning
         cerr << "EOP file '" << eopFile << "' doesn't exist or you don't "
              << "have permission to read it. Skipping it." << endl;
         continue;
      }
   }
      // Close file
   eopFileListStream.close();

      //**********************************************
      // Now, Let's read MSC data
      //**********************************************
      
      // Declare a "MSCStore" object to handle msc file 
   MSCStore mscStore;

   try
   {
      mscStore.loadFile( mscFileName );
   }
   catch (FileMissingException& e)
   {
         // If file doesn't exist, issue a warning
      cerr << "MSC file '" << mscFileName << "' doesn't exist or you don't "
           << "have permission to read it. Skipping it." << endl;
      exit(-1);
   }

      //***********************
      // Let's read DCB files
      //***********************

      // Read and store dcb data
   DCBDataReader dcbStore;

   if(dcbFileListOpt.getCount() )
   {
         // Now read dcb file from 'dcbFileName'
      ifstream dcbFileListStream;

         // Open dcbFileList File
      dcbFileListStream.open(dcbFileListName.c_str(), ios::in);
      if(!dcbFileListStream)
      {
            // If file doesn't exist, issue a warning
         cerr << "dcb file List Name '" << dcbFileListName << "' doesn't exist or you don't "
              << "have permission to read it." << endl;
         exit(-1);
      }

      string dcbFile;

         // Here is just a dcb file, we only read one month's dcb data.
      while(dcbFileListStream >> dcbFile)
      {
         try
         {
            dcbStore.open(dcbFile);
         }
         catch(FileMissingException e)
         {
            cerr << "Warning! The DCB file '"<< dcbFile <<"' does not exist!" 
                 << endl;
            exit(-1);
         }
      };

      dcbFileListStream.close();
   }    

      //**********************************************************
      // Now, Let's perform the pppgnss for each rinex files
      //**********************************************************

   vector<string> rnxFileListVec;
      
      // Now read eop files from 'eopFileList'
   ifstream rnxFileListStream;

      // Open eopFileList File
   rnxFileListStream.open(rnxFileListName.c_str());
   if(!rnxFileListStream)
   {
         // If file doesn't exist, issue a warning
      cerr << "rinex file List Name'" << rnxFileListName << "' doesn't exist or you don't "
           << "have permission to read it. Skipping it." << endl;

      exit(-1);
   }

   string rnxFile;
   while( rnxFileListStream >> rnxFile )
   {
      rnxFileListVec.push_back(rnxFile);
   }
      // Close file
   rnxFileListStream.close();

   if(rnxFileListVec.size() == 0 )
   {
      cerr << rnxFileListName  << "rnxFileList is empty!! "
           << endl;
   }

      //////////////////////////////////////
      // read output file list name
      //////////////////////////////////////
      
   vector<string> outputFileListVec;
      
      // Now read eop files from 'eopFileList'
   ifstream outputFileListStream;

   if(outputFileListOpt.getCount())
   {
         // Open ouputFileList File
      outputFileListStream.open(outputFileListName.c_str());
      if( ! outputFileListStream.is_open() )
      {
            // If file doesn't exist, issue a warning
         cerr << "output file List Name'" << outputFileListName << "' doesn't exist or you don't "
              << "have permission to read it. Skipping it." << endl;

         exit(-1);
      }

      string outputFile;
      while( outputFileListStream >> outputFile )
      {
         outputFileListVec.push_back(outputFile);
      }
         // Close file
      outputFileListStream.close();

      if(outputFileListVec.size() == 0 )
      {
         cerr << outputFileListName  << "outputFileList is empty!! "
              << endl;
      }
   }

         // ===================
         // Let's read rinex file list !!!!
         // ===================

      // We will read each rinex file
   vector<string>::const_iterator rnxit = rnxFileListVec.begin();
   vector<string>::const_iterator outit = outputFileListVec.begin();

   while( rnxit != rnxFileListVec.end() )
   {
         // Read rinex file from the vector!
      string rnxFile = (*rnxit);

         // Create input observation file stream
      Rinex3ObsStream rin;
      rin.exceptions(ios::failbit); // Enable exceptions

         // Try to open Rinex observations file
      try
      {
         rin.open( rnxFile, std::ios::in );
      }
      catch(...)
      {
         cerr << "Problem opening file '"
              << rnxFile
              << "'." << endl;
         cerr << "Maybe it doesn't exist or you don't have "
              << "proper read permissions."
              << endl;

         cerr << "Skipping receiver '" << rnxFile << "'."
              << endl;

            // Close current Rinex observation stream
         rin.close();

           // Index for rinex file iterator.
         ++rnxit;
            // Index for output file iterator.
         if(outputFileListOpt.getCount())
         {
            ++outit;
         }

         continue;

      }  // End of 'try-catch' block

         
         // ===================
         // Let's read the header firstly!!!!
         // ===================
      Rinex3ObsHeader roh;
      try
      {
         rin >> roh;
      }
      catch(FFStreamError& e)
      {
         cerr << "Problem in reading file '"
              << rnxFile
              << "'." << endl;

         cerr << "Skipping receiver '" << rnxFile << "'."
              << endl;

            // Close current Rinex observation stream
         rin.close();

            // Index for rinex file iterator.
         ++rnxit;
            // Index for output file iterator.
         if(outputFileListOpt.getCount())
         {
            ++outit;
         }

         continue;
      }

         // Get the station name for current rinex file 
      string station = roh.markerName;
         // First time for this rinex file
      CommonTime initialTime( roh.firstObs ) ;
      
         // Let's check the ocean loading data for current station before
         // the real data processing.
      if( ! blqStore.isValid(station) )
      {
         cout << "There is no BLQ data for current station: " << station << endl;
         cout << "Skipping this station!" << endl;

            // Close current Rinex observation stream
         rin.close();
            // Index for rinex file iterator.
         ++rnxit;
            // Index for output file iterator.
         if(outputFileListOpt.getCount())
         {
            ++outit;
         }

         continue;
      }

         // Show a message indicating that we are starting with this station
      cout << "Starting processing for station: '" << station << "'." << endl;

         // MSC data for this station
      initialTime.setTimeSystem(TimeSystem::Unknown);
      MSCData mscData;
      try
      {
         mscData = mscStore.findMSC( station, initialTime );
      }
      catch (InvalidRequest& ie)
      {
                // If file doesn't exist, issue a warning
         cerr << "The station " << station 
              << " isn't included in MSC file." << endl;

         ++rnxit;
         if(outputFileListOpt.getCount())
         {
            ++outit;
         }
         continue;
      }

      initialTime.setTimeSystem(TimeSystem::GPS);

         // The former peculiar code is possible because each time we
         // call a 'fetchListValue' method, it takes out the first element
         // and deletes it from the given variable list.

      Position nominalPos( mscData.coordinates );

         // Create a 'ProcessingList' object where we'll store
         // the processing objects in order
      ProcessingList pList;

      bool usingGPS     (confReader.getValueAsBoolean( "useGPS" ));
      bool usingGlonass (confReader.getValueAsBoolean( "useGlonass" ));
      bool usingGalileo (confReader.getValueAsBoolean( "useGalileo" ));
      bool usingBeiDou  (confReader.getValueAsBoolean( "useBeiDou" ));

          // Declare a CC2NONCC object
      CC2NONCC cc2noncc(dcbStore);
         // Read the receiver type file.
      cc2noncc.loadRecTypeFile( confReader.getValue("recTypeFile"));
         // warning: change receiver type to upper case, if not,
         // some receiver type(lower case) can not be find in receiver_bernese.lis
      cc2noncc.setRecType(upperCase(roh.recType));
      cc2noncc.setCopyC1ToP1(true);

         // Add to processing list
      pList.push_back(cc2noncc);

         // This object will check that all required observables are present
      RequireObservables requireObs;
          
             // for GPS 
      if (usingGPS)
      {
        requireObs.addRequiredType(TypeID::P1); 
        requireObs.addRequiredType(TypeID::P2);    
        requireObs.addRequiredType(TypeID::L1);    
        requireObs.addRequiredType(TypeID::L2);    
      }
             // for Glonass     
      if (usingGlonass)
      {
        requireObs.addGLORequiredType(TypeID::P1); 
        requireObs.addGLORequiredType(TypeID::P2);    
        requireObs.addGLORequiredType(TypeID::L1);    
        requireObs.addGLORequiredType(TypeID::L2);    
      }

      if (usingGalileo)
      {   
           // for Galileo E1/E5a
        requireObs.addGALRequiredType(TypeID::C1);    
        requireObs.addGALRequiredType(TypeID::C5);    
        requireObs.addGALRequiredType(TypeID::L1);    
        requireObs.addGALRequiredType(TypeID::L5);    
      }
      
      if (usingBeiDou)
      {
           // for BeiDou B1/B2
        requireObs.addBDSRequiredType(TypeID::C2);    
        requireObs.addBDSRequiredType(TypeID::C7);    
        requireObs.addBDSRequiredType(TypeID::L2);    
        requireObs.addBDSRequiredType(TypeID::L7); 
      }
         // This object will check that code observations are within
         // reasonable limits
      SimpleFilter pObsFilter;

      if ( usingGPS )
      {
         pObsFilter.addFilteredType(TypeID::P1);
         pObsFilter.addFilteredType(TypeID::P2);
    
      }
         // for Glonass
      if (usingGlonass)
      {
         pObsFilter.addGLOFilteredType(TypeID::P1);
         pObsFilter.addGLOFilteredType(TypeID::P2);
      }

      if (usingGalileo)
      {
          // for Galileo
         pObsFilter.addGALFilteredType(TypeID::C1);
         pObsFilter.addGALFilteredType(TypeID::C5);
      }

      if (usingBeiDou)
      {
         // for BeiDou
        pObsFilter.addBDSFilteredType(TypeID::C2);
        pObsFilter.addBDSFilteredType(TypeID::C7);
      }

         // Add 'requireObs' to processing list (it is the first)
      pList.push_back(requireObs);

         // IMPORTANT NOTE:
         // It turns out that some receivers don't correct their clocks
         // from drift.
         // When this happens, their code observations may drift well beyond
         // what it is usually expected from a pseudorange. In turn, this
         // effect causes that "SimpleFilter" objects start to reject a lot of
         // satellites.
         // Thence, the "filterCode" option allows you to deactivate the
         // "SimpleFilter" object that filters out C1, P1 and P2, in case you
         // need to.
      bool filterCode( confReader.getValueAsBoolean( "filterCode" ) );

         // Check if we are going to use this "SimpleFilter" object or not
      if( filterCode )
      {
         pList.push_back(pObsFilter);       // Add to processing list

      }


         // This object defines several handy linear combinations
      LinearCombinations comb;


         // Object to compute linear combinations for cycle slip detection
      ComputeLinear linear1;

      if (usingGPS)
      {
         linear1.addLinear(comb.mwubbenaCombination);
         linear1.addLinear(comb.liCombination);
      }

          // for Glonass
      if (usingGlonass)
      {
         linear1.addGlonassLinear(comb.mwubbenaCombForGlonass);
         linear1.addGlonassLinear(comb.liCombination);
      } 
          
          // for Galileo          
      if (usingGalileo)
      {
         linear1.addGalileoLinear(comb.mwubbenaCombForGalileo);
         linear1.addGalileoLinear(comb.liCombForGalileo);
      } 
         // for BeiDou
      if (usingBeiDou)
      {
         linear1.addBeiDouLinear(comb.mwubbenaCombForBeiDou);
         linear1.addBeiDouLinear(comb.liCombForBeiDou);
      }

      pList.push_back(linear1);       // Add to processing list

         // Objects to mark cycle slips
      LICSDetector markCSLI;          // Checks LI cycle slips
      pList.push_back(markCSLI);      // Add to processing list
      MWCSDetector markCSMW;          // Checks Merbourne-Wubbena cycle slips
      pList.push_back(markCSMW);      // Add to processing list


         // Object to keep track of satellite arcs
      SatArcMarker2 markArc;
      markArc.setDeleteUnstableSats(false);
      markArc.setUnstablePeriod(151.0);
      pList.push_back(markArc);       // Add to processing list


         // Object to decimate data
      Decimate decimateData(
               confReader.getValueAsDouble( "decimationInterval"),
               confReader.getValueAsDouble( "decimationTolerance"),
               initialTime );
      pList.push_back(decimateData);       // Add to processing list


         // Declare a basic modeler
      BasicModel basic(nominalPos, SP3EphList);
         // Set the minimum elevation
      basic.setMinElev(confReader.getValueAsDouble("cutOffElevation"));
         
         // for GPS and Glonass
      basic.setDefaultObservable(TypeID::P1);

         // Add to processing list
      pList.push_back(basic);


         // Object to compute weights based on elevation
      ComputeElevWeights elevWeights;
      pList.push_back(elevWeights);       // Add to processing list


         // Object to remove eclipsed satellites
      EclipsedSatFilter eclipsedSV;
      pList.push_back(eclipsedSV);       // Add to processing list


         // Object to compute gravitational delay effects
      GravitationalDelay grDelay(nominalPos);
      pList.push_back(grDelay);       // Add to processing list


         // Vector from monument to antenna ARP [UEN], in meters
      Triple offsetARP( roh.antennaDeltaHEN );

         // Declare some antenna-related variables
      Triple offsetL1( 0.0, 0.0, 0.0 ), offsetL2( 0.0, 0.0, 0.0 );
      AntexReader antexReader;
      Antenna receiverAntenna;

         // Check if we want to use Antex information
      bool useantex( confReader.getValueAsBoolean( "useAntex") );
      string antennaModel;
      if( useantex )
      {
            // Feed Antex reader object with Antex file
         antexReader.open( confReader.getValue( "antexFile" ) );

            // Antenna model 
         antennaModel = roh.antType;

            // Get receiver antenna parameters
            // Warning: If no corrections are not found for one specific 
            // radome, then the antenna with radome NONE are used.
         try
         {
            receiverAntenna = antexReader.getAntenna( antennaModel );
         }
         catch(ObjectNotFound& notFound)
         {
               // new antenna model
            antennaModel.replace(16,4,"NONE");
               // new receiver antenna with new antenna model
            receiverAntenna = antexReader.getAntenna( antennaModel );
         }

      }

         // Object to compute satellite antenna phase center effect
      ComputeSatPCenter svPcenter(nominalPos);
      if( useantex )
      {
            // Feed 'ComputeSatPCenter' object with 'AntexReader' object
         svPcenter.setAntexReader( antexReader );
      }

      pList.push_back(svPcenter);       // Add to processing list


         // Declare an object to correct observables to monument
      CorrectObservables corr(SP3EphList);
      corr.setNominalPosition(nominalPos);
      corr.setMonument( offsetARP );

         // Check if we want to use Antex patterns
      bool usepatterns(confReader.getValueAsBoolean("usePCPatterns" ));
      if( useantex && usepatterns )
      {
         corr.setAntenna( receiverAntenna );

            // Should we use elevation/azimuth patterns or just elevation?
         corr.setUseAzimuth(confReader.getValueAsBoolean("useAzim" ));
      }
      else
      {
            // Fill vector from antenna ARP to L1 phase center [UEN], in meters
         offsetL1[0] = confReader.fetchListValueAsDouble("offsetL1");
         offsetL1[1] = confReader.fetchListValueAsDouble("offsetL1");
         offsetL1[2] = confReader.fetchListValueAsDouble("offsetL1");

            // Vector from antenna ARP to L2 phase center [UEN], in meters
         offsetL2[0] = confReader.fetchListValueAsDouble("offsetL2");
         offsetL2[1] = confReader.fetchListValueAsDouble("offsetL2");
         offsetL2[2] = confReader.fetchListValueAsDouble("offsetL2");

         corr.setL1pc( offsetL1 );
         corr.setL2pc( offsetL2 );

      }

      pList.push_back(corr);       // Add to processing list
      
         // Object to compute wind-up effect
      ComputeWindUp windup( SP3EphList,
                            nominalPos);
      if( useantex )
      {
            // Feed 'ComputeSatPCenter' object with 'AntexReader' object
         windup.setAntexReader( antexReader );
      }

      pList.push_back(windup);       // Add to processing list


         // Declare a NeillTropModel object, setting its parameters
      NeillTropModel neillTM( nominalPos, 
                              initialTime );

         // We will need this value later for printing
      double drytropo( neillTM.dry_zenith_delay() );


         // Object to compute the tropospheric data
      ComputeTropModel computeTropo(neillTM);
      pList.push_back(computeTropo);       // Add to processing list


         // Object to compute ionosphere-free combinations to be used
         // as observables in the pppgnss processing
      ComputeLinear linear3;

      if ( usingGPS )
      
      {
         linear3.addLinear(comb.pcCombination);
         linear3.addLinear(comb.lcCombination);
      }

      if (usingGlonass)
      {
         linear3.addGlonassLinear(comb.pcCombForGlonass);
         linear3.addGlonassLinear(comb.lcCombForGlonass);
      }

      if (usingGalileo)
      {
         linear3.addGalileoLinear(comb.pcCombForGalileo);
         linear3.addGalileoLinear(comb.lcCombForGalileo);
      }
      if (usingBeiDou)
      {
         linear3.addBeiDouLinear(comb.pcCombForBeiDou);
         linear3.addBeiDouLinear(comb.lcCombForBeiDou);
      }
      pList.push_back(linear3);       // Add to processing list

         // Declare a simple filter object to screen PC
      SimpleFilter pcFilter;
      if (usingGPS)
      {
        pcFilter.setFilteredType(TypeID::PC);
      }
      if (usingGlonass)
      {
        pcFilter.addGLOFilteredType(TypeID::PC);
      }
      if (usingGalileo)
      {
        pcFilter.addGALFilteredType(TypeID::PC);
      }
      if (usingBeiDou)
      {
        pcFilter.addBDSFilteredType(TypeID::PC);
      }
         // IMPORTANT NOTE:
         // Like in the "filterCode" case, the "filterPC" option allows you to
         // deactivate the "SimpleFilter" object that filters out PC, in case
         // you need to.
      bool filterPC( confReader.getValueAsBoolean( "filterPC") );

         // Check if we are going to use this "SimpleFilter" object or not
      if( filterPC )
      {
         pList.push_back(pcFilter);       // Add to processing list
      }

        // Object to align phase with code measurements
      PhaseCodeAlignment phaseAlign;
      pList.push_back(phaseAlign); 
         // Object to compute prefit-residuals
      ComputeLinear linear4;
      if (usingGPS)
      {
        linear4.addLinear(comb.pcPrefit);
        linear4.addLinear(comb.lcPrefit);
      }
      
      if (usingGlonass)
      {
        linear4.addGlonassLinear(comb.lcPrefitForGlonass);
        linear4.addGlonassLinear(comb.pcPrefit);
      }

      if (usingGalileo)
      {
        linear4.addGalileoLinear(comb.lcPrefitForGalileo);
        linear4.addGalileoLinear(comb.pcPrefit);
      }

      if (usingBeiDou)
      {
        linear4.addBeiDouLinear(comb.lcPrefitForBeiDou);
        linear4.addBeiDouLinear(comb.pcPrefit);
      }

      pList.push_back(linear4);       // Add to processing list

         // Declare a base-changing object: From ECEF to North-East-Up (NEU)
      XYZ2NEU baseChange(nominalPos);
         // We always need both ECEF and NEU data for 'ComputeDOP', so add this
      pList.push_back(baseChange);


         // Object to compute DOP values
      ComputeDOP cDOP;
      pList.push_back(cDOP);       // Add to processing list

      
         // Get if we want results in ECEF or NEU reference system
      bool isNEU( confReader.getValueAsBoolean( "USENEU") );

         // Declare solver objects
      SolverPPPGNSS  pppgnssSolver( isNEU);
      pppgnssSolver.setSatSystem( usingGPS,
                                  usingGlonass,
                                  usingBeiDou,
                                  usingGalileo );

         // Get if we want 'forwards-backwards' or 'forwards' processing only
      int cycles( confReader.getValueAsInt("filterCycles") );

         // Get if we want to process coordinates as white noise
      bool isWN( confReader.getValueAsBoolean( "coordAsWhiteNoise") );

         // White noise stochastic model
      WhiteNoiseModel wnM(100.0);      // 100 m of sigma

            // Check about coordinates as white noise
      if ( isWN )
      {
               // Reconfigure solver
          pppgnssSolver.setCoordinatesModel(&wnM);
      }

            // Add solver to processing list
      pList.push_back(pppgnssSolver);


         // Object to compute tidal effects
      SolidTides solid;

         // Configure ocean loading model
      OceanLoading ocean(blqStore);

         // Object to model pole tides
      PoleTides pole(eopStore);


         // This is the GNSS data structure that will hold all the
         // GNSS-related information
      gnssRinex gRin;


         // Prepare for printing
      int precision( confReader.getValueAsInt( "precision" ) );


      string outputFileName;

         // Let's open the output file
      if( outputFileListOpt.getCount() )
      {
        outputFileName = (*outit);
      }
      else
      {
        outputFileName = ( rnxFile + ".out" );
      }

      ofstream outfile;
      outfile.open( outputFileName.c_str(), ios::out );

         // print out the header
      outfile << "# col  1 -  3: year/doy/sod \n" 
              << "# col  4 -  7: dLat/dLon/dH/ZTD \n" 
              << "# col  8 - 11: TotalSatNumber/Converged/GDOP/PDOP \n"
              << "# END OF HEADER" << endl;

         // Let's check if we are going to print the model
      bool printmodel( confReader.getValueAsBoolean( "printModel" ) );

      string modelName;
      ofstream modelfile;

         // Prepare for model printing
      if( printmodel )
      {
         modelName = rnxFile + ".model";
         modelfile.open( modelName.c_str(), ios::out );
      }

         //// *** Now comes the REAL forwards processing part *** ////

         // Loop over all data epochs
         // there FeedFromRinex3Obs function is used to replace (rin>>gRin)
         // as glonass freqency number is needed to process glonass data
         // the Rinex3ObsHeader of some stations also have this messeage.
      while(FeedFromRinex3Obs(rin,gRin,FreqNumber))
      {
     
            // Store current epoch
         CommonTime time(gRin.header.epoch);
            // Compute solid, oceanic and pole tides effects at this epoch
         Triple tides( solid.getSolidTide( time, nominalPos )  +
                       ocean.getOceanLoading( station, time )  +
                       pole.getPoleTide( time, nominalPos )    );

            // Update observable correction object with tides information
         corr.setExtraBiases(tides);
         try
         {
               // Let's process data. Thanks to 'ProcessingList' this is
               // very simple and compact: Just one line of code!!!.
             gRin >> pList;
         
         
         }
         catch(DecimateEpoch& d)
         {
               // If we catch a DecimateEpoch exception, just continue.
            continue;
         }
         catch(SVNumException& s)
         {
               // If we catch a SVNumException, just continue.
            continue;
         }
         catch(Exception& e)
         {
            cerr << "Exception for receiver '" << station <<
                    "' at epoch: " << time << "; " << e << endl;
            continue;
         }
         catch(...)
         {
            cerr << "Unknown exception for receiver '" << station <<
                    " at epoch: " << time << endl;
            continue;
         }


            // Ask if we are going to print the model
         if ( printmodel )
         {
            printModel( modelfile,
                        gRin,
                        precision );

         }

            // Check what type of solver we are using
         if ( cycles < 1 )
         {

               // This is a 'forwards-only' filter. Let's print to output
               // file the results of this epoch
            printSolution( outfile,
                           pppgnssSolver,
                           time,
                           cDOP,
                           isNEU,
                           usingGPS,
                           usingGlonass,
                           usingBeiDou,
                           usingGalileo,
                           gRin.numSats(),
                           drytropo,
                           precision );

         }  // End of 'if ( cycles < 1 )'



         // The given epoch hass been processed. Let's get the next one

      }  // End of 'while(rin >> gRin)'


         // Close current Rinex observation stream
      rin.close();


         // If we printed the model, we must close the file
      if ( printmodel )
      {
            // Close model file for this station
         modelfile.close();
      }

         //// *** Forwards processing part is over *** ////



         // Now decide what to do: If solver was a 'forwards-only' version,
         // then we are done and should continue with next station.
      if ( cycles < 1 )
      {
            // Now, Let's extract the Time-To-First-Fix information
            // that stored in 'ttfc/ttffL1' in the Solver.
         std::vector<double> ttfc;
         ttfc = pppgnssSolver.getTTFC(); 

            // Let's open the output file
         string ttfcName( outputFileName+ ".ttfc" );

         ofstream ttfcFile;
         ttfcFile.open( ttfcName.c_str(), ios::out );

         for(std::vector<double>::iterator it=ttfc.begin();
             it!=ttfc.end();
             ++it)
         {
            ttfcFile << (*it) << endl;
         }
         ttfcFile.close();

            // Close output file for this station
         outfile.close();

            // We are done with this station. Let's show a message
         cout << "Processing finished for station: '" << station
              << "'. Results in file: '" << outputFileName << "'." << endl;

            // WARNING:::::::::::::::::
            // Index for rinex file iterator.
            // before goto the next file
         ++rnxit;
         if(outputFileListOpt.getCount())
         {
            ++outit;
         }

            // Go process next station
         continue;

      }

   
         // We are done. Close and go for next station
         
         // Now, Let's extract the Time-To-First-Fix information
         // that stored in 'ttfc/ttffL1' in the Solver.
      std::vector<double> ttfc;
      ttfc = pppgnssSolver.getTTFC(); 

         // Let's open the output file
      string ttfcName( outputFileName+ ".ttfc" );

      ofstream ttfcFile;
      ttfcFile.open( ttfcName.c_str(), ios::out );

      for(std::vector<double>::iterator it=ttfc.begin();
          it!=ttfc.end();
          ++it)
      {
         ttfcFile << (*it) << endl;
      }
      ttfcFile.close();

         // Close output file for this station
      outfile.close();


         // We are done with this station. Let's show a message
      cout << "Processing finished for station: '" << station
           << "'. Results in file: '" << outputFileName << "'." << endl;

         // Index for rinex file iterator.
      ++rnxit;

      if(outputFileListOpt.getCount())
      {
         ++outit;
      }

   }  // end of 'while (...)'
      //***********************************************
      //
      // At last, Let's clear the content of SP3/EOP/MSC object
      //
      //***********************************************
   SP3EphList.clear();
   eopStore.clear();
   mscStore.clear();


   return;

}  // End of 'pppgnss::process()'



   // Main function
int main(int argc, char* argv[])
{

   try
   {

      pppgnss program(argv[0]);

         // We are disabling 'pretty print' feature to keep
         // our description format
      if ( !program.initialize(argc, argv, true) )
      {
         return 0;
      }

      if ( !program.run() )
      {
         return 1;
      }

      return 0;

   }
   catch(Exception& e)
   {

      cerr << "Problem: " << e << endl;

      return 1;

   }
   catch(...)
   {

      cerr << "Unknown error." << endl;

      return 1;

   }

   return 0;

}  // End of 'main()'

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeTurfHkJsonFiles
////      This is a simple program that converts ANITA Turf hk root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "SummedTurfRate.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


SummedTurfRate *turfRatePtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/turfRateFile13.root\n";  
}


int main(int argc, char **argv) {
  if(argc<2) {
    usage(argv);
    return -1;
  }


  TFile *fp = TFile::Open(argv[1]);
  if(!fp) {
    std::cerr << "Can't open file\n";
    return -1;
  }
  TTree *sumTurfRateTree = (TTree*) fp->Get("sumTurfRateTree");
  if(!sumTurfRateTree) {
    std::cerr << "Can't find sumTurfRateTree\n";
    return -1;
  }

  if(sumTurfRateTree->GetEntries()<1) {
    std::cerr << "No entries in sumTurfRateTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  sumTurfRateTree->SetBranchAddress("turf",&turfRatePtr);
  
  sumTurfRateTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)turfRatePtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=turfRatePtr->run;


  //Now we set up out run list
  Long64_t numEntries=sumTurfRateTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA3",60);

  

  char instrumentName[20];
  sprintf(instrumentName,"ANITA3");


  char polString[2]={'V','H'};

  //  numEntries=1;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }

    //This line gets the Hk Entry
    sumTurfRateTree->GetEntry(event);

    TTimeStamp timeStamp((time_t)turfRatePtr->realTime,(Int_t)0);
    //    std::cout << "Run: "<< realEvPtr->
    if(lastTime<turfRatePtr->realTime) lastTime=turfRatePtr->realTime;

    //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
    //Summary file fun
    char elementName[180];
    char elementLabel[180];


    sprintf(elementName,"deadTime");
    sprintf(elementLabel,"Dead Time");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,turfRatePtr->getDeadTimeFrac());


    for(int buf=0;buf<4;buf++) {
      sprintf(elementName,"bufferCount%d",buf);
      sprintf(elementLabel,"Buffer Count -- %d",buf);
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,turfRatePtr->bufferCount[buf]);
    }



    for( int phi=0; phi<PHI_SECTORS; ++phi ) {
      AnitaPol::AnitaPol_t pol=AnitaPol::kVertical;
      sprintf(elementName,"l3Rates%c_%d",AnitaPol::polAsChar(pol),phi);
      sprintf(elementLabel,"%d-%c",phi+1,AnitaPol::polAsChar(pol));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,turfRatePtr->getL3Rate(phi,pol));
      pol=AnitaPol::kHorizontal;
      sprintf(elementName,"l3Rates%c_%d",AnitaPol::polAsChar(pol),phi);
      sprintf(elementLabel,"%d-%c",phi+1,AnitaPol::polAsChar(pol));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,turfRatePtr->getL3Rate(phi,pol));         
    }


   int numPhiMaskV=0;
   int numPhiMaskH=0;
   int numL1MaskV=0;
   int numL1MaskH=0;
   for(int bit=0;bit<16;bit++) {
      sprintf(elementName,"phiTrigMask%d",bit);
      sprintf(elementLabel,"Phi Mask VPol %d",bit+1);
      int value=(bit+1)*turfRatePtr->isPhiMasked(bit,AnitaPol::kVertical);
      if(value>0) numPhiMaskV++;
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,value);

      sprintf(elementName,"phiTrigMaskH%d",bit);
      sprintf(elementLabel,"Phi Mask HPol %d",bit+1);
      value=(bit+1)*turfRatePtr->isPhiMasked(bit,AnitaPol::kHorizontal);
      if(value>0) numPhiMaskH++;
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,value);

      sprintf(elementName,"l1TrigMask%d",bit);
      sprintf(elementLabel,"L1 Mask VPol %d",bit+1);
      value=(bit+1)*turfRatePtr->isL1Masked(bit,AnitaPol::kVertical);
      if(value)numL1MaskV++;
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,value);

      sprintf(elementName,"l1TrigMaskH%d",bit);
      sprintf(elementLabel,"L1 Mask HPol %d",bit+1);
      value=(bit+1)*turfRatePtr->isL1Masked(bit,AnitaPol::kHorizontal);
      if(value)numL1MaskH++;
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,value);
   }


   sprintf(elementName,"numPhiMaskH");
   sprintf(elementLabel,"Num Phi H-Pol");
   summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,numPhiMaskH);  

   sprintf(elementName,"numPhiMaskV");
   sprintf(elementLabel,"Num Phi V-Pol");
   summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,numPhiMaskV); 

   sprintf(elementName,"numL1MaskH");
   sprintf(elementLabel,"Num L1 H-Pol");
   summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,numL1MaskH);  

   sprintf(elementName,"numL1MaskV");
   sprintf(elementLabel,"Num L1 V-Pol");
   summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,numL1MaskV);  


  }
  std::cerr << "\n";


  char outputDir[FILENAME_MAX];
  char *outputDirEnv=getenv("AWARE_OUTPUT_DIR");
  if(outputDirEnv==NULL) {
    sprintf(outputDir,"/unix/anita1/data/aware/output");
  }
  else {
    strncpy(outputDir,outputDirEnv,FILENAME_MAX);
  }
    


  char dirName[FILENAME_MAX];
  char dateDirName[FILENAME_MAX];
  char subDateDirName[FILENAME_MAX];
  sprintf(dirName,"%s/%s/runs%d/runs%d/run%d/",outputDir,instrumentName,runNumber-runNumber%10000,runNumber-runNumber%100,runNumber);
  sprintf(dateDirName,"%s/%s/%d/%04d/run%d/",outputDir,instrumentName,dateInt/10000,dateInt%10000,runNumber);
  sprintf(subDateDirName,"%s/%s/%d/%04d/",outputDir,instrumentName,dateInt/10000,dateInt%10000);
  gSystem->mkdir(dirName,kTRUE);
  gSystem->mkdir(subDateDirName,kTRUE);
  gSystem->Symlink(dirName,dateDirName);

  std::cout << "Making: " << dirName << "\n";
  

  char fullDir[FILENAME_MAX];
  sprintf(fullDir,"%s/full",dirName);
  gSystem->mkdir(fullDir,kTRUE);
  summaryFile.writeFullJSONFiles(fullDir,"turfRate");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/turfRateSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/turfRateTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastSummedTurfRate",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
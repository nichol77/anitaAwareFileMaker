#!/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no> <skip root>" 1>&2
   exit 1
fi



RUN=$1
BASE_DIR=/anitaStorage/palestine14
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}

#Step 1: Generate the ROOT Files


if [ "$2" = "" ]
then
    cd /home/radio/anita14/simpleTreeMaker/
    ./runAnitaIIIFileMaker.sh $RUN
fi

#Step 2: Generate the AWARE Files
cd /home/radio/anita14/anitaAwareFileMaker/
source setupAwareVariables.sh
if [ -d "$ROOT_RUN_DIR" ]; then
    ./makeHeaderJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root 
    ./makePrettyHkJsonFiles ${ROOT_RUN_DIR}/hkFile${RUN}.root  
    ./makeSurfHkJsonFiles ${ROOT_RUN_DIR}/surfHkFile${RUN}.root    
    ./makeAcqdStartRunJsonFiles ${ROOT_RUN_DIR}/auxFile${RUN}.root     
    ./makeMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root     
    ./makeOtherMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root    
#    ./makeEventJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root ${ROOT_RUN_DIR}/eventFile${RUN}.root  &
fi

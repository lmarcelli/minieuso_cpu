//============================================================================
// Name        : listserials.cpp
// Author      : Sara Turriziani
// Version     : 1.0
// Copyright   : Mini-EUSO copyright notice
// Description : Cameras Module in C++, ANSI-style, for linux
//============================================================================


#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>
#include <math.h>
#include <time.h>
#include <ctime>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "/usr/include/flycapture/FlyCapture2.h"
using namespace FlyCapture2;
using namespace std;


volatile sig_atomic_t done = 0;

void term(int signum)
{
    done = 1;
}

void PrintError( Error error )
{
    error.PrintErrorTrace();
}


//int main(int /*argc*/, char** /*argv*/)
int main(int argc, char* argv[])
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    if (sigaction(SIGTERM, &action, NULL) == -1) {
        perror("Error: cannot handle SIGTERM"); // Should not happen
    }   

    if (sigaction(SIGINT, &action, NULL) == -1) {
        perror("Error: cannot handle SIGINT"); // Should not happen
    }

        if (sigaction(SIGHUP, &action, NULL) == -1) {
        perror("Error: cannot handle SIGHUP"); // Should not happen
    }


	unsigned int ulValue;
	CameraInfo camInfo;


    // Check the number of parameters
	    if (argc < 1) {
	        // Tell the user how to run the program if the user enters the command incorrectly.
	        std::cerr << "Usage: " << argv[0] << " PARFILESPATH"  << std::endl; // Usage message
	        return 1;
	    }

	    std:string pardir = argv[1];

    Error error;
   	

    BusManager busMgr;
    unsigned int numCameras;

    

    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }


    if (numCameras < 1)
       {
       printf( "WARNING: No camera was detected\n");
       printf( "WARNING: File cameras.ini will not be created\n");        
       return -1;
       }
    
    if (numCameras < 2)
       {
       printf( "WARNING: Only one camera was detected\n");
       printf( "WARNING: File cameras.ini will not be created\n");        
       return -1;
       }
 


        string filename;                
        stringstream f;
        f << pardir  << "cameras.ini";
        filename = f.str();
  
	ofstream myfile (filename.c_str()); 	
        
        if (myfile.is_open()) { 
        printf("Success: you have permission to write file %s in %s folder.\n", filename.c_str(), pardir.c_str());
        Camera *pCameras = new Camera[numCameras]; // initialize an array of cameras

    for (unsigned int i=0; i < numCameras; i++)
          {
            PGRGuid guid;
            error = busMgr.GetCameraFromIndex(i, &guid);
            if (error != PGRERROR_OK)
             {
                PrintError( error );
                delete[] pCameras;
                return -1;
             }          
            error = pCameras[i].Connect(&guid); // connect both cameras
            if (error != PGRERROR_OK)
              {
                PrintError(error);
                delete[] pCameras;
                return -1;
               }

          error = pCameras[i].GetCameraInfo(&camInfo);
          if (error != PGRERROR_OK)
           {
             PrintError(error);
             delete[] pCameras;
             return -1;
            }     

     char* name1;
             name1 = strtok(camInfo.modelName, " ");

           //  cout << name1 << '\n'; uncomment for debugging

           if ( strcmp("Chameleon", name1) == 0)
            {
        	  myfile <<  camInfo.serialNumber << " NIR " << camInfo.modelName << endl;
            }

          else
           {
        	  myfile << camInfo.serialNumber << " VIS " << camInfo.modelName << endl; 
           }
        
          
    // disconnect the camera

          
          pCameras[i].Disconnect();
        }

 
        myfile.close();
        delete[] pCameras;	
	}
        else
        {
          printf("Failed to create file %s in %s folder.  Please check permissions.\n", filename.c_str(), pardir.c_str());
		return -1;
        }
	
	

    

    return 0;
}

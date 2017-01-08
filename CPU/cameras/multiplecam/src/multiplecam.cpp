//============================================================================
// Name        : multiplecam.cpp
// Author      : Sara Turriziani
// Version     : 1.0
// Copyright   : Mini-EUSO copyright notice
// Description : Cameras Acquisition Module in C++, ANSI-style, for linux
//============================================================================

#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "/usr/include/flycapture/FlyCapture2.h"
using namespace FlyCapture2;
using namespace std;

void wait(int seconds)
{
	clock_t endwait;
	endwait = clock() + seconds * CLOCKS_PER_SEC;
	while (clock() < endwait) {}
}

unsigned createMask(unsigned a, unsigned b)
{
	unsigned int r = 0;
	for (unsigned i = a; i < b; i++)
	{
		r = r+1;
		r  = r*2;
	}
	r = r + 1;
	return r;
}


const std::string currentDateTime() {
     time_t     now = time(0);
     struct tm  tstruct;
     char       buf[80];
     tstruct = *localtime(&now);
     // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
     // for more information about date/time format
     strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

     return buf;

 }

void PrintBuildInfo()
{
    FC2Version fc2Version;
    Utilities::GetLibraryVersion( &fc2Version );
    char version[128];
    sprintf(
        version,
        "FlyCapture2 library version: %d.%d.%d.%d\n",
        fc2Version.major, fc2Version.minor, fc2Version.type, fc2Version.build );

    printf( version );

    char timeStamp[512];
    sprintf( timeStamp, "Application build date: %s %s\n\n", __DATE__, __TIME__ );

    printf( timeStamp );
}

void PrintCameraInfo( CameraInfo* pCamInfo )
{
    printf(
        "\n*** CAMERA INFORMATION ***\n"
        "Serial number - %u\n"
        "Camera model - %s\n"
        "Camera vendor - %s\n"
        "Sensor - %s\n"
        "Resolution - %s\n"
        "Firmware version - %s\n"
        "Firmware build time - %s\n\n",
        pCamInfo->serialNumber,
        pCamInfo->modelName,
        pCamInfo->vendorName,
        pCamInfo->sensorInfo,
        pCamInfo->sensorResolution,
        pCamInfo->firmwareVersion,
        pCamInfo->firmwareBuildTime );
}

void PrintError( Error error )
{
    error.PrintErrorTrace();
}

int RunSingleCameraRegister( PGRGuid guid )
{

    Error error;
    Camera cam;

    // Connect to a camera
    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    // Get the camera information
    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    PrintCameraInfo(&camInfo);


        Property frmRate;
        frmRate.type = FRAME_RATE;
        error = cam.GetProperty( &frmRate );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        PropertyInfo frRate;
        frRate.type = FRAME_RATE;
        error = cam.GetPropertyInfo( &frRate );
                if (error != PGRERROR_OK)
                {
                    PrintError( error );
                    return -1;
                }
        float minfrmRate = frRate.absMin;
        float maxfrmRate = frRate.absMax;

        printf( "Default Frame rate is %3.1f fps \n" , frmRate.absValue );

        Property shutter;
        shutter.type = SHUTTER;
        error = cam.GetProperty( &shutter );
        if (error != PGRERROR_OK)
        {
         PrintError( error );
         return -1;
         }

        printf( "Default Shutter is %3.1f ms\n", shutter.absValue );

        if (camInfo.serialNumber == 15568736)
            {
        	unsigned int ulValue;
        	        error = cam.ReadRegister(0x82C, &ulValue); // read the temperature register
        	        if (error != PGRERROR_OK)
        	        {
        	         PrintError( error );
        	         return -1;
        	        }

        	        unsigned r = createMask(20, 31); // extract the bits of interest
        	        unsigned int res = r & ulValue;  // here you have already the Kelvin temperature * 10

        	        cout << "Initial Temperature is " << res / 10.0 << " K/ " << res / 10.0	- 273.15 << " Celtius" << endl;
            }



        Property autoExposure;
        autoExposure.type = AUTO_EXPOSURE;
        error = cam.GetProperty( &autoExposure );
        if (error != PGRERROR_OK)
        {
         PrintError( error );
         return -1;
        }

        printf( "Default AutoExposure is %3.3f \n", autoExposure.absValue );

        PropertyInfo auexp;
        auexp.type = AUTO_EXPOSURE;
        error = cam.GetPropertyInfo( &auexp );
        if (error != PGRERROR_OK)
           {
             PrintError( error );
             return -1;
           }
        float minauexp = auexp.absMin;
        float maxauexp = auexp.absMax;

//        printf( "Min %3.3f Max %3.3f \n" , auexp.absMin, auexp.absMax ); // uncomment to debug


        std::string line;
        float frate = frmRate.absValue, shutt = shutter.absValue, autexpo = autoExposure.absValue; // initialize to current values

        ifstream parfile ("/home/minieusouser/CPU/cameras/multiplecam/test.ini");

        // Set the new values from the parameters reading them from the parameter file
        if (parfile.is_open())
        {
        	printf( "Reading from parfile: \n" );
        	while ( getline (parfile,line) )
        	      {
                  	   std::istringstream in(line);      //make a stream for the line itself
                         std::string type;
                         in >> type;                  //and read the first whitespace-separated token

                         if(type == "FRAMERATE")       //and check its value
                         {
                            in >> frate;       //now read the whitespace-separated floats
                            cout << type << " " << frate << " fps " << endl;
                         }
                         else if(type == "SHUTTER")
                         {
                            in >> shutt;
                            cout << type << " " << shutt << " ms " << endl;
                          }
                          else if((type == "AUTOEXPOSURE"))
                           {
                             in >> autexpo;
                             cout << type << " " << autexpo << endl;
                           }
                     }
        	parfile.close();
        }
        else
       {
       printf( "Unable to open parfile!!! \n" );
       return -1;
        }


        frmRate.absControl = true;
        frmRate.onePush = false;
        frmRate.autoManualMode = false;
        frmRate.onOff = true;

        if (frate > minfrmRate && frate < maxfrmRate)
        {
        	frmRate.absValue = frate;
        }

        else{
        	 printf( "Frame Rate outside allowed range. Abort. \n" );
             return -1;
             }

        error = cam.SetProperty( &frmRate );
        if (error != PGRERROR_OK)
        {
          PrintError( error );
          return -1;
        }

        PropertyInfo Shut;
        Shut.type = SHUTTER;
        error = cam.GetPropertyInfo( &Shut );
        if (error != PGRERROR_OK)
        {
           PrintError( error );
           return -1;
        }
        float minShutter = Shut.absMin;
        float maxShutter = Shut.absMax;

 //       printf( "Min %3.1f ms Max %3.1f ms  \n" , Shut.absMin, Shut.absMax ); // uncomment this line to debug


        shutter.absControl = true;
        shutter.onePush = false;
        shutter.autoManualMode = false;
        shutter.onOff = true;

        if (shutt > minShutter && shutt < maxShutter)
        {
          shutter.absValue = shutt;
        }
        else{
             printf( "WARNING! Shutter outside allowed range: setting it to maximum allowed value: %3.1f ms \n", maxShutter );
             shutter.absValue = maxShutter;
             }

        error = cam.SetProperty( &shutter );
        if (error != PGRERROR_OK)
        {
         PrintError( error );
         return -1;
         }

        autoExposure.absControl = true;
        autoExposure.onePush = false;
        autoExposure.autoManualMode = false;
        autoExposure.onOff = true;

        if (autexpo > minauexp && autexpo < maxauexp)
        {
         autoExposure.absValue = autexpo;
        }
        else{
             printf( "WARNING! AutoExposure outside allowed range: setting it to default: %3.3f \n", minauexp );
             autoExposure.absValue = minauexp;
        }

        error = cam.SetProperty( &autoExposure );
        if (error != PGRERROR_OK)
        {
        PrintError( error );
        return -1;
        }

    return 0;
}

int RunSingleCamera( PGRGuid guid , int imageCnt )
{
    Error error;
    Camera cam;

    unsigned int ulValue;

    // Connect to the camera
       error = cam.Connect( &guid );
       if ( error != PGRERROR_OK )
       {
           PrintError( error );
           return -1;
       }

       // Get the camera information
           CameraInfo camInfo;
           error = cam.GetCameraInfo(&camInfo);
           if (error != PGRERROR_OK)
           {
               PrintError( error );
               return -1;
           }

// Enable timestamping
   EmbeddedImageInfo embeddedInfo;

    error = cam.GetEmbeddedImageInfo( &embeddedInfo );
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    }

    if ( embeddedInfo.timestamp.available != 0 )
    {
        embeddedInfo.timestamp.onOff = true;
    }

    error = cam.SetEmbeddedImageInfo( &embeddedInfo );
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    }


 // Start capturing images
 error = cam.StartCapture();
 if (error != PGRERROR_OK)
 {
     PrintError( error );
     return -1;
 }


 Image rawImage;

     // Retrieve an image
     error = cam.RetrieveBuffer( &rawImage );
     if (error != PGRERROR_OK)
     {
         PrintError( error );
 //        continue;
     }

     unsigned int res =	 0; // for initialization purposes

     if (camInfo.serialNumber == 15568736)
     {
      error = cam.ReadRegister(0x82C, &ulValue); // read the temperature register for the NIR camera only
      unsigned r  = createMask(20, 31); // extract the bits of interest
      res = r & ulValue;  // here you have already the Kelvin temperature * 10
      if (error != PGRERROR_OK)
        {
          PrintError( error );
          return -1;
         }
     }

     Property autoExposure;
     autoExposure.type = AUTO_EXPOSURE;
     error = cam.GetProperty( &autoExposure );
     if (error != PGRERROR_OK)
       {
         PrintError( error );
         return -1;
       }

     Property shutter;
     shutter.type = SHUTTER;
     error = cam.GetProperty( &shutter );
     if (error != PGRERROR_OK)
       {
         PrintError( error );
         return -1;
       }

     Property frmRate;
     frmRate.type = FRAME_RATE;
     error = cam.GetProperty( &frmRate );
     if (error != PGRERROR_OK)
       {
         PrintError( error );
         return -1;
       }

     PixelFormat pixFormat;
             unsigned int rows, cols, stride;
             rawImage.GetDimensions( &rows, &cols, &stride, &pixFormat );

     TimeStamp timestamp = rawImage.GetTimeStamp();
                     printf(
                         "TimeStamp [%d %d]: ",
                         timestamp.cycleSeconds,
                         timestamp.cycleCount);
     printf( "Grabbed image %d\n", imageCnt );
     printf( "Frame rate is %3.1f fps\n", frmRate.absValue );
     printf( "Shutter is %3.1f ms\n", shutter.absValue );
     printf( "Autoexposure is %3.3f \n", autoExposure.absValue );
     if (camInfo.serialNumber == 15568736)
     {
      cout << "Temperature is " << res / 10.0 << " K/ " << res / 10.0	- 273.15 << " Celtius" << endl;
     }
      cout << "Raw Image Dimensions: " << rows  << " x " << cols << " Image Stride: " << stride << endl;

     // Create a converted image
     Image convertedImage;

     // Convert the raw image
     if (camInfo.serialNumber == 15568736)
          {
    	       error = rawImage.Convert( PIXEL_FORMAT_MONO8, &convertedImage ); // for NIR camera
    	       if (error != PGRERROR_OK)
    	            {
    	                PrintError( error );
    	                return -1;
    	            }
          }
     else
     {
               error = rawImage.Convert( PIXEL_FORMAT_RGB8, &convertedImage ); // for visible camera
               if (error != PGRERROR_OK)
                    {
                        PrintError( error );
                        return -1;
                    }
     }

     // Create a unique filename

     std::string str;     //temporary string to hold the filename
     int lengthOfString1; //hold the number of characters in the string
     int lengthOfString2; //hold the number of characters in the string

     std::stringstream sstm;
     sstm << currentDateTime();
     str = sstm.str();

     lengthOfString1=str.length();
     stringstream ss;
     ss << imageCnt;
     std::string number = ss.str();
     lengthOfString2=str.length();

     int lenghtsum = lengthOfString1 + lengthOfString2 + 5;
//        cout << lenghtsum  << endl; // uncomment for testing
     char filename[lenghtsum];
     sprintf( filename, "%s-%d.png", str.c_str(),imageCnt );
//        cout <<  filename << endl; //uncomment for testing

     // Save the image. If a file format is not passed in, then the file
     // extension is parsed to attempt to determine the file format.
     error = convertedImage.Save( filename );
     if (error != PGRERROR_OK)
     {
         PrintError( error );
         return -1;
     }

 // Stop capturing images
 error = cam.StopCapture();
 if (error != PGRERROR_OK)
 {
     PrintError( error );
     return -1;
 }

 // Disconnect the camera
 error = cam.Disconnect();
 if (error != PGRERROR_OK)
 {
     PrintError( error );
     return -1;
 }
 printf( "Acquisition Done! Camera Closed! \n" );
 return 0;
}

int main(int argc, char *argv[])
{
	//const int k_numImages = 10;
    int k_numImages = atoi(argv[1]);

    PrintBuildInfo();

    Error error;

    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
	FILE* tempFile = fopen("test.txt", "w+");
	if (tempFile == NULL)
	{
		printf("Failed to create file in current folder.  Please check permissions.\n");
		return -1;
	}
	fclose(tempFile);
	remove("test.txt");

    BusManager busMgr;
    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    printf( "Number of cameras detected: %u\n", numCameras );

   for ( int imageCnt=0; imageCnt < k_numImages; imageCnt++ )

     {
      for (unsigned int i=0; i < numCameras; i++)
      {
        PGRGuid guid;
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        RunSingleCameraRegister( guid );
        RunSingleCamera( guid , imageCnt);
        wait(5);
      }

     }
    return 0;
}

#include "spincapture.h"
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <iostream>
#include <sstream>
#include "SpinVideo.h"



// Disables heartbeat on GEV cameras so debugging does not incur timeout errors
int DisableHeartbeat(CameraPtr pCam, INodeMap & nodeMap, INodeMap & nodeMapTLDevice)
{
    cout << "Checking device type to see if we need to disable the camera's heartbeat..."<< endl << endl;

    //
    // Write to boolean node controlling the camera's heartbeat
    //
    // *** NOTES ***
    // This applies only to GEV cameras and only applies when in DEBUG mode.
    // GEV cameras have a heartbeat built in, but when debugging applications the
    // camera may time out due to its heartbeat. Disabling the heartbeat prevents
    // this timeout from occurring, enabling us to continue with any necessary debugging.
    // This procedure does not affect other types of cameras and will prematurely exit
    // if it determines the device in question is not a GEV camera.
    //
    // *** LATER ***
    // Since we only disable the heartbeat on GEV cameras during debug mode, it is better
    // to power cycle the camera after debugging. A power cycle will reset the camera
    // to its default settings.
    //
    CEnumerationPtr ptrDeviceType = nodeMapTLDevice.GetNode("DeviceType");
    if (!IsAvailable(ptrDeviceType) || !IsReadable(ptrDeviceType))
    {
        cout << "Error with reading the device's type. Aborting..." << endl << endl;
        return -1;
    }
    else
    {
        if (ptrDeviceType->GetIntValue() == DeviceType_GEV)
        {
            cout << "Working with a GigE camera. Attempting to disable heartbeat before continuing..." << endl << endl;
            CBooleanPtr ptrDeviceHeartbeat = nodeMap.GetNode("GevGVCPHeartbeatDisable");
            if (!IsAvailable(ptrDeviceHeartbeat) || !IsWritable(ptrDeviceHeartbeat))
            {
                cout << "Unable to disable heartbeat on camera. Continuing with execution as this may be non-fatal..." << endl << endl;
            }
            else
            {
                ptrDeviceHeartbeat->SetValue(true);
                cout << "WARNING: Heartbeat on GigE camera disabled for the rest of Debug Mode." << endl;
                cout << "         Power cycle camera when done debugging to re-enable the heartbeat..." << endl << endl;
            }
        }
        else
        {
            cout << "Camera does not use GigE interface. Resuming normal execution..." << endl << endl;
        }
    }
    return 0;
}

int printDeviceInfo(INodeMap &nodeMap, unsigned int camNum)
{
    int result = 0;

    qDebug() << "*** DEVICE INFORMATION ***";
    cout << "Printing device information for camera " << camNum << "..." << endl << endl;

    try{
        FeatureList_t features;
        CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
        if(IsAvailable(category) && IsReadable(category)){
            category->GetFeatures(features);

            FeatureList_t::const_iterator it;
            for (it = features.begin(); it != features.end(); ++it){
                CNodePtr pfeatureNode = *it;
                qDebug() << pfeatureNode->GetName() << " : ";
                CValuePtr pValue = (CValuePtr)pfeatureNode;
                qDebug() << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
            }
        }
        else{
            qDebug() << "Device control information not available.";
        }
    }
    catch (Spinnaker::Exception &e){
        qDebug() << "Error: " << e.what() ;
        result = -1;
    }

    return result;
}

// This function acts as the body of the example; please see NodeMapInfo example
// for more in-depth comments on setting up cameras.

int ConfigureCustomImageSettings(CameraPtr pCam)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING CUSTOM IMAGE SETTINGS ***" << endl << endl;

    try
    {
        //
        // Apply mono 8 pixel format
        //
        // *** NOTES ***
        // In QuickSpin, enumeration nodes are as easy to set as other node
        // types. This is because enum values representing each entry node
        // are added to the API.
        //
        if (IsReadable(pCam->PixelFormat) && IsWritable(pCam->PixelFormat))
        {
            pCam->PixelFormat.SetValue(PixelFormat_Mono8);

            cout << "Pixel format set to " << pCam->PixelFormat.GetCurrentEntry()->GetSymbolic() << "..." << endl;
        }
        else
        {
            cout << "Pixel format not available..." << endl;
            result = -1;
        }

        //
        // Apply minimum to offset X
        //
        // *** NOTES ***
        // Numeric nodes have both a minimum and maximum. A minimum is retrieved
        // with the method GetMin(). Sometimes it can be important to check
        // minimums to ensure that your desired value is within range.
        //
        if (IsReadable(pCam->OffsetX) && IsWritable(pCam->OffsetX))
        {
            pCam->OffsetX.SetValue(pCam->OffsetX.GetMin());

            cout << "Offset X set to " << pCam->OffsetX.GetValue() << "..." << endl;
        }
        else
        {
            cout << "Offset X not available..." << endl;
            result = -1;
        }

        //
        // Apply minimum to offset Y
        //
        // *** NOTES ***
        // It is often desirable to check the increment as well. The increment
        // is a number of which a desired value must be a multiple. Certain
        // nodes, such as those corresponding to offsets X and Y, have an
        // increment of 1, which basically means that any value within range
        // is appropriate. The increment is retrieved with the method GetInc().
        //
        if (IsReadable(pCam->OffsetY) && IsWritable(pCam->OffsetY))
        {
            pCam->OffsetY.SetValue(pCam->OffsetY.GetMin());

            cout << "Offset Y set to " << pCam->OffsetY.GetValue() << "..." << endl;
        }
        else
        {
            cout << "Offset Y not available..." << endl;
            result = -1;
        }

        //
        // Set maximum width
        //
        // *** NOTES ***
        // Other nodes, such as those corresponding to image width and height,
        // might have an increment other than 1. In these cases, it can be
        // important to check that the desired value is a multiple of the
        // increment.
        //
        // This is often the case for width and height nodes. However, because
        // these nodes are being set to their maximums, there is no real reason
        // to check against the increment.
        //
        if (IsReadable(pCam->Width) && IsWritable(pCam->Width) && pCam->Width.GetInc() != 0 && pCam->Width.GetMax() != 0)
        {
            pCam->Width.SetValue(pCam->Width.GetMax());

            cout << "Width set to " << 480 << "..." << endl;
        }
        else
        {
            cout << "Width not available..." << endl;
            result = -1;
        }

        //
        // Set maximum height
        //
        // *** NOTES ***
        // A maximum is retrieved with the method GetMax(). A node's minimum and
        // maximum should always be a multiple of its increment.
        //
        if (IsReadable(pCam->Height) && IsWritable(pCam->Height) && pCam->Height.GetInc() != 0 && pCam->Height.GetMax() != 0)
        {
            pCam->Height.SetValue(pCam->Height.GetMax());

            cout << "Height set to " << 840 << "..." << endl;
        }
        else
        {
            cout << "Height not available..." << endl;
            result = -1;
        }
    }
    catch (Spinnaker::Exception &e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}


int acquireImages(CameraList camList)
{
    int result = 0;
    CameraPtr pCam = nullptr;

    cout << endl << "*** IMAGE ACQUISITION ***" << endl << endl;

    try
    {
        //
        // Prepare each camera to acquire images
        //
        // *** NOTES ***
        // For pseudo-simultaneous streaming, each camera is prepared as if it
        // were just one, but in a loop. Notice that cameras are selected with
        // an index. We demonstrate pseduo-simultaneous streaming because true
        // simultaneous streaming would require multiple process or threads,
        // which is too complex for an example.
        //
        // Serial numbers are the only persistent objects we gather in this
        // example, which is why a vector is created.
        //
        vector<gcstring> strSerialNumbers(camList.GetSize());

        for (unsigned int i = 0; i < camList.GetSize(); i++)
        {
            // Select camera
            pCam = camList.GetByIndex(i);

            // Set acquisition mode to continuous
            CEnumerationPtr ptrAcquisitionMode = pCam->GetNodeMap().GetNode("AcquisitionMode");
            if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
            {
                cout << "Unable to set acquisition mode to continuous (node retrieval; camera " << i << "). Aborting..." << endl << endl;
                return -1;
            }

            CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
            if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
            {
                cout << "Unable to set acquisition mode to continuous (entry 'continuous' retrieval " << i << "). Aborting..." << endl << endl;
                return -1;
            }

            int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

            ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

            cout << "Camera " << i << " acquisition mode set to continuous..." << endl;


//#ifdef _DEBUG
            cout << endl << endl << "*** DEBUG ***" << endl << endl;

            // If using a GEV camera and debugging, should disable heartbeat first to prevent further issues
            if (DisableHeartbeat(pCam, pCam->GetNodeMap(), pCam->GetTLDeviceNodeMap()) != 0)
            {
                return -1;
            }

            cout << endl << endl << "*** END OF DEBUG ***" << endl << endl;
//#endif
            // Begin acquiring images
            pCam->BeginAcquisition();

            cout << "Camera " << i << " started acquiring images..." << endl;

            // Retrieve device serial number for filename
            strSerialNumbers[i] = "";

            CStringPtr ptrStringSerial = pCam->GetTLDeviceNodeMap().GetNode("DeviceSerialNumber");

            if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
            {
                strSerialNumbers[i] = ptrStringSerial->GetValue();
                cout << "Camera " << i << " serial number set to " << strSerialNumbers[i] << "..." << endl;
            }

            cout << endl;
        }

        //
        // Retrieve, convert, and save images for each camera
        //
        // *** NOTES ***
        // In order to work with simultaneous camera streams, nested loops are
        // needed. It is important that the inner loop be the one iterating
        // through the cameras; otherwise, all images will be grabbed from a
        // single camera before grabbing any images from another.
        //
        const unsigned int k_numImages = 10;

        for (unsigned int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
        {
            for (unsigned int i = 0; i < camList.GetSize(); i++)
            {
                try
                {
                    // Select camera
                    pCam = camList.GetByIndex(i);

                    // Retrieve next received image and ensure image completion
                    ImagePtr pResultImage = pCam->GetNextImage();

                    if (pResultImage->IsIncomplete())
                    {
                        cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl << endl;
                    }
                    else
                    {
                        // Print image information
                        cout << "Camera " << i << " grabbed image " << imageCnt << ", width = " << pResultImage->GetWidth() << ", height = " << pResultImage->GetHeight() << endl;

                        // Convert image to mono 8
                        ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);

                        // Create a unique filename
                        ostringstream filename;

                        filename << "AcquisitionMultipleCamera-";
                        if (strSerialNumbers[i] != "")
                        {
                            filename << strSerialNumbers[i].c_str();
                        }
                        else
                        {
                            filename << i;
                        }
                        filename << "-" << imageCnt << ".jpg";

                        // Save image
                        convertedImage->Save(filename.str().c_str());

                        cout << "Image saved at " << filename.str() << endl;
                    }

                    // Release image
                    pResultImage->Release();

                    cout << endl;
                }
                catch (Spinnaker::Exception &e)
                {
                    cout << "Error: " << e.what() << endl;
                    result = -1;
                }
            }
        }

        //
        // End acquisition for each camera
        //
        // *** NOTES ***
        // Notice that what is usually a one-step process is now two steps
        // because of the additional step of selecting the camera. It is worth
        // repeating that camera selection needs to be done once per loop.
        //
        // It is possible to interact with cameras through the camera list with
        // GetByIndex(); this is an alternative to retrieving cameras as
        // CameraPtr objects that can be quick and easy for small tasks.
        //
        for (unsigned int i = 0; i < camList.GetSize(); i++)
        {
            // End acquisition
            camList.GetByIndex(i)->EndAcquisition();
        }
    }
    catch (Spinnaker::Exception &e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }
    return result;
}

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo example for more in-depth comments on printing
// device information from the nodemap.

SpinCapture::SpinCapture(QObject *parent)
{

    // Retrieve singleton reference to system object
    system = System::GetInstance();

    // Retrieve list of cameras from the system
    camList = system->GetCameras();

    unsigned int numCameras = camList.GetSize();
    for (unsigned int i = 0; i < numCameras; ++i)
    {

       CameraPtr pCamera = camList.GetByIndex(i);

       pCamera->Init();

    }
    qDebug() << "Number of cameras detected: " << numCameras;
    //DisableHeartbeat();

    cout << "Application build date: " << __DATE__ << " " << __TIME__ << endl << endl;



    // Print out current library version
    const LibraryVersion spinnakerLibraryVersion = system->GetLibraryVersion();
    cout << "Spinnaker library version: "
        << spinnakerLibraryVersion.major << "."
        << spinnakerLibraryVersion.minor << "."
        << spinnakerLibraryVersion.type << "."
        << spinnakerLibraryVersion.build << endl << endl;



    // Finish if there are no cameras
    if(numCameras == 0){
        // Clear camera list before releasing system
        camList.Clear();


        // Release system
        system->ReleaseInstance();

        qDebug() << "Not enough cameras!" ;
        qDebug() << "Done! Press Enter to exit..." ;
        getchar();

        qDebug()<<"BAD";
    }

    //pCam = NULL;
    int i =0;

    pCam = camList.GetByIndex(i);
    int result =0;
    try{
        // Retrieve TL device nodemap and print device information
        for (unsigned int i = 0; i < camList.GetSize(); i++)
        {
            // Select camera
            pCam = camList.GetByIndex(i);

            // Retrieve TL device nodemap
            INodeMap & nodeMapTLDevice = pCam->GetTLDeviceNodeMap();

            // Print device information
            result = printDeviceInfo(nodeMapTLDevice, i);
        }


        // Initialize camera
        pCam->Init();

        // Retrieve GenICam nodemap
        nodeMap = &pCam->GetNodeMap();

        width = pCam->Width.GetValue();
        height = pCam->Height.GetValue();
        exposureTime = pCam->ExposureTime.GetValue();
        pixelFormat = pCam->PixelFormat.GetValue();
        qDebug() <<"Initialized Spinnaker compatible camera ("
                 << width << "x" << height
                 << pCam->PixelFormat.GetCurrentEntry()->GetSymbolic()
                 << ")";

        isInitialized = true;
        int result = 0;
        int result2 = 0;

        // Configure custom image settings
        result2 = ConfigureCustomImageSettings(pCam);
//        if (result2 < 0)
//        {
//            return result2;
//        }

        // Acquire images
        result = result | acquireImages(camList);
        cout << "Example complete..." << endl << endl;

        // Clear camera list before releasing system
        camList.Clear();

        // Release system
        system->ReleaseInstance();

        cout << endl << "Done! Press Enter to exit..." << endl;
        getchar();

        //class ODevFileStream(int i);
        //return result;

        }
    catch (Spinnaker::Exception &e){
        qDebug() << "Error: " << e.what() ;
    }
}


// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo example for more in-depth comments on printing
// device information from the nodemap.

SpinCapture::~SpinCapture()
{
    // Deinitialize camera
    pCam->DeInit();
    pCam = NULL;
    nodeMapTLDevice = NULL;
    nodeMap = NULL;
    // Clear camera list before releasing system
    camList.Clear();
    // Release system
    system->ReleaseInstance();
}

#pragma once

#include "CameraData.h"
#include "TalkMasterConsole.h"
#include <map>

using namespace std;

class CameraDataManager
{
public:
	~CameraDataManager(void);
	static CameraDataManager &getInstance();
	void ReadRegistry(CTalkMasterConsoleApp* app, _Options& options);
	void WriteRegistry();	
	_Options* getOptions();

	/**
	 * Adds the provided CameraData to the map.
	 */
	void addCameraData(CameraData *data);

	/** 
	 * This function will get you the Camera Data for the
	 * provided intercom.
	 */
	CameraData* getCameraData(string intercom);

protected:

	void decode(string encoded, CameraData *data);
	string encode(CameraData *data);
    
private:
	CameraDataManager(void);
	map<string, CameraData*> cameraData;

	/** Managed outside of this class; we just want a reference.  */
	_Options* options;

	/** Reference to the application.  */
	CTalkMasterConsoleApp* app;

	static CameraDataManager instance;
};

#include "StdAfx.h"
#include "CameraDataManager.h"

#include <sstream>

/* Loads the singleton. */
CameraDataManager CameraDataManager::instance;


/** 
 * Namespace to contain strings/helper functions reguarding data
 * to be stored in the registry.
 */
namespace CameraDataManagerStrings
{
	const static basic_string<TCHAR> KEY_GROUP = "CameraSettings";
	const static basic_string<TCHAR> KEY_COUNT = "CameraDataManager.count";
	const static basic_string<TCHAR> KEY_BASE = "CameraDataManager.intercom.";

	static basic_string<TCHAR> getKeyName(int index)
	{
		ostringstream oss;
		oss << KEY_BASE << index;

		basic_string<TCHAR> out = oss.str().c_str();

		return out;
	}
};

using namespace CameraDataManagerStrings;

CameraDataManager &CameraDataManager::getInstance()
{
	return instance;
}

CameraDataManager::CameraDataManager(void)
{
	// TODO
}

CameraDataManager::~CameraDataManager(void)
{
	for(map<string, CameraData*>::iterator i=cameraData.begin();
		i!=cameraData.end(); ++i)
	{
		delete i->second;
		i->second = NULL;
	}
}

/**
 * This is the function that goes out and loads the Camera Data from
 * the Registry.
 */
void CameraDataManager::ReadRegistry(CTalkMasterConsoleApp* app, _Options& options)
{
	this->options = &options;
	this->app = app;
	int count = app->GetProfileInt(KEY_GROUP.c_str(),KEY_COUNT.c_str(),0);

	cameraData.clear();

	for(int i=0; i<count; i++)
	{
		// Get the Camera information from the Registry:
		string csTmp = app->GetProfileString(KEY_GROUP.c_str(), 
			getKeyName(i).c_str(), "");
		
		// Create a CameraData and decode the registry key into it:
		CameraData* cdTmp = new CameraData();
		decode(csTmp, cdTmp);

		// Put the data in a pair so it can be put in the map:
		pair<string, CameraData*> p;
		p.first = cdTmp->getIntercom();
		p.second = cdTmp;

		// Put the data in the map:
		cameraData.insert(p);
	}
}

void CameraDataManager::WriteRegistry()
{
	app->WriteProfileInt(KEY_GROUP.c_str(), KEY_COUNT.c_str(), cameraData.size());

	map<string, CameraData*>::iterator i;
	int index = 0;

	for(i=cameraData.begin(); i!=cameraData.end(); ++i)
	{
		basic_string<TCHAR> name = getKeyName(index);
		basic_string<TCHAR> value = encode(i->second);
		app->WriteProfileString(KEY_GROUP.c_str(), 
			name.c_str(), 
			value.c_str());		
 		++index;
	}
}

CameraData* CameraDataManager::getCameraData(string intercom)
{
	map<string,CameraData*>::iterator i = cameraData.find(intercom.c_str());

	CameraData *data = NULL;

	if(i!=cameraData.end())
	{
		 data = i->second;
	}

	return data;
}


/**
 * Function that decodes the provided string for you and sets
 * the data members of the provided CameraData.
 */
void CameraDataManager::decode(string encoded, CameraData* data)
{
	size_t fc = encoded.find(',', 0);
	size_t lc = 0;

	if(fc>0)
	{
		// Intercom Name
		data->setIntercom(encoded.substr(0,fc));
		lc = ++fc;

		// URL
		fc = encoded.find(',', lc);

		if(fc > encoded.length()) {
			data->setUrl(encoded.substr(lc));
			return;
		}
		else if( fc > lc )
		{
			data->setUrl(encoded.substr(lc, fc-lc));
			lc = ++fc;

			// Username
			fc = encoded.find(',', lc);

			if(fc > encoded.length())
			{
				return;
			}

			if(fc > lc)
			{
				data->setUsername(encoded.substr(lc, fc-lc));
				lc = ++fc;
				
				// Password
				data->setPassword(encoded.substr(lc)); 
			} else {
				data->setUsername(encoded.substr(fc));
			}

		} else
		{
			data->setUrl(encoded.substr(fc));
		}
	} else
	{
		data->setIntercom(encoded);
	}
}

void CameraDataManager::addCameraData(CameraData *data)
{
	// Cleanup first:
	CameraData *tData = getCameraData(data->getIntercom());
	if(tData)
	{
		cameraData.erase(cameraData.find(data->getIntercom()));
		if(tData!=data)
		{
			delete tData;		
		}
	}

	pair<string, CameraData*> p;
	p.first = data->getIntercom();
	p.second = data;

	cameraData.insert(p);
	WriteRegistry();
}

/**
 * This function will encode the provided CameraData for you.
 */
string CameraDataManager::encode(CameraData *data)
{
	ostringstream oss;

	oss << data->getIntercom() << "," << data->getUrl();

	if(data->getUsername().size()>0)
	{
		oss << "," << data->getUsername();
	}
	
	if(data->getPassword().size()>0)
	{
		oss << "," << data->getPassword();
	}

	return oss.str();
}

_Options* CameraDataManager::getOptions()
{
	return this->options;
}
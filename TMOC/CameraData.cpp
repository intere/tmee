#include "StdAfx.h"
#include ".\cameradata.h"

//------------------------------------------------------------------------
// Constructors/Destructors:
//------------------------------------------------------------------------

/** Default Constructor.  */
CameraData::CameraData() : intercom(""), url(""), username(""), password("")
{

}

/** Constructor that sets the URL only.  */
CameraData::CameraData(string intercom) : intercom(""), username(""), password("")
{
	this->intercom = intercom;
}

/** Constructor that sets the URL, username and password.  */
CameraData::CameraData(string url, string username, string password) : intercom("")
{
	this->url = url;
	this->username = username;
	this->password = password;
}

/** Constructor that sets the Intercom, URL, username and password.  */
CameraData::CameraData(string intercom, string url, string username, string password)
{
	this->intercom = intercom;
	this->url = url;
	this->username = username;
	this->password = password;
}

/** Destructor.  */
CameraData::~CameraData(void)
{
	// nothing to do
}

//------------------------------------------------------------------------
// Getters:
//------------------------------------------------------------------------

string CameraData::getUrl()
{
	return this->url;
}

string CameraData::getUsername()
{
	return this->username;
}

string CameraData::getPassword()
{
	return this->password;
}

string CameraData::getIntercom()
{
	return this->intercom;
}

//------------------------------------------------------------------------
// Setters:
//------------------------------------------------------------------------

void CameraData::setUrl(string url)
{
	this->url = url;
}

void CameraData::setUsername(string username)
{
	this->username = username;
}

void CameraData::setPassword(string password)
{
	this->password = password;
}

void CameraData::setIntercom(string intercom)
{
	this->intercom = intercom;
}

/**
 * Overloaded Assignment operator; this allows us to "assign"
 * a CameraData to another CameraData; it literally just copies
 * the data in.
 */
CameraData& CameraData::operator=(const CameraData &old)
{
	if(this != &old)
	{
		this->intercom = old.intercom;
		this->url = old.url;
		this->username = old.username;
		this->password = old.password;
	}

	return *this;
}	
#pragma once

#include <string>

using namespace std;

/**
 * The Metadata behind a Web Camera.
 */
class CameraData
{
public:
	/** Default Constructor.  */
	CameraData();

	/** Constructor that sets the intercom name only.  */
	CameraData(string intercom);

	/** Constructor that sets the URL, username and password.  */
	CameraData(string url, string username, string password);

	/** Constructor that sets the Intercom, URL, username and password.  */
	CameraData(string intercom, string url, string username, string password);
    
	/** Destructor.  */
	~CameraData(void);

	// Getters:
	string getUrl();
	string getUsername();
	string getPassword();
	string getIntercom();

	// Setters:
	void setUrl(string url);
	void setUsername(string username);
	void setPassword(string password);
	void setIntercom(string intercom);	
	
	/** Overloaded assignment operator. */
	CameraData& CameraData::operator=(const CameraData& old);

private:

	// private data members:
	string intercom;
	string url;
	string username;
	string password;
};

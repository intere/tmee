// This is the main project file for VC++ application project 
// generated using an Application Wizard.

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <ostream>
#include <io.h>
#include "curl/curl.h"
#include "mjpegtojpeg.h"
#include "MJpegStream.h"
//#using <mscorlib.dll>

using namespace std;

// -----------------------------------------------------------------
// Function declarations (signatures)
// -----------------------------------------------------------------
void getUrlToStdout(void);
void writeUrlToStream(ostream *oss);
void writeUrlToFunction(void);
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
void testWithFile(void);
void writeUrlToFile(string file);
size_t write_data_to_file(void *ptr, size_t size, size_t nmemb, FILE *stream);
void testReadThenWriteFile(void);

// -----------------------------------------------------------------
// Global Data
// -----------------------------------------------------------------
namespace global
{
	MJpegStream mjpegStream;
	long bytesRead;
};

/**
 * main function.
 */ 
int main(void)
{
	global::bytesRead = 0;

	//FILE *file;
	//cout << "Hello World" << endl;

	//MJpegStream out;
	//out << "Testing new stream" << endl;
	//ostream *ref = dynamic_cast<ostream*>(&out);
	//blah(ref);
	
	// TEST 1: Test writing bytes from cURL to a function
	// Tests writing bytes from the URL to a function (write_data):
	//
	//writeUrlToFunction();

	// TEST 2: Collect the bytes from the cURL and write them to a temp file:
	//
	//string filename("c:\\tmp\\curl-out");
	//writeUrlToFile(filename);

	// TEST 3: Tests the MJPEG Stream Parser using the collected file:
	//
	testWithFile();

	// TEST 4: Tests reading a file and writing it out directly:
	// 
	//testReadThenWriteFile();

    return 0;
}

/**
 * Callback function for where data is written to.  The purpose of this
 * function is to pass the data on to the mjpeg stream.
 */
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {

	cout << "read (" << size << ", " << nmemb << ") bytes" << endl;	
	
	for(size_t i=0; i<nmemb; ++i)
	{
		global::mjpegStream.put(((char*)ptr)[i]);
	}
    
	return nmemb;
}

/**
 * Alternate implementation - writes the bytes to the user specified file.
 * This implementation is to be used in conjunction with the "writeUrlToFile" function.
 */
size_t write_data_to_file(void *ptr, size_t size, size_t nmemb, FILE *stream) {

	cout << "read (" << size << ", " << nmemb << ") bytes" << endl;	
	
	write(stream->_file, ptr, nmemb);	
	
	return nmemb;
}

/**
 * Writes the bytes to the provided file.
 */
void writeUrlToFile(string file)
{
	CURL *curl;
    FILE *fp;
    CURLcode res;
    
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(file.c_str(),"wb");
		curl_easy_setopt(curl, CURLOPT_USERNAME, "root");
		curl_easy_setopt(curl, CURLOPT_PASSWORD, "pass");
		curl_easy_setopt(curl, CURLOPT_URL, 
			"http://137.89.235.200/axis-cgi/mjpg/video.cgi?resolution=320x240");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_to_file);
        res = curl_easy_perform(curl);
        
		/* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
}

/**
 * Writes the bytes to a function.
 */
void writeUrlToFunction(void)
{
	CURL *curl;
    //FILE *fp;
    CURLcode res;
    //char outfilename[FILENAME_MAX] = "C:\\bbb.txt";
    curl = curl_easy_init();
    if (curl) {
        //fp = fopen(outfilename,"wb");
		curl_easy_setopt(curl, CURLOPT_USERNAME, "root");
		curl_easy_setopt(curl, CURLOPT_PASSWORD, "pass");
		curl_easy_setopt(curl, CURLOPT_URL, 
			"http://137.89.235.200/axis-cgi/mjpg/video.cgi?resolution=320x240");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        //fclose(fp);
    }
}

/**
 * Default implementation of cURL writes the output fetched to STDOUT.
 */
void getUrlToStdout(void)
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();

	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_USERNAME, "root");
		curl_easy_setopt(curl, CURLOPT_PASSWORD, "pass");
		curl_easy_setopt(curl, CURLOPT_URL, 
			"http://137.89.235.200/axis-cgi/mjpg/video.cgi?resolution=320x240");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0);
		//curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, 0);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
	}
}


/**
 * Tests the cUrl capabilities.
 */
void writeUrlToStream(ostream *oss)
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();

	if(curl) {
		curl_easy_setopt(curl, CURLOPT_USERNAME, "root");
		curl_easy_setopt(curl, CURLOPT_PASSWORD, "pass");
		curl_easy_setopt(curl, CURLOPT_URL, 
			"http://137.89.235.200/axis-cgi/mjpg/video.cgi?resolution=320x240");
		curl_easy_setopt(curl, CURLOPT_FILE, &(*oss));
		res = curl_easy_perform(curl);

		// always cleanup
		curl_easy_cleanup(curl);
	}
}

/**
 * Simulates the API using a flat file as the input.
 */
void testWithFile(void)
{
	string filename = "c:\\tmp\\curl-out";
	ifstream in(filename.c_str(),std::ios_base::binary);

	char c=0;
	
	while(in.peek()!=EOF)
	{
		in.get(c);
		global::mjpegStream.put(c);

		++global::bytesRead;
	}

	in.close();

	cout << "read " << global::bytesRead << " bytes total" << endl;

}

void testReadThenWriteFile(void)
{
	string fin = "c:\\tmp\\test.jpg";
	string fout = "c:\\tmp\\test-out.jpg";
	int bytesRead = 0;

	ifstream in(fin.c_str(),std::ios_base::binary);
	ofstream out(fout.c_str(),std::ios_base::binary);

	char c=0;

	while(in.peek()!=EOF)
	{
		in.get(c);
		out.put(c);
		++bytesRead;
	}

	out.close();
	in.close();

	cout << "read " << bytesRead << " bytes total" << endl;

}

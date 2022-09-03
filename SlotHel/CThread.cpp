#include "pch.h"
#include "CSlotHel.h"
#include "CThreading.h"


namespace
{
	std::size_t callback(
		const char* in,
		std::size_t size,
		std::size_t num,
		std::string* out)
	{
		const std::size_t totalBytes(size * num);
		out->append(in, totalBytes);
		return totalBytes;
	}
}

CThreading::CThreading(){

}


void CThreading::thread_run() {

	this->curl = curl_easy_init();


		while (thr_run.load()) {

			
			/*  DEBUG  */
			{const std::lock_guard<std::mutex> lock(mtx_mes);
			global_message = "Check Json"; }
			//###

			// Do stuff:

			//retrieve temporary json
			this->CurlAuthentification();
			this->json_tmp = this->ReceiveData();

			//push temp json to mutex protected global
			{const std::lock_guard<std::mutex> lock(mtx_json);
				json_stor = this->json_tmp; }


			// lock mutex and cond variable for defined time -> updaterate or until notified (e.g. closing application)
			std::unique_lock<std::mutex> lk(mtx);
			using namespace std::chrono_literals;
			cv.wait_for(lk, updaterate.load() * 1000ms);

		}
		
		curl_easy_cleanup(this->curl);
}

void CThreading::CurlAuthentification() {

	//const std::string url = "http://192.168.0.4/data/LOWW.standard.departure.json"; //debug
	//const std::string url = SLOT_SYSTEM_PATH + AIRPORT + ".standard.departure.json";
	const std::string url = SLOT_SYSTEM_PATH + "LOWW.standard.departure.json";

	// the actual code has the actual url string in place of <my_url>
	curl_easy_setopt(this->curl, CURLOPT_URL, url.c_str());

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	// the actual code has the actual token in place of <my_token>
	headers = curl_slist_append(headers, "Authorization: Basic <my_token>");
	curl_easy_setopt(this->curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

	curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, headers);

	// Don't bother trying IPv6, which would increase DNS resolution time.
	curl_easy_setopt(this->curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

	// Don't wait forever, time out after 5 seconds.
	curl_easy_setopt(this->curl, CURLOPT_TIMEOUT, 5);

	// Follow HTTP redirects if necessary.
	curl_easy_setopt(this->curl, CURLOPT_FOLLOWLOCATION, 1L);
}

json CThreading::ReceiveData()
{
	try {

		// Response information.
		long httpCode(0);
		std::unique_ptr<std::string> httpData(new std::string());

		// Hook up data handling function.
		curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, callback);

		// Hook up data container (will be passed as the last parameter to the
		// callback handling function).  Can be any pointer type, since it will
		// internally be passed as a void pointer.
		curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, httpData.get());

		// Run our HTTP GET command, capture the HTTP response code, and clean up.
		curl_easy_perform(this->curl);
		curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &httpCode);


		if (httpCode == 200)
		{
			const std::lock_guard<std::mutex> lock(mtx_mes);
			global_message = "Successfully data retrieved";

			return json::parse(*httpData.get());
		}
		else
		{	
			const std::lock_guard<std::mutex> lock(mtx_mes);
			global_error = "TimeOut or No Connection, Code: " + std::to_string(httpCode);

			return NULL;
		}

	}
	catch (std::exception e)
	{	
		const std::lock_guard<std::mutex> lock(mtx_mes);
		global_error = "Failed to read slot data from web system. Error: " + std::string(e.what());

		return NULL;
	}
}

void CThreading::SendData() {

	// Response information.
	long httpCode(0);
	std::unique_ptr<std::string> httpData(new std::string());


	curl_easy_perform(this->curl);
	curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &httpCode);

	if (httpCode == 200)
	{
		const std::lock_guard<std::mutex> lock(mtx_mes);
		global_message = "SEND - successful";

	}
	else
	{
		const std::lock_guard<std::mutex> lock(mtx_mes);
		global_error = "SEND - TimeOut or No Connection, Code: " + std::to_string(httpCode) + " - " + SLOT_AUTH_PATH;

	}



}


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



		while (thr_run.load()) {

			
			/*  DEBUG  */
			{const std::lock_guard<std::mutex> lock(mtx_mes);
			global_message = "Check Json"; }
			//###

			// Do stuff:

			//retrieve temporary json
			this->json_tmp = this->ConnectJson();

			//push temp json to mutex protected global
			{const std::lock_guard<std::mutex> lock(mtx_json);
				json_stor = this->json_tmp; }


			// lock mutex and cond variable for defined time -> updaterate or until notified (e.g. closing application)
			std::unique_lock<std::mutex> lk(mtx);
			using namespace std::chrono_literals;
			cv.wait_for(lk, updaterate.load() * 1000ms);

		}
}

json CThreading::ConnectJson()
{

	try {
		//mtx.lock(); //lock for AIRPORT variable (could have been changed in the meantime by mainthread)

		//const std::string url = SLOT_SYSTEM_PATH + AIRPORT + ".standard.departure.json";
		const std::string url = SLOT_SYSTEM_PATH + "LOWW.standard.departure.json";

		//mtx.unlock();

		//const std::string url = "http://192.168.0.4/data/LOWW.standard.departure.json"; //debug

		CURL* curl = curl_easy_init();

		// Set remote URL.
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Don't bother trying IPv6, which would increase DNS resolution time.
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

		// Don't wait forever, time out after 5 seconds.
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

		// Follow HTTP redirects if necessary.
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		// Response information.
		long httpCode(0);
		std::unique_ptr<std::string> httpData(new std::string());

		// Hook up data handling function.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

		// Hook up data container (will be passed as the last parameter to the
		// callback handling function).  Can be any pointer type, since it will
		// internally be passed as a void pointer.
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

		// Run our HTTP GET command, capture the HTTP response code, and clean up.
		curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		curl_easy_cleanup(curl);

		if (httpCode == 200)
		{
			const std::lock_guard<std::mutex> lock(mtx_mes);
			global_message = "Successfully data retrieved";

			return json::parse(*httpData.get());
		}
		else
		{	
			const std::lock_guard<std::mutex> lock(mtx_mes);
			global_error = "TimeOut or No Connection, Code: " + std::to_string(httpCode) + " - " + url;

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


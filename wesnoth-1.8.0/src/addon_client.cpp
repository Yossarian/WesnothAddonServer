#include "addon_client.hpp"
#include <sstream>

using namespace network;

addon_client_error::addon_client_error(std::string why) :
std::runtime_error(why)
{
}

addon_client::addon_client(void)
{
	handle_ = curl_easy_init();

	//DEBUG: make verbose
	//TODO: get rid of this in production code
	//curl_easy_setopt(handle_, CURLOPT_VERBOSE, 1);

	//autoadd http headers
	curl_easy_setopt(handle_, CURLOPT_HEADER, 1);

	//put errors in readable form into error_buffer
	curl_easy_setopt(handle_, CURLOPT_ERRORBUFFER, error_buffer_);

	//set default callback
	curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, addon_client::default_recv_callback);

	//do not include http  header in output
	curl_easy_setopt(handle_, CURLOPT_HEADER, 0);
}

addon_client::~addon_client(void)
{
	curl_easy_cleanup(handle_);
}

void addon_client::flush()
{
	CURLcode error = curl_easy_perform(handle_);
	if(error != CURLE_OK)
		throw addon_client_error(error_buffer_);
}

void addon_client::set_base_url(std::string base_url)
{
	base_url_ = base_url;
}

std::string addon_client::get_addon_description(unsigned int addon_id)
{
	std::ostringstream address;
	address << base_url_ << "details/" << addon_id <<"/?simple_iface";

	//setup data to pass to callback
	std::string buffer;
	curl_easy_setopt(handle_, CURLOPT_WRITEDATA, &buffer);

	//setup actual url
	curl_easy_setopt(handle_, CURLOPT_URL, address.str().c_str());
	flush();
	return buffer;
}

std::string addon_client::get_addon_list()
{
	std::ostringstream address;
	address << base_url_<<"?simple_iface";

	//setup data to pass to callback
	std::string buffer;
	curl_easy_setopt(handle_, CURLOPT_WRITEDATA, &buffer);

	//setup actual url
	curl_easy_setopt(handle_, CURLOPT_URL,address.str().c_str());
	flush();
	return buffer;
}

/*std::vector<char> addon_client::get_addon_file(unsigned int addon_id);*/
//Might want to use a different callback then default for big files!

size_t addon_client::default_recv_callback
		(void* buffer, size_t size, size_t nmemb, void* userp)
{
	//userp will be a std::string passed by an addon_client method
	//buffer size is size*nmemb
	//I have no idea what nmemb stands for.
	(static_cast<std::string*>(userp))->append((char*)buffer, size*nmemb);
	//if return size differs from total size, 
	//an error is returned by curl_easy_perform
	return size*nmemb;	
}

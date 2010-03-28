#ifndef ADDON_CLIENT_HPP_INCLUDED
#define ADDON_CLIENT_HPP_INCLUDED

#include <exception>
#include <string>
#include <vector>
#include <curl\curl.h>

namespace network {

/*
exception thrown by various unsafe methods of addon_client
*/
class addon_client_error :
	public std::exception
{
public:
	addon_client_error(std::string why)
	{
		std::exception(why.c_str());
	}
};

/*
HTTP client to access remote addon server
example usage:

network::addon_client ac;
ac.set_base_url("http://localhost:8000/addons/");
std::cout<<ac.get_addon_list()<<std::endl;
std::cout<<ac.get_addon_description(2)<<std::endl;

*/
class addon_client
{
protected:
	std::string base_url_;
	CURL* handle_;
	char error_buffer_[CURL_ERROR_SIZE];

	void flush();

	static size_t default_recv_callback
		(void* buffer, size_t size, size_t nmemb, void* userp);

public:
	addon_client(void);
	~addon_client(void);

	//include 'http://' in base_url, ex. http://wesnoth.org/addons
	//no trailing slash!
	void set_base_url(std::string base_url);

	std::string get_addon_description(unsigned int addon_id);

	std::string get_addon_list(/* TODO: filters? */);

	//std::vector<char> get_addon_file(unsigned int addon_id); not yet implemented

	//TODO: callback registering methods
	//or perhaps even dialog registering?
};

}

#endif
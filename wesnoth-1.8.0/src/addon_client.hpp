#ifndef ADDON_CLIENT_HPP_INCLUDED
#define ADDON_CLIENT_HPP_INCLUDED

#include "config.hpp"

#include <stdexcept>
#include <string>
#include <vector>
#include <curl/curl.h>

namespace network {

/*
exception thrown by various unsafe methods of addon_client
*/
class addon_client_error :
	public std::runtime_error
{
public:
	addon_client_error(std::string why);
};

/*
HTTP client to access remote addon server
example usage:

network::addon_client ac;
ac.set_base_url("http://localhost:8000/addons");
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

	//include 'http://' in base_url, ex. http://wesnoth.org/addons/
	void set_base_url(std::string base_url);

	std::string get_addon_description(unsigned int addon_id);

	std::string get_addon_list(/* TODO: filters? */);
	config get_addon_list_cfg();
	config get_addon_cfg(unsigned int addon_id);
	config get_addon_cfg(std::string addon_name);
	bool is_pbl_valid(const config& pbl, std::string& error_message);
	void publish_addon(const config& addon, std::string login, std::string pass);
	void delete_remote_addon(std::string addon_name, std::string login, std::string pass);

	//TODO: callback registering methods
	//or perhaps even dialog registering?
};

}

#endif
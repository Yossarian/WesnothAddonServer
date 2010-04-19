#include "addon_client.hpp"
#include "foreach.hpp"
#include "serialization/parser.hpp"
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

	curl_easy_setopt(handle_, CURLOPT_FOLLOWLOCATION, true);
}

addon_client::~addon_client(void)
{
	curl_easy_cleanup(handle_);
}

std::string addon_client::url_encode(std::string raw_string) const
{
	//this may be slow but it shouldn't become a bottleneck
	//during any sane use case
	//optimize only if you really need to
	std::ostringstream encoded;
	foreach(char c, raw_string)
	{
		//note: you could just convert /all/ chars to %hex_val and it would be
		//proper but typically only special chars are escaped in such a way
		if ( (48 <= c && c <= 57) ||//0-9
			(65 <= c && c <= 90) ||//abc...xyz
			(97 <= c && c <= 122) || //ABC...XYZ
			(c=='~' || c=='!' || c=='*' || c=='(' || c==')' || c=='\''))
		{
			encoded << c;
		}
		else
		{
			encoded << '%' << std::hex << int(c);
		}
	}
	return encoded.str();
}

std::string addon_client::get_response(std::string url, 
		string_map_t arguments,
		bool post)
{
	//convert arguments to an encoded string like key=value?other+key=1
	std::ostringstream params;
	for(string_map_t::iterator i = arguments.begin(); i != arguments.end(); i++)
		params << '?' << url_encode(i->first) << '=' << url_encode(i->second);

	if(post)
	{
		//set post body
		curl_easy_setopt(handle_, CURLOPT_POSTFIELDS, (void*)(params.str().c_str()));
	}
	else //GET
	{
		//just slap the params to the url
		url += params.str();
	}

	//setup actual url
	curl_easy_setopt(handle_, CURLOPT_URL, url.c_str());

	//setup data to pass to callback
	std::string buffer;
	curl_easy_setopt(handle_, CURLOPT_WRITEDATA, &buffer);

	//execute
	flush();
	return buffer;
}

void addon_client::flush()
{
	CURLcode error = curl_easy_perform(handle_);
	if(error != CURLE_OK)
		throw addon_client_error(error_buffer_);
}

void addon_client::set_base_url(std::string base_url)
{
	//TODO trailing slash bulletproofing
	base_url_ = base_url;
}

std::string addon_client::get_addon_description(unsigned int addon_id)
{
	std::ostringstream address;
	address <<base_url_<< "details/" << addon_id <<"/?simple_iface";
	
	return get_response(address.str());
}

std::string addon_client::get_addon_list()
{
	std::ostringstream address;
	address <<base_url_;//<<"?simple_iface";
	string_map_t args;
	args["simple_iface"] = "1";

	return get_response(address.str(), args);;
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

config addon_client::get_addon_list_cfg()
{
	std::string list = get_addon_list();
	config cfg;
	read(cfg, list);
	return cfg;
}

config addon_client::get_addon_cfg(unsigned int addon_id)
{
	std::ostringstream id;
	id << addon_id;
	return get_addon_cfg(id.str());
}

config addon_client::get_addon_cfg(std::string addon_name)
{
	std::ostringstream address;
	address <<base_url_<< "download/" << url_encode(addon_name) <<"/?wml";

	std::string buffer = get_response(address.str());
	config cfg;
	read(cfg, buffer);
	return cfg;
}

//bool addon_client::is_addon_valid(const config& pbl, std::string login, std::string pass, std::string& error_message);
void addon_client::publish_addon(const config& addon, std::string login, std::string pass)
{
	std::ostringstream parsed_config;
	write(parsed_config, addon);
	string_map_t args;
	args["login"] = login;
	args["password"] = pass;
	args["wml"] = parsed_config.str();

	std::ostringstream address;
	address <<base_url_<< "publish/";
	std::string response = get_response(address.str(), args, true);
}
//void addon_client::delete_remote_addon(std::string addon_name, std::string login, std::string pass);
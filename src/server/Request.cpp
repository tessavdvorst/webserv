#include "Request.hpp"
#include <iomanip>

// ============================= CONSTRUCTOR ===================================

Request::Request() {
	// this->_packet_header = new std::vector<char>();
	// this->_packet_body = new std::vector<char>();
}

Request::Request(std::vector<char> header, std::vector<char> body,
		std::vector<ServerBlock*> config)
{
	this->_packet_header.erase(this->_packet_header.begin(), this->_packet_header.end());
	this->_packet_header.insert(this->_packet_header.end(), header.begin(), header.end());
	this->_packet_body.erase(this->_packet_body.begin(), this->_packet_body.end());
	this->_packet_body.insert(this->_packet_body.end(), body.begin(), body.end());

	this->_config = config;

	parse_request();
}

Request::Request(const Request& that)
{
	*this = that;
}

// ============================= DESTRUCTOR ===================================

Request::~Request() {
	// delete this->_packet_header;
	// delete this->_packet_body;
}

// ====================== ASSIGN OPERATOR OVERLOAD ============================

Request& Request::operator=(const Request& that)
{
	this->_packet_header.erase(this->_packet_header.begin(), this->_packet_header.end());
	this->_packet_header.insert(this->_packet_header.end(), that._packet_header.begin(), that._packet_header.end());
	this->_packet_body.erase(this->_packet_body.begin(), this->_packet_body.end());
	this->_packet_body.insert(this->_packet_body.end(), that._packet_body.begin(), that._packet_body.end());
	this->_method = that._method;
	this->_url = that._url;
	this->_decoded_url = that._decoded_url;
	this->_index_url = that._index_url;
	this->_local_url = that._local_url;
	this->_full_url = that._full_url;
	this->_protocol = that._protocol;
	this->_host = that._host;
	this->_content_type = that._content_type;
	this->_port = that._port;
	this->_config = that._config;
	return (*this);
}

//for what is this needed?
std::vector<char>::size_type Request::get_max_client_size_dummy() {
	return (30000000);
}

std::string Request::url_decode(std::string in) {
	std::string out;
	size_t start_idx;
	while ((start_idx = in.find("%")) != std::string::npos) {
		out += in.substr(0, start_idx);
		std::string temp = in.substr(start_idx + 1, 2);
		char c = static_cast<char>(strtol(temp.c_str(), NULL, 16));
		out += c;
		in = in.substr(start_idx + 3, std::string::npos);
	}
	out += in;
	return (out);
}

// ================================ PARSING ===================================

void Request::parse_request(void)
{
	std::stringstream in;
	std::copy(this->_packet_header.begin(), this->_packet_header.end(), std::ostream_iterator<char>(in));

	parse_request_line(in);
	parse_header(in);
	parse_url();
}

void Request::parse_request_line(std::stringstream& in)
{
	std::string line;
	std::getline(in, line);

	std::stringstream stream_line(line);
	std::getline(stream_line, this->_method, ' ');
	std::getline(stream_line, this->_url, ' ');
	std::getline(stream_line, this->_protocol, ' ');
}

void Request::parse_header(std::stringstream& in)
{
	std::string line;

	in.peek();
	while (!in.eof())
	{
		std::string key, value;
		std::getline(in, line);
		if (line.empty()) // body
			break;
		std::stringstream temp(line);
		std::getline(temp, key, ' ');
		std::getline(temp, value, '\r');
		if (key == "Host:")
			this->_host = value;
		if (key == "Content-Type:" && this->_content_type.length() == 0)
			this->_content_type = value;
		in.peek();
	}

	//This is for the directory listing so we can direct the client to the right port.
	this->_port = "80";
	if (this->_host.find(":") != std::string::npos) {
		this->_port = this->_host.substr(this->_host.find(":") + 1);
		this->_host = this->_host.substr(0, this->_host.find(":"));
	}
}

void Request::parse_url(void)
{
	this->_decoded_url = url_decode(this->_url);

	// TODO: redirections.
	this->_local_url = this->get_directive("root")[0] + this->_decoded_url;
	std::string return_code = "200";
	std::string payload = "";
	std::string::size_type where;
	
	while ((where = this->_local_url.find("//")) != std::string::npos) {
		this->_local_url.replace(where, 2, "/");
	}
	if ((*this->_local_url.rbegin()) == '/')
		this->_local_url.erase(this->_local_url.length() - 1);
	this->_full_url = this->_local_url;
	if (this->_local_url.find('?') != std::string::npos)
		this->_local_url.erase(this->_local_url.find('?'));
	if (this->_decoded_url.find('?') != std::string::npos)
		this->_decoded_url.erase(this->_decoded_url.find('?'));
	if (is_dir()) {
		this->_local_url += '/';
		this->_index_url = this->_local_url + this->get_directive("index")[0];
		this->_decoded_url += '/' + this->get_directive("index")[0];
	} else {
		this->_index_url = this->_local_url;
	}
}

bool Request::is_dir(void)
{
	struct stat s;

 	if (stat(this->_local_url.c_str(), &s) == 0) {
		if (s.st_mode & S_IFDIR)
			return (true);
	}
	return (false);
}

// ============================== FIND SERVERBLOCK ==================================

ServerBlock *Request::get_server(void) {
	ServerBlock *default_server = NULL;
	for (std::vector<ServerBlock*>::iterator block = this->_config.begin(); block != this->_config.end(); block++) {
		std::vector<std::string> names = ((*block)->get_server_name());
		for (std::vector<std::string>::iterator server_name = names.begin(); server_name != names.end(); server_name++) {
			if (!default_server && (*server_name) == "_")
				default_server = (*block);
			if ((*server_name) == this->_host)
				return (*block);
		}
	}
	if (default_server)
		return (default_server);
	return (this->_config[0]);
}

// =================================== GETTERS ======================================

std::vector<std::string> Request::get_directive(std::string directive) { return (this->get_server()->get_directive(directive, this->_url)); }

std::string Request::get_url(void) { return (this->_url); }

std::string Request::get_index_url(void) { return (this->_index_url); }

std::string Request::get_decoded_url(void) { return (this->_decoded_url); }

std::string Request::get_full_url(void) { return (this->_full_url); }

std::string Request::get_local_url(void) { return (this->_local_url); }

std::string Request::get_request_url(void) { return (this->_url); }

std::string Request::get_method(void) { return (this->_method); }

std::string Request::get_protocol(void) { return (this->_protocol); }

std::string Request::get_host(void) { return (this->_host); }

std::string Request::get_content_type(void) { return (this->_content_type); }

std::string Request::get_port(void) { return (this->_port); }

std::vector<char> Request::get_packet_body(void) { return (this->_packet_body); }

void		Request::set_packet_body(std::vector<char> body) {
	this->_packet_body.erase(this->_packet_body.begin(), this->_packet_body.end());
	this->_packet_body.insert(this->_packet_body.end(), body.begin(), body.end());
}

#include "Response.hpp"

// ============================= CONSTRUCTOR ===================================

Response::Response() {}

Response::Response(Request request)
{
	this->_request = request;
	this->_index_url = this->_request.get_index_url();
	this->_infile.open(this->_index_url.c_str(), std::ios::in | std::ios::binary | std::ios::ate/* | O_NONBLOCK*/ ); // O_NONBLOCK gets rejected by GCC because of a type mismatch
	this->_need_cgi = false;
	this->_request_type = check_request_type();
	try {
		// this->_payload = handle_request();
		handle_request();
		this->_headers = "HTTP/1.1 200 OK\r\ntransfer-encoding: chunked\r\n\r\n";
	} catch (Response::HTTPException &e) {
		try {
			if (e.get_error_code().substr(0, 1) == "3") {
				// handle redirection?
				if (this->_infile.good())
					this->_infile.close();
				this->_headers = std::string("HTTP/1.1 ") + e.get_error_code() + "OK\r\nLocation: " + e.get_target() + "\r\n\r\n";
				this->_payload = "";
			} else
				serve_config_error_page(e);
		} catch (Response::HTTPException &e2) {
			handle_error_pages(e2);
		}
	} catch (Response::CGIDeferer &e) {
		this->_need_cgi = true;
	}
}

Response::Response(const Response& that)
{
	*this = that;
}

// ================================ DESTRUCTOR =====================================

Response::~Response() {}

// ========================= ASSIGN OPERATOR OVERLOAD ==============================

Response& Response::operator=(const Response& that)
{
	this->_index_url = that._index_url;
	if (this->_infile.good())
		this->_infile.close();
	this->_headers = that._headers;
	this->_payload = that._payload;
	this->_need_cgi = that._need_cgi;
	this->_request = that._request;
	this->_request_type = that._request_type;
	return (*this);
}

// ============================= HANDLE REQUEST ===================================

RequestType Response::check_request_type(void)
{
	std::vector<std::string> allowed_methods = this->_request.get_directive("allowed_methods");

	if (allowed_methods[0].length() == 0 && this->_request.get_method() != "GET")
		return (NOT_ALLOWED);
	if (allowed_methods[0].length() != 0 && std::find(allowed_methods.begin(), allowed_methods.end(), this->_request.get_method()) == allowed_methods.end())
		return (NOT_ALLOWED);
	else if (this->_request.get_method() == "DELETE" && std::find(allowed_methods.begin(), allowed_methods.end(), "DELETE") != allowed_methods.end())
		return (DELETE);
	else if (this->_request.get_method() == "POST" && std::find(allowed_methods.begin(), allowed_methods.end(), "POST") != allowed_methods.end())
		return (POST);
	return (GET);
}

void Response::handle_request(void) {

	if (this->_request_type == NOT_ALLOWED)
		throw Response::HTTPException("405");
	else if (this->_request.get_directive("return").size() != 1)
		throw Response::HTTPException(this->_request.get_directive("return")[0], this->_request.get_directive("return")[1]);	// } else if (this->_request.get_method() == "DELETE" && std::find(allowed_methods.begin(), allowed_methods.end(), "DELETE") != allowed_methods.end()) {
	else if (this->_request_type == DELETE) {
		if (this->_infile.good()) {
			this->_infile.close();
			if (remove(this->_index_url.c_str()))
				throw Response::HTTPException("500"); // maybe be more explicit about why it failed?
			else
				this->_payload = "<!doctype html><html><head><title>Successfully removed</title></head><body><h1>200</h1>The file you requested has been successfully deleted.</body></html>";
		} else
			throw Response::HTTPException("404");
	} else {
		if (!this->_infile.good() && this->_request.get_directive("autoindex")[0] == "on")
			serve_directory_listing();
		else if (!this->_infile.good())
			throw Response::HTTPException("404");
		else {
			if (this->_request.get_directive("cgi_pass").size() == 2 && this->_request.get_index_url().find(this->_request.get_directive("cgi_pass")[0]) != std::string::npos && (this->_request.get_index_url().length() - this->_request.get_index_url().rfind(this->_request.get_directive("cgi_pass")[0])) == this->_request.get_directive("cgi_pass")[0].length())
				throw Response::CGIDeferer();
			this->_payload = get_file_data();
		}
	}
}

// void Response::handle_request(void) {

// 	// std::vector<std::string> allowed_methods = this->_request.get_directive("allowed_methods");
// 	// if (allowed_methods[0] != "" && std::find(allowed_methods.begin(), allowed_methods.end(), this->_request.get_method()) == allowed_methods.end()) {
// 	if (this->_request_type == NOT_ALLOWED)
// 		throw Response::HTTPException("405");
// 	// if allowed and delete
// 	else if (this->_request.get_directive("return").size() != 1)
// //		std::cout << "Redirect" << std::endl;
// 		throw Response::HTTPException(this->_request.get_directive("return")[0], this->_request.get_directive("return")[1]);
// 	// } else if (this->_request.get_method() == "DELETE" && std::find(allowed_methods.begin(), allowed_methods.end(), "DELETE") != allowed_methods.end()) {
// 	else if (this->_request_type == DELETE) {
// //		std::cout << "DELETE" << std::endl;
// 		if (this->_infile.good()) {
// 			this->_infile.close();
// 			if (remove(this->_index_url.c_str())) { // Can't remove
// 				throw Response::HTTPException("500"); // maybe be more explicit about why it failed?
// 			} else {
// //				std::cout << "File removed..." << std::endl;
// 				this->_payload = "<!doctype html><html><head><title>Successfully removed</title></head><body><h1>200</h1>The file you requested has been successfully deleted.</body></html>";
// 			}
// 		} else {
// 			throw Response::HTTPException("404");
// 		}
// 	// if allowed and not delete
// 	} else {
// 		// if post, notify user
// //		if (this->_request.get_method() == "POST" && std::find(allowed_methods.begin(), allowed_methods.end(), "POST") != allowed_methods.end())
// //			std::cout << "POST: ";
// 		//if method-allowed-file-not-found-auto-index-on
// 		if (!this->_infile.good() && this->_request.get_directive("autoindex")[0] == "on") {
// 			serve_directory_listing();
// 		// if method-allowed-file-not-found-auto-index-off
// 		} else if (!this->_infile.good()) {
// 			throw Response::HTTPException("404");
// 		} else { // if method-allowed-file-found
// //			std::cout << "{" << this->_request.get_directive("cgi_pass").size() << ", " << this->_request.get_request_url() << ", " << this->_request.get_directive("cgi_pass")[0] << "}" << std::endl;
// 			if (this->_request.get_directive("cgi_pass").size() == 2 && this->_request.get_index_url().find(this->_request.get_directive("cgi_pass")[0]) != std::string::npos)
// 				throw Response::CGIDeferer();
// //			std::cout << "Returning file..." << std::endl;

// 			// std::ifstream::pos_type file_size = this->_infile.tellg();
// 			// this->_infile.seekg(0, std::ios::beg);
// 			// std::vector<char> bytes(file_size);
// 			// this->_infile.read(bytes.data(), file_size);
// 			// this->_payload = std::string(bytes.data(), file_size);
// 			// this->_infile.close();
// 			this->_payload = get_file_data();
// 		}
// 	}
// }

// =============================== HANDLE ERROR =====================================

void Response::serve_config_error_page(Response::HTTPException &e)
{
	if (this->_request.get_directive(std::string("error_page") + e.get_error_code())[0].length() == 0)
		throw Response::HTTPException(e.get_error_code(), false);
	if (this->_infile.good())
		this->_infile.close();
	this->_infile.open(this->_request.get_directive(std::string("error_page") + e.get_error_code())[0].c_str(), std::ios::in | std::ios::binary | std::ios::ate /* | O_NONBLOCK */ ); // O_NONBLOCK rejected by GCC
	if (!this->_infile.good())
		throw Response::HTTPException(e.get_error_code(), false);
	
	// std::ifstream::pos_type file_size = this->_infile.tellg();
	// this->_infile.seekg(0, std::ios::beg);
	// std::vector<char> bytes(file_size);
	// this->_infile.read(bytes.data(), file_size);
	// this->_payload = std::string(bytes.data(), file_size);
	// this->_infile.close();
	std::string error_code = e.get_error_code();
	this->_headers = "HTTP/1.1 " + error_code + " OK\r\ntransfer-encoding: chunked\r\n\r\n";
	this->_payload = get_file_data();
}


std::string Response::get_error_msg(std::string error_code)
{
	if (error_code == "404")
		return ("The requested URL can't be found on this server.");
	else if (error_code == "405")
		return ("The requested method is not allowed on this server.");
	else if (error_code == "413")
		return ("The request you made was too large for this server.");
	else
		return ("The server encountered an internal error.");
}

void Response::handle_error_pages(Response::HTTPException &e)
{
	if (this->_infile.good())
		this->_infile.close();
	if (e.is_soft())
		throw std::exception();

	std::string error_code = e.get_error_code();
	if (!(error_code == "404" || error_code == "405" || error_code == "413"))
		error_code = "500";
	std::string error_msg = get_error_msg(error_code);
	this->_headers = "HTTP/1.1 " + error_code + " OK\r\ntransfer-encoding: chunked\r\n\r\n";
	this->_payload = "<!doctype html>\r\n<html>\r\n<head>\r\n<title>Error " + error_code + "</title>\r\n</head><body><h1>" + error_code + "</h1>\r\n<br />\r\n<p>" + error_msg + "</p>\r\n</body>\r\n</html>";

	// this->_headers = "HTTP/1.1 " + e.get_error_code() + " OK\r\ntransfer-encoding: chunked\r\n\r\n";
	// if (e.get_error_code() == "404")
	// 	this->_payload = "<!doctype html>\r\n<html>\r\n<head>\r\n<title>Error 404</title>\r\n</head><body><h1>404</h1>\r\n<br />\r\n<p>The requested URL can't be found on this server.</p>\r\n</body>\r\n</html>";
	// else if (e.get_error_code() == "405")
	// 	this->_payload = "<!doctype html>\r\n<html>\r\n<head>\r\n<title>Error 405</title>\r\n</head><body><h1>405</h1>\r\n<br />\r\n<p>The requested method is not allowed on this server.</p>\r\n</body>\r\n</html>";
	// else if (e.get_error_code() == "500")
	// 	this->_payload = "<!doctype html>\r\n<html>\r\n<head>\r\n<title>Error 500</title>\r\n</head><body><h1>500</h1>\r\n<br />\r\n<p>The server encountered an internal error.</p>\r\n</body>\r\n</html>";
	// else { // Treat other errors as error 500
	// 	this->_headers = "HTTP/1.1 500 OK\r\ntransfer-encoding: chunked\r\n\r\n";
	// 	this->_payload = "<!doctype html>\r\n<html>\r\n<head>\r\n<title>Error 500</title>\r\n</head><body><h1>500</h1>\r\n<br />\r\n<p>The server encountered an internal error.</p>\r\n</body>\r\n</html>";
	// }
}

// =========================== SERVE DIRECTORY LISTING =================================

void Response::serve_directory_listing(void)
{
	DIR *dir = opendir(this->_request.get_local_url().c_str());
	if (dir) {
//		std::cout << "Returning directory listing..." << std::endl;
		this->_payload = "<!doctype html><html><head><title>Directory listing for " + this->_request.get_request_url() + "</title></head><body>";
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL) {
			this->_payload += "<a href=\"http://" + this->_request.get_host() + ":" + this->_request.get_port() + this->_request.get_request_url() + entry->d_name + ((entry->d_type == DT_DIR) ? "/" : "") + "\">" + entry->d_name + ((entry->d_type == DT_DIR) ? "/" : "") + "</a><br />";
		}
		this->_payload += "</body></html>";
		closedir(dir);
	} else {
		throw Response::HTTPException("404");
	}
}

// ================================= CGI CHECK ==========================================

bool		Response::need_cgi(void) { return (this->_need_cgi); }

// ================================== GETTERS ===========================================

std::string Response::get_file_data(void)
{
	std::string payload;

	std::ifstream::pos_type file_size = this->_infile.tellg();
	this->_infile.seekg(0, std::ios::beg);
	std::vector<char> bytes(file_size);
	this->_infile.read(bytes.data(), file_size);
	payload = std::string(bytes.data(), file_size);
	this->_infile.close();

	return (payload);
}

std::string	Response::get_payload(void) { return (this->_payload); }

std::string	Response::get_headers(void) { return (this->_headers); }

RequestType Response::get_request_type(void) { return (this->_request_type); }

// ================================== SETTERS ===========================================

void		Response::set_payload(std::string payload) { this->_payload = payload; }

void		Response::set_headers(std::string headers) { this->_headers = headers; }
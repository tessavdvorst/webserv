#include "Client.hpp"

Client::Client(): _fd(0), _listener(0), _status(READY_TO_READ) {
	// this->_header = new std::vector<char>();
}

Client::Client(int fd, int listener, Server* server): _fd(fd), _listener(listener),
	_status(READY_TO_READ), _server(server) {
	// this->_header = new std::vector<char>();
}

Client::Client(const Client& that)
{
	// this->_header = new std::vector<char>();
	*this = that;
}

Client::~Client() {
	// delete this->_header;
	// if (this->_body)
	// 	delete this->_body;
//	logger.log(Logger::DEBUG) << "Client Destructor called";
	if (this->_cgi)
		delete this->_cgi;
	// close(this->_fd);
}

Client& Client::operator=(const Client& that)
{
	this->_fd = that.get_fd();
	this->_listener = that.get_listener();
	this->_status = that.get_status();
	this->_header.erase(this->_header.begin(), this->_header.end());
	this->_header.insert(this->_header.end(), that.get_header().begin(), that.get_header().end());
	this->_body.erase(this->_body.begin(), this->_body.end());
	this->_body.insert(this->_body.end(), that.get_body().begin(), that.get_body().end());
	this->_server = that._server;
	this->_request = that._request;
	return (*this);
}

void Client::update_status(ClientStatus newStatus)
{
	this->_status = newStatus;
}

void Client::parse_request(void)
{
	this->_request = Request(this->_header, this->_body, this->_server->get_server_blocks());
	update_status(HEADER_PARSED);
}

void Client::generate_response(void)
{
	this->_response = Response(this->_request);
	if (this->_response.need_cgi())
	{
		this->_cgi = new CGI(this->_request, this->_response);
		update_status(FORK_SET_PIPES);
	}
	else
	{
		this->_cgi = NULL;
		update_status(READY_TO_SEND);
	}
}

bool Client::process_buffer(char *buffer, int bytes_read)
{
	std::vector<char> crlfcrlf;
	crlfcrlf.push_back('\r');
	crlfcrlf.push_back('\n');
	crlfcrlf.push_back('\r');
	crlfcrlf.push_back('\n');

	this->_header.insert(this->_header.end(), buffer, &(buffer[bytes_read]));

	if (std::search(this->_header.begin(), this->_header.end(), crlfcrlf.begin(), crlfcrlf.end()) != this->_header.end()) {
		set_body(this->_header);
		this->_body.erase(this->_body.begin(), std::search(this->_body.begin(), this->_body.end(), crlfcrlf.begin(), crlfcrlf.end()) + 4);
		this->_header.erase(std::search(this->_header.begin(), this->_header.end(), crlfcrlf.begin(), crlfcrlf.end()), this->_header.end());
		return (true);
	}

	return (false);
}

bool Client::process_cgi_buffer(char *buffer, int bytes_read)
{
	std::vector<char> crlfcrlf;
	crlfcrlf.push_back('\r');
	crlfcrlf.push_back('\n');
	crlfcrlf.push_back('\r');
	crlfcrlf.push_back('\n');

	std::vector<char> old_header = this->_cgi->get_header();
	old_header.insert(old_header.end(), buffer, &(buffer[bytes_read]));

	this->_cgi->set_header(old_header);

	if (std::search(this->_cgi->get_header().begin(), this->_cgi->get_header().end(), crlfcrlf.begin(), crlfcrlf.end()) != this->_cgi->get_header().end()) {
		this->_cgi->set_body(this->_cgi->get_header());
		this->_cgi->erase_body();
		this->_cgi->erase_header();
		return (true);
	}

	return (false);
}

void Client::read_header(int fd, bool is_cgi)
{
	char buffer[8193];
	int bytes_received;

	if (is_cgi) {
		update_status(CGI_HEADER);
		bytes_received = read(fd, buffer, 8192);
	} else {
		update_status(READING_HEADER);
		bytes_received = recv(fd, buffer, 8192, 0);
	}
/*	if (bytes_received == 0) {
		if (is_cgi) {
			update_status(CGI_BODY);
		} else {
			update_status(HEADER_READ);
		}
		return ; // throw exception ??(close_and_remove(fd));
	}
	else*/ if (bytes_received < 0)
		return ; // throw exception ??(close_and_remove(fd));
	buffer[bytes_received] = '\0';

	if (is_cgi && process_cgi_buffer(buffer, bytes_received))
		update_status(CGI_BODY);
	else if (!is_cgi && process_buffer(buffer, bytes_received))
		update_status(HEADER_READ);
}

void Client::read_body(int fd, bool is_cgi)
{
	char buffer[8193];
	int bytes_received;

	if (is_cgi)
		bytes_received = read(fd, buffer, 8192);
	else
		bytes_received = recv(fd, buffer, 8192, 0);

	if (bytes_received == 0)
	{
		if (is_cgi) {
			update_status(CGI_TO_PAYLOAD);
		} else {
			this->set_request_body(this->_body);
			this->_cgi->set_request(this->_request);
			update_status(RUN_CGI_POST);
		}
	}
	else if (bytes_received < 0)
		throw SocketMonitor::CloseFdException(fd); // throw exception ??(close_and_remove(fd));
	buffer[bytes_received] = '\0';

	if (is_cgi) {
		std::vector<char> old_body = this->_cgi->get_body();
		old_body.insert(old_body.end(), buffer, &(buffer[bytes_received]));
		this->_cgi->set_body(old_body);
	} else
		this->_body.insert(this->_body.end(), buffer, &(buffer[bytes_received]));
}

int Client::get_fd(void) const
{
	return (this->_fd);
}

int Client::get_listener(void) const
{
	return (this->_listener);
}

ClientStatus Client::get_status(void) const
{
	return (this->_status);
}

std::vector<char> Client::get_header(void) const
{
	return (this->_header);
}

std::vector<char> Client::get_body(void) const
{
	return (this->_body);
}

void	Client::set_body(std::vector<char> body) {
	this->_body = body;
}

void Client::print_header()
{
	std::vector<char>::iterator it = this->_header.begin();
	for (; it != this->_header.end(); it++)
		std::cout << (*it);
}

void Client::print_body()
{
	std::vector<char>::iterator it = this->_body.begin();
	for (; it != this->_body.end(); it++)
		std::cout << (*it);
}

Response	Client::get_response(void) const {
	return (this->_response);
}

Request	Client::get_request(void) const {
	return (this->_request);
}

void		Client::set_response_payload(std::string payload) {
	this->_response.set_payload(payload);
}

void		Client::set_request_body(std::vector<char> body) {
	this->_request.set_packet_body(body);
}

void		Client::set_response_headers(std::string headers) {
	this->_response.set_headers(headers);
}

CGI	*Client::get_cgi(void) const {
	return (this->_cgi);
}

void Client::erase_body(std::vector<char>::size_type len)
{
	this->_body.erase(this->_body.begin(), this->_body.begin() + len);
}

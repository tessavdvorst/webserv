#include "CGI.hpp"

CGI::CGI() {
	// this->_header = new std::vector<char>();
	// this->_body = new std::vector<char>();
}

CGI::CGI(Request request, Response response):
	_request(request),
	_response(response)
{
	pipe(this->_pipe_out);
	// std::vector<std::string> allowed_methods = this->_request.get_directive("allowed_methods");
	// if (this->_request.get_method() == "POST" && std::find(allowed_methods.begin(), allowed_methods.end(), "POST") != allowed_methods.end())
	if (this->_response.get_request_type() == POST)
	{
		pipe(this->_pipe_in);
	}
//	run();
}

CGI::CGI(const CGI& that):
	_request(that._request),
	_response(that._response)
{
	*this = that;
}

CGI::~CGI() {
	// delete this->_header;
	// delete this->_body;
}

CGI& CGI::operator=(const CGI& that)
{
	this->_pid = that._pid;
	for (int i = 0; i < 2; i++)
	{
		this->_pipe_in[i] = that._pipe_in[i];
		this->_pipe_out[i] = that._pipe_out[i];
	}
	this->_args.erase(this->_args.begin(), this->_args.end());
	this->_args.insert(this->_args.end(), that._args.begin(), that._args.end());
	this->_env.erase(this->_env.begin(), this->_env.end());
	this->_env.insert(this->_env.end(), that._env.begin(), that._env.end());
//	delete this->_header;
	this->_header.erase(this->_header.begin(), this->_header.end());
	this->_header.insert(this->_header.end(), that._header.begin(), that._header.end());
//	delete this->_body;
	this->_body.erase(this->_body.begin(), this->_body.end());
	this->_body.insert(this->_body.end(), that._body.begin(), that._body.end());
	return (*this);
}

void CGI::run(void)
{
	if (this->_response.get_request_type() == POST)
	{
		std::stringstream temp;
		temp << "CONTENT_LENGTH=" << this->_request.get_packet_body().size();
		this->_content_length = temp.str();
	}
	this->_pid = fork();
	if (this->_pid < 0) // if forking fails
	{
		// error_msg("What the fork?!\n", clientSocketFd);
		/*if (this->_request.get_method() == "POST" && this->_request.get_directive("accept_post")[0] == "yes") {
			close(this->_pipe_in[0]);
			close(this->_pipe_in[1]);
		}
		close(this->_pipe_out[0]);
		close(this->_pipe_out[1]);*/
		throw std::runtime_error("Error forking...");
	}
	else if (this->_pid == 0)
	{ // Am kiddo
		if (this->_response.get_request_type() == POST)
		{
			dup2(this->_pipe_in[0], 0);
			close(this->_pipe_in[0]);
			close(this->_pipe_in[1]);
		}
		dup2(this->_pipe_out[1], 1);
		close(this->_pipe_out[1]);
		close(this->_pipe_out[0]);

		init_env_args();
	}
	close(this->_pipe_out[1]);
	if (this->_response.get_request_type() == POST)
		close(this->_pipe_in[0]);
}

void CGI::init_env_args(void)
{
	// SET ARGS
	set_args(this->_request.get_directive("cgi_bin")[0]);
	set_args(this->_request.get_index_url());
	this->_args.push_back(NULL);

	// SET ENV
	set_env("PATH_INFO=" + this->_request.get_url());
	set_env("REDIRECT_STATUS=200");

	if (this->_response.get_request_type() == POST)
		set_env(this->_content_length);
	set_env("SERVER_PROTOCOL=" + this->_request.get_protocol());
	set_env("REQUEST_METHOD=" + this->_request.get_method());
	set_env("CONTENT_TYPE=" + this->_request.get_content_type());
	set_env("SCRIPT_NAME=" + this->_request.get_decoded_url());
	set_env("SCRIPT_FILENAME=" + this->_request.get_index_url());
	if (this->_request.get_full_url() != this->_request.get_local_url())
		set_env("QUERY_STRING=" + (this->_request.get_full_url().substr(this->_request.get_full_url().find('?') + 1, this->_request.get_full_url().length())));
	set_env(std::string("PATH=") + getenv("PATH"));
	this->_env.push_back(NULL);

	//execute cgi
	if (execve(this->_request.get_directive("cgi_pass")[1].c_str(), this->_args.data(), this->_env.data()))
		throw std::runtime_error("Error exec'ing...");
}

void CGI::set_args(std::string directive)
{
	//what if a directive has multiple args?
	char *arg = new char[directive.length() + 1];
	memcpy(arg, directive.c_str(), directive.length() + 1);
	this->_args.push_back(arg);
}

void CGI::set_env(std::string directive)
{
	char *env = new char[directive.length() + 1];
	memcpy(env, directive.c_str(), directive.length() + 1);
	this->_env.push_back(env);
}

std::vector<char *> CGI::get_args(void)
{
	return (this->_args);
}

int		CGI::get_input_read_fd()
{
	return this->_pipe_in[0];
}

int		CGI::get_input_write_fd()
{
	return this->_pipe_in[1];
}

int		CGI::get_output_read_fd()
{
	return this->_pipe_out[0];
}

int		CGI::get_output_write_fd()
{
	return this->_pipe_out[1];
}

pid_t	CGI::get_pid()
{
	return this->_pid;
}

std::vector<char> CGI::get_header(void) {
	return (this->_header);
}

std::vector<char> CGI::get_body(void) {
	return (this->_body);
}

//check if copy goes well
void CGI::set_body(std::vector<char> body) {
	// this->_body = body;
	this->_body.erase(this->_body.begin(), this->_body.end());
	this->_body.insert(this->_body.end(), body.begin(), body.end());
}

void CGI::set_header(std::vector<char> header) {
	// this->_header = header;
	this->_header.erase(this->_header.begin(), this->_header.end());
	this->_header.insert(this->_header.end(), header.begin(), header.end());
}

void CGI::set_request(Request request) {
	this->_request = request;
}

void CGI::erase_body()
{
	std::vector<char> crlfcrlf;
	crlfcrlf.push_back('\r');
	crlfcrlf.push_back('\n');
	crlfcrlf.push_back('\r');
	crlfcrlf.push_back('\n');

//	std::cout << "[" << std::string(this->_body.data(), this->_body.size()) << "]" << std::endl;
//	std::cout << "_" << this->_body.size() << "_" << static_cast<size_t>(std::search(this->_body.begin(), this->_body.end(), crlfcrlf.begin(), crlfcrlf.end()) - this->_body.begin()) << "_" << std::endl;
	this->_body.erase(this->_body.begin(), std::search(this->_body.begin(), this->_body.end(), crlfcrlf.begin(), crlfcrlf.end()) + 4);
}

void CGI::erase_header()
{
	std::vector<char> crlfcrlf;
	crlfcrlf.push_back('\r');
	crlfcrlf.push_back('\n');
	crlfcrlf.push_back('\r');
	crlfcrlf.push_back('\n');

	this->_header.erase(std::search(this->_header.begin(), this->_header.end(), crlfcrlf.begin(), crlfcrlf.end()), this->_header.end());
}

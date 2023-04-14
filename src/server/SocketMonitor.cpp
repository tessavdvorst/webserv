#include "SocketMonitor.hpp"
#include "Logger.hpp"
#include "LoggerStream.hpp"
#include "Server.hpp"
#include "Request.hpp"

SocketMonitor::SocketMonitor(): _fdMax(0) {
	FD_ZERO(&this->_master);
	// FD_ZERO(&this->_read_fds);
	// FD_ZERO(&this->_write_fds);
	this->_zero_time.tv_sec = 0;
	this->_zero_time.tv_usec = 0;
}

SocketMonitor::SocketMonitor(const SocketMonitor& that)
{
	*this = that;
}

SocketMonitor::~SocketMonitor()
{
	std::map<int, Server*>::iterator server_it = this->_servers.begin();
	for (; server_it != this->_servers.end(); ++server_it)
		delete (*server_it).second;

	std::map<int, Client*>::iterator client_it = this->_clients.begin();
	for (; client_it != this->_clients.end(); ++client_it)
		delete client_it->second;
}

SocketMonitor& SocketMonitor::operator=(const SocketMonitor& that)
{
	this->_fdMax = that._fdMax;
	this->_master = that._master;
	this->_read_fds = that._read_fds;
	this->_write_fds = that._write_fds;
	this->_servers.erase(this->_servers.begin(), this->_servers.end());
	this->_servers.insert(that._servers.begin(), that._servers.end());
	this->_clients.erase(this->_clients.begin(), this->_clients.end());
	this->_clients.insert(that._clients.begin(), that._clients.end());
	return (*this);
}


void SocketMonitor::add_server(Server *server)
{
	int listener = server->get_socketfd();
	FD_SET(listener, &this->_master);
	if (listener > this->_fdMax)
		this->_fdMax = listener;
	this->_servers.insert(std::make_pair(listener, server));
}

void SocketMonitor::add_server_config(ServerBlock& server)
{
	std::map<int, Server*>::iterator it = this->_servers.begin();
	for (; it != this->_servers.end(); it++)
	{
		if ( (*it).second->get_port() == server.get_port())
		{
			ServerBlock* config = new ServerBlock();
			config = &server;
			(*it).second->get_server_blocks().push_back(config);
		}
	}
}

static void do_nothing(int signal) {
	(void)signal;
}

void SocketMonitor::run(void)
{
	signal(SIGPIPE, &do_nothing);
	while (1)
	{
		FD_ZERO(&this->_read_fds);
		FD_ZERO(&this->_write_fds);
		this->_read_fds = this->_master;
		this->_write_fds = this->_master;

		if ((select(this->_fdMax + 1, &this->_read_fds, &this->_write_fds, NULL, &this->_zero_time)) == -1)
			throw std::runtime_error("select call failed");

		std::map<int, Server*>::iterator server_it = this->_servers.begin();
		for (; server_it != this->_servers.end(); server_it++)
		{
			if (FD_ISSET(server_it->first, &this->_read_fds)) {
				accept_connection(server_it->first);
			}
		}

		try {
		// if (this->_clients.size()) {
			std::map<int, Client*>::iterator client_it = this->_clients.begin();
			for (; client_it != this->_clients.end(); client_it++)
			{
				// if ready to read -> recv
				if (FD_ISSET(client_it->first, &this->_read_fds) && (client_it->second->get_status() == READY_TO_READ || client_it->second->get_status() == READING_HEADER))
				{
//						std::cout << "A" << std::endl;
					client_it->second->read_header(client_it->first, false); //recv
				}
				// if status == reading (means that everything has been received)
				else if (client_it->second->get_status() == HEADER_READ)
				{
					//http parse
//						std::cout << "B" << std::endl;
					client_it->second->parse_request();
					//generate response
				}
				else if (client_it->second->get_status() == HEADER_PARSED)
				{
					//http parse
//						std::cout << "C" << std::endl;
					client_it->second->generate_response();
					//generate response
				}
				else if (client_it->second->get_status() == FORK_SET_PIPES)
				{
//						std::cout << "D1" << std::endl;
					add_fd(client_it->second->get_cgi()->get_output_read_fd());
//						std::cout << "D2" << std::endl;
					if (client_it->second->get_response().get_request_type() == POST) {
//							std::cout << "D3a" << std::endl;
						add_fd(client_it->second->get_cgi()->get_input_write_fd());
						client_it->second->update_status(READY_TO_READ_BODY);
//							std::cout << "D4a" << std::endl;
					} else {
//							std::cout << "D3b" << std::endl;
						client_it->second->update_status(RUN_CGI);
//							std::cout << "D4b" << std::endl;
					}
//						std::cout << "D5" << std::endl;
				}
				else if (FD_ISSET(client_it->first, &this->_read_fds) && (client_it->second->get_status() == READING_BODY || client_it->second->get_status() == READY_TO_READ_BODY))
				{
					//read body func?
//						std::cout << "E" << std::endl;
					client_it->second->update_status(READING_BODY);
					client_it->second->read_body(client_it->first, false);
					if (client_it->second->get_request().get_directive("client_max_body_size")[0].length() && client_it->second->get_body().size() > static_cast<unsigned long>(strtol(client_it->second->get_request().get_directive("client_max_body_size")[0].c_str(), NULL, 10))) {
						close_and_remove(client_it->second->get_cgi()->get_output_read_fd());
						client_it->second->set_response_headers("HTTP/1.1 413 Payload too large\r\ntransfer-encoding: chunked\r\n\r\n");
						client_it->second->set_response_payload("<!doctype html>\r\n<html>\r\n<head>\r\n<title>Error 413</title>\r\n</head><body><h1>413</h1>\r\n<br />\r\n<p>The request you made was too large for this server.</p>\r\n</body>\r\n</html>");
						client_it->second->update_status(READY_TO_SEND);
					}
					//opening file descriptors to master list
				}
				else if (client_it->second->get_status() == READING_BODY || (client_it->second->get_status() == READY_TO_READ_BODY && client_it->second->get_body().size()))
				{
//						std::cout << "F" << client_it->second->get_body().size() << std::endl;
					client_it->second->set_request_body(client_it->second->get_body());
					client_it->second->get_cgi()->set_request(client_it->second->get_request());
					client_it->second->update_status(RUN_CGI_POST);
				}
				else if (client_it->second->get_status() == RUN_CGI)
				{
//						std::cout << "F_" << std::endl;
					client_it->second->get_cgi()->run();
					client_it->second->update_status(READY_TO_READ_PIPE);
				}
				else if (client_it->second->get_status() == RUN_CGI_POST)
				{
//						std::cout << "F-" << std::endl;
					client_it->second->get_cgi()->run();
					client_it->second->update_status(BODY_TO_PIPE);
				}
				else if (client_it->second->get_status() == BODY_TO_PIPE
					&& FD_ISSET(client_it->second->get_cgi()->get_input_write_fd(), &this->_write_fds))
				{
//						std::cout << "G" << std::endl;
					if (client_it->second->get_body().size() > 8192) {
						::write(client_it->second->get_cgi()->get_input_write_fd(), client_it->second->get_body().data(), 8192);
						//::write(2, client_it->second->get_body()->data(), 4096);
						client_it->second->erase_body(8192);
					} else {
						::write(client_it->second->get_cgi()->get_input_write_fd(), client_it->second->get_body().data(), client_it->second->get_body().size());
//							::write(2, client_it->second->get_body().data(), client_it->second->get_body().size());
						client_it->second->update_status(BODY_DONE);
					}
				}
				else if (client_it->second->get_status() == BODY_DONE)
				{
					close_and_remove(client_it->second->get_cgi()->get_input_write_fd());
					client_it->second->update_status(READY_TO_READ_PIPE);
				}
				else if ((client_it->second->get_status() == READY_TO_READ_PIPE
					|| client_it->second->get_status() == CGI_HEADER)
					&& FD_ISSET(client_it->second->get_cgi()->get_output_read_fd(), &this->_read_fds))
				{
//						std::cout << "H" << std::endl;
					// read_cgi_header(client_it->second);
					client_it->second->read_header(client_it->second->get_cgi()->get_output_read_fd(), true);
//					std::cout << "Ready for cgi..." << std::endl;
				}
				else if (client_it->second->get_status() == CGI_HEADER
					&& !FD_ISSET(client_it->second->get_cgi()->get_output_read_fd(), &this->_read_fds))
				{
//						std::cout << "I" << std::endl;
					client_it->second->update_status(CGI_TO_PAYLOAD);
//					std::cout << "Ready for cgi..." << std::endl;
				}
				else if (client_it->second->get_status() == CGI_BODY
					&& FD_ISSET(client_it->second->get_cgi()->get_output_read_fd(), &this->_read_fds))
				{
//						std::cout << "J" << std::endl;
					client_it->second->update_status(CGI_BODY);
					client_it->second->read_body(client_it->second->get_cgi()->get_output_read_fd(), true);
					// read_cgi_body(client_it->second);
//					std::cout << "Ready for cgi..." << std::endl;
				}
				else if (client_it->second->get_status() == CGI_BODY)
				{
//						std::cout << "K" << std::endl;
					client_it->second->update_status(CGI_TO_PAYLOAD);
//					std::cout << "Ready for cgi..." << std::endl;
				}
				else if (client_it->second->get_status() == CGI_TO_PAYLOAD)
				{
//						std::cout << "L" << std::endl;
					fix_payload(client_it->second);
				}
				else if (client_it->second->get_status() == READY_TO_SEND
					&& FD_ISSET(client_it->first, &this->_write_fds))
				{
					//send
//						std::cout << "M" << std::endl;
					response_handler(client_it->second);
					// std::cout << "closing client connection..." << std::endl;
					// close(//close connectionclient_it->second->get_fd());
				}
				else if (client_it->second->get_status() == SENDING_BODY
					&& FD_ISSET(client_it->first, &this->_write_fds))
				{
//						std::cout << "N" << std::endl;
					handle_body(client_it->second);
					// std::cout << "closing client connection..." << std::endl;
					// close(client_it->second->get_fd());
					//close connection
				}
				else if (client_it->second->get_status() == ALMOST_DONE
					&& FD_ISSET(client_it->first, &this->_write_fds))
				{
					//send
//						std::cout << "O" << std::endl;
					final_packet(client_it->second);
					// std::cout << "closing client connection..." << std::endl;
					// close(client_it->second->get_fd());
					//close connection
				}
				else if (client_it->second->get_status() == FINISHED)
				{
//						std::cout << "P" << client_it->first << std::endl;
					close_and_remove(client_it->first);
					delete client_it->second;
					this->_clients.erase(client_it);
//						std::cout << "Z" << client_it->first << std::endl;
					break ;
				}
			}
		} catch (CloseFdException &e) {
			close_and_remove(e.get_fd());
		}
	}
}

void	SocketMonitor::fix_payload(Client *client) {
	close_and_remove(client->get_cgi()->get_output_read_fd());
	::waitpid(client->get_cgi()->get_pid(), NULL, 0);

	std::stringstream out;
	std::string header_str(client->get_cgi()->get_header().data(), client->get_cgi()->get_header().size());
	out << "HTTP/1.1 200 OK\r\ntransfer-encoding: chunked\r\n" << header_str;;
	if (out.str().find("Status: ") != std::string::npos) {
		std::string correct_status = "HTTP/1.1 " + out.str().substr(out.str().find("Status: ") + std::string("Status: ").length(), out.str().find("\r\n", out.str().find("Status: ")) - (out.str().find("Status: ") + std::string("Status: ").length())) + "\r\n";
		out.str("");
		out << correct_status;
		std::copy(client->get_cgi()->get_header().begin(), client->get_cgi()->get_header().end(), std::ostream_iterator<char>(out));
		std::string fixed = out.str().substr(0, out.str().find("Status: ")) + out.str().substr(out.str().find("\r\n", out.str().find("Status: ")) + 2);
		out.str("");
		out << fixed;
	}
	out << "\r\n\r\n";
	client->set_response_headers(out.str());
	client->set_response_payload(std::string(client->get_cgi()->get_body().data(), client->get_cgi()->get_body().size()));
	client->update_status(READY_TO_SEND);
}

void SocketMonitor::accept_connection(int fd) {
	Client *client = NULL;
	try {
		client = new Client(this->_servers.at(fd)->accept(), fd, this->_servers.at(fd));
		this->_clients[client->get_fd()] = client;
		add_fd(client->get_fd());
	} catch (const std::exception &e) { //make custom exception
		logger.log(Logger::ERROR) << e.what() << '\n';
		close_and_remove(client->get_fd()); ///???
		delete client;
	}
}

void SocketMonitor::response_handler(Client* client)
{
	std::stringstream ss;
	ss << client->get_response().get_headers();
	if (client->get_response().get_payload().length())
		ss << std::hex << client->get_response().get_payload().substr(0, 4096).length() << "\r\n" << client->get_response().get_payload().substr(0, 4096) << "\r\n";
//	client->set_response_payload(ss.str());

	if ((send(client->get_fd(), ss.str().c_str(), ss.str().length(), 0)) == -1)
		std::cerr << "Error sending...\n" << std::endl;

	if (client->get_response().get_payload().length()) {
		client->set_response_payload(client->get_response().get_payload().substr(client->get_response().get_payload().substr(0, 4096).length()));
		client->update_status(SENDING_BODY);
	} else {
		client->update_status(ALMOST_DONE);
	}
}

void SocketMonitor::final_packet(Client* client)
{
	std::stringstream ss;
	ss << "0\r\n\r\n";
//	client->set_response_payload(ss.str());

	if ((send(client->get_fd(), ss.str().c_str(), ss.str().length(), 0)) == -1)
		std::cerr << "Error sending...\n";

	client->update_status(FINISHED);
}

void SocketMonitor::handle_body(Client* client)
{
	std::stringstream ss;
	ss << std::hex << client->get_response().get_payload().substr(0, 8182).length() << "\r\n" << client->get_response().get_payload().substr(0, 8182) << "\r\n";
//	client->set_response_payload(ss.str());

	if ((send(client->get_fd(), ss.str().c_str(), ss.str().length(), 0)) == -1)
	{
		client->update_status(FINISHED);
		return ;
	}

	if (client->get_response().get_payload().length()) {
		client->set_response_payload(client->get_response().get_payload().substr(client->get_response().get_payload().substr(0, 8182).length()));
	} else {
		client->update_status(FINISHED);
	}
}

void SocketMonitor::add_fd(int fd)
{
	FD_SET(fd, &this->_master); // add to master set
	if (fd > this->_fdMax)
		this->_fdMax = fd;
}

void SocketMonitor::remove_fd(int fd)
{
	FD_CLR(fd, &this->_master);
	if (fd == this->_fdMax)
		this->_fdMax--;
}

void SocketMonitor::close_and_remove(int fd)
{
	close(fd);
	remove_fd(fd);
}

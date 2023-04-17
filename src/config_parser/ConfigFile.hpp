#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

# include <fstream>
# include "utils.hpp"
# include "Block.hpp"
# include "ServerBlock.hpp"
# include <climits>
# include <cstdlib>

enum BodySizeType
{
	KILO,
	MEGA,
	GIGA,
	BYTES,
	UNKNOWN
};

class ConfigFile {
	private:
		std::string					_fileName;
		std::vector<std::string>	_content;
		std::vector<ServerBlock>	_serverBlock;

		ConfigFile();

	public:
		ConfigFile(const std::string& fileName);
		~ConfigFile();

		enum ErrCode {
			INVALID_SYNTAX,
			PORT_NOT_DEFINED,
			SERVER_NAME_NOT_DEFINED,
			DUPLICATE_DIRECTIVE,
			UNKNOWN_DIRECTIVE,
			INVALID_VALUES,
			INVALID_PORT_VALUE,
			INVALID_SERVER_NAME
		};

		class InvalidFileException: public std::exception {
			public:
				const char* what() const throw() {return "could not open config file";};
		};


		class InvalidSyntaxException: public std::exception {
			private:
				int _index;
				ErrCode _err;
				
			public:
				InvalidSyntaxException(int index, ErrCode err): _index(index), _err(err) {}

				ErrCode getErrorCode() const { return (this->_err); }
				int getIndex() const { return (this->_index); }
		};

		void						load_file(void);
		void						save_configurations(void);
		bool						is_valid_location(const std::string& context);
		bool						is_correct_end(const std::string& line, int index);
		Block::ConfigType			check_syntax_line(const std::string& line, int index);
		void						check_syntax_server_block(void);
		int							check_syntax_location_block(std::vector<std::string> content, int i);
		BodySizeType				get_body_size_type(std::string& str_size);
		bool						convert_client_max_body(std::string& line);
		static void					handle_syntax_exception(int index, ErrCode err);
		std::vector<std::string>	get_content(void);
		std::vector<ServerBlock>	get_server_blocks(void);

};

std::ostream& operator<<(std::ostream& output, ConfigFile& config);

#endif

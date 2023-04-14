NAME		= 		webserv
CC			= 		c++ -std=c++98

VPATH		=		src:\
					src/utils: \
					src/config_parser: \
					src/server

SRC_FILES	=		main.cpp\
					$(SERVER_FILES)\
					$(UTILS_FILES)\
					$(C_PARSER_FILES)

SERVER_FILES =		Server.cpp \
					Client.cpp \
					SocketMonitor.cpp \
					Request.cpp \
					Response.cpp \
					CGI.cpp

UTILS_FILES =		Logger.cpp \
					LoggerStream.cpp \

C_PARSER_FILES	=	ConfigFile.cpp \
					Block.cpp \
					ServerBlock.cpp \
					LocationBlock.cpp \
					utils.cpp

OBJS_DIR 	= 		objs
OBJS		= 		$(addprefix $(OBJS_DIR)/,$(SRC_FILES:.cpp=.o))
DEPS		= 		$(OBJS:.o=.d)
HEADER_FILES =		-Isrc/config_parser -Isrc/utils -Isrc/server

ifdef DEBUG
 CFLAGS = -Wextra -Wall -Werror -fsanitize=address -g
else
 CFLAGS = -Wextra -Wall -Werror -O3
endif

all:				$(NAME)

$(NAME):			$(OBJS)
					@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
					@echo "\033[0;32mCompilation completed!\033[0m"

debug:
					$(MAKE) DEBUG=1 all

$(OBJS_DIR)/%.o:	%.cpp | $(OBJS_DIR)
					@$(CC) $(CFLAGS) $(HEADER_FILES) -MMD -o $@ -c $<

$(OBJS_DIR):
					@mkdir -p $@
	
clean:
					@rm -rf $(OBJS_DIR)
					@echo "\033[0;31mCleaned!\033[0m"

fclean:				clean
					@rm -f $(NAME)

re:					clean fclean all

.PHONY:				all clean fclean re

-include $(DEPS)

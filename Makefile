CXX			?=	/usr/bin/c++
MAKE		?=	/usr/bin/make
SRC_DIR		=	./src
CFLAGS		+=	-Wall -Wextra -Werror -std=c++98 -pedantic -Iinclude
OBJ			=	gopher++.o
NAME		=	gopher++

all			:	$(NAME)

$(NAME)		:	$(OBJ)
				$(CXX) $(CFLAGS) -o $(NAME) $(OBJ)

clean		:
				$(RM) -f $(OBJ)

fclean		:	clean
				$(RM) -f $(NAME)

re			:	fclean all

%.o			:	$(SRC_DIR)/%.cpp
				$(CXX) $(CFLAGS) -c $<

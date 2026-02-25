NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = \
	src/main.cpp \
	src/Server.cpp \
	src/ConfigParser.cpp \
	src/HttpParser.cpp \
	src/Router.cpp \
	src/CGIHandler.cpp

OBJ = $(SRC:.cpp=.o)

INCLUDES = -Iincludes

all: $(NAME)

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
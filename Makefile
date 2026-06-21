CXX = c++
CPPFLAGS = -I includes  -Wall -Wextra -g -std=c++98 #-Werror
NAME = webserv
SRC := $(wildcard src/*/*.cpp)
BINDIR = bin
OBJ := $(patsubst %.cpp, $(BINDIR)/%.o, $(notdir $(SRC)))
DEP = $(wildcard includes/*.h)
vpath %.cpp src/config src/server src/utils src/httpMessage src/main

all : $(NAME)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/%.o: %.cpp $(DEP) | $(BINDIR)
	$(CXX) $(CPPFLAGS) -c $< -o $@

$(NAME): $(OBJ) | $(BINDIR)
	$(CXX) $^ -o $(NAME)

re: fclean $(NAME)

clean:
	rm -rf $(BINDIR)
fclean: clean
	rm -f $(NAME)
.PHONY:
	all re fclean clean

.SECONDARY:
	$(OBJ)
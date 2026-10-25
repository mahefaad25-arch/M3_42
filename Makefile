NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

SRC_DIR		= src
OBJ_DIR		= obj

SRCS		= main.c \
			  ft_atoi.c \
			  parsing.c \
			  parsing2.c \
			  utils.c \
			  dongle.c \
			  dongle_waiters.c \
			  coder.c \
			  monitor.c \
			  init.c

OBJS		= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))
HEADER		= $(SRC_DIR)/codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

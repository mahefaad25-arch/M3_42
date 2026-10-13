# ************************************************************************** #
#                                                                            #
#                                                        :::      ::::::::  #
#   Makefile                                          :+:      :+:    :+:  #
#                                                    +:+ +:+         +:+    #
#   By: student <student@student.42.fr>            +#+  +:+       +#+      #
#                                                +#+#+#+#+#+   +#+         #
#   Created: 2026/08/31 00:00:00 by student           #+#    #+#          #
#   Updated: 2026/08/31 00:00:00 by student          ###   ########.fr    #
#                                                                            #
# ************************************************************************** #

NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

SRC_DIR		= .
OBJ_DIR		= obj

SRCS		= main.c \
			  parsing.c \
			  utils.c \
			  dongle.c \
			  dongle_waiters.c \
			  coder.c \
			  monitor.c \
			  init.c

OBJS		= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c codexion.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

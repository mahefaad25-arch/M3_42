NAME = codexion

all: $(NAME)

$(NAME):
	$(MAKE) -C $(NAME)

clean:
	$(MAKE) -C $(NAME) clean

fclean:
	$(MAKE) -C $(NAME) fclean

re:
	$(MAKE) -C $(NAME) re

.PHONY: all $(NAME) clean fclean re

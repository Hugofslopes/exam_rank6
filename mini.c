//Copy from main

#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int count  = 0, max_fd = 0;
int ids[1024];
char *msgs[1024];

fd_set fdread,fdwrite,fdactive;
char bufer_r[1001], but_w[42];

//Copy from main
int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}
//end

void print_exit()
{
	write(2, "Fatal error\n", 12);
	exit(1);
}

void send_to_clients(int author, char *msg)
{
	for (int fd = 0; fd < max_fd ; fd++)
	{
		if (FD_ISSET(fd, &fdwrite) && fd != author)
			send(fd, msg, strlen(msg), 0);
	}
}

void add_client(int fd)
{
	max_fd = fd > max_fd ? fd : max_fd;
	ids[fd] = count++;
	msgs[fd] = NULL;
	FD_SET(fd, &fdactive);
	sprintf(bufer_r, "server: client %d just arrived\n", ids[fd]);
	send_to_clients(fd, bufer_r);
}

void remove_client(int fd)
{
	sprintf(bufer_r, "server: client %d just left\n", ids[fd]);
	send_to_clients(fd, bufer_r);
	free(msgs[fd]);
	FD_CLR(fd, &fdactive);
	close(fd);
}

void send_msg(int fd)
{
	char* msg;

	while (extract(&msgs[fd], &msg))
	{
		sprintf(bufer_r, "client %d: ", ids[fd]);
		send_to_clients(fd, bufer_r);
		send_to_clients(fd, msg);
	}
}

int creat_sockect()
{
	//ipv4, tcp
	max_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (max_fd < 0)
		print_exit();
	FD_SET(max_fd, &fdactive);
	return max_fd;
}

int		main(int ac, char **av)
{
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	FD_ZERO(&fdactive);
	int socket = creat_sockect();

	//copy and remove ther client
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(8081);
	if (bind(socket, (const struct sockaddr *)&servaddr, sizeof(servaddr)));
		print_exit();
	if (listen(socket, 4096))
		print_exit();

	// END COPY-PASTE

	while (1)
	{
		fdread = fdwrite = fdactive ;

		if (select(max_fd + 1, &fdread, &fdwrite, NULL, NULL) < 0)
			print_exit();

		for (int fd = 0; fd <= max_fd; fd++)
		{	
			if (!FD_ISSET(fd, &fdread))
				continue;
			if (fd == socket)
			{
				socklen_t addr_len = sizeeof(servaddr);
				int client_fd = accept(socket,(const struct sockaddr *)&servaddr,&addr_len);
				if (client_fd >= 0)
				{
					add_client(fd);
					break ;
				}
			}
			else{
				int bytes_r = recv(fd, bufer_r, 1000, 0);
				if (bytes_r <= 0)
				{
					remove_client(fd);
					break;
				}
				bufer_r[bytes_r] = '\0';
				msgs[fd] = str_join(msgs[fd], bufer_r);
				send_msg(fd);
			}
		}
	}
	return 0;	
}
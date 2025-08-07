#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

int ids[35563];
char *msgs[35563];
int count = 0, max_fd = 0;
fd_set rfd,wfd, afd;
char w_bf[42], r_bf[1024];

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

void fatal()
{
    write(2,"Fatal error\n",12);
}

void broadcast(char *msg, int fd)
{
    for (int i = 0; i <= max_fd; i++)
    {
        if (FD_ISSET(i, &wfd) && i != fd)
            send(i,msg, strlen(msg), 0);
    }
}

void remove_client(int fd)
{
    FD_CLR(fd, &afd);
    close(fd);
    sprintf(w_bf,"server: client %d just left\n",ids[fd]);
    broadcast(w_bf,fd);
    if (msgs[fd])
        free(msgs[fd]);
}

void close_exit()
{
    for (int i = 0; i <= max_fd; i++)
        if (FD_ISSET(i,&afd))
            remove_client(i);
}

void add_c(int n_fd)
{
    max_fd = max_fd > n_fd ? max_fd : n_fd;
    ids[n_fd] = count++;
    msgs[n_fd] = NULL;
    FD_SET(n_fd, &afd);
    sprintf(w_bf,"server: client %d just arrived\n",ids[n_fd]);
    broadcast(w_bf,n_fd);

}

void send_msg(int i)
{
    char *msg;

    while(extract_message(&msgs[i], &msg))
    {
        sprintf(w_bf,"client %d: ",ids[i]);
        broadcast(w_bf,i);
        broadcast(msg,i);
        free(msg);
    }
}

int main(int ac, char **av) {
    if (ac != 2)
    {
        write(2,"Wrong number of arguments\n", 26);
        return 1;
    }

	int sockfd;
	struct sockaddr_in servaddr; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		fatal();
		return 1; 
	} 
	FD_SET(sockfd,&afd);
    max_fd = sockfd;
	bzero(&servaddr, sizeof(servaddr)); 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1])); 
  
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		fatal();
		return 1; 
	} 
	if (listen(sockfd, 10) != 0) {
		fatal();
		return 1; 
	}

    while (1)
    {
        rfd = wfd = afd;

        if (select(max_fd + 1, &rfd, &wfd, NULL, NULL) < 0)
        {
            fatal();
            close_exit();
        }

        for (int i = 0; i <= max_fd ; i++)
        {
            if (!FD_ISSET(i,&rfd))
                continue;
            if (i == sockfd)
            {
                socklen_t size = sizeof(servaddr);
                int n_fd = accept(sockfd, (struct sockaddr *)&servaddr,&size);
                if (n_fd >= 0)
                    add_c(n_fd);
            }
            else
            {
                int b_rcv = recv(i, r_bf, 1023, 0);
                if (b_rcv <= 0)
                {
                    remove_client(i);
                    continue;
                }
                r_bf[b_rcv] = '\0';
                msgs[i] = str_join(msgs[i],r_bf);
                send_msg(i);
            }
        }

    }
}
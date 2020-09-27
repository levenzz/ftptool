static inline int ftp_socket_send(int fd, char *str)
{
    send(fd, str, strlen(str), 0);
    return 0;
}

static inline int ftp_socket_recv(int fd, char *str)
{
    int size;
    size = recv(fd, str, MAX_LEN-1, 0);
    str[size] = 0;

    //printf("ftp recv: %s\n",str);
    return 0;
}

static int ftp_get_data_port(char *buff, in_port_t *port)
{
    int i = 0, j = 0;
    short port_l = 0, port_h = 0;

    if (buff == NULL || port == NULL)
    {
        return -1;
    }
    // (192,168,186,1,4,0).
    while (buff[i++] != '(');
    while (j < 4)
    {
        if(buff[i++] == ',')
            j++;
    }

    while (buff[i] != ',')
    {
        port_h *= 10;
        port_h += buff[i] - 0x30;
        i++;
    }

    i++;
    while (buff[i] != ')')
    {
        port_l *= 10;
        port_l += buff[i] - 0x30;
        i++;
    }

    printf("data_port : %u\n", port_h << 8 | port_l);
    *port = htons((short)(port_h << 8 | port_l));

    return 0;
}
static int ftp_get_upload_file_name(const char *upload_file, char *file_name)
{
    int i = 0;
    int path_lenth = 0;

    if (upload_file == NULL || file_name == NULL)
    {
        return -1;
    }

    path_lenth = strlen(upload_file);

    while (path_lenth - i > 0)
    {
        // find index of '/'
        if (upload_file[path_lenth - i]== 47)
        {
            i--;
            break;
        }
        i++;
    }

    strcpy(file_name, &upload_file[path_lenth - i]);


    return 0;
}

int ftp_upload_file(const char *ip, unsigned int port, const char *user, const char *pwd,
const char *upload_file,const char *rempte_upload_name, const char *remote_folder_name)
{
    int ret;
    int size;
    char buff[MAX_LEN];
    char cmd[MAX_CMD_LEN];
    char file_name[256];
    int fd_socket, fd_data;
    struct sockaddr_in addr;
    struct sockaddr_in data;
    int send_ret=0;

    addr.sin_family = AF_INET;
    inet_aton(ip, &addr.sin_addr);
    addr.sin_port   = htons(port);
    data.sin_family = AF_INET;
    inet_aton(ip, &data.sin_addr);
    //用于建立和ftp服务的连接，通过21端口
    fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_socket == -1) {
        return -1;
    }
    fd_data = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_data == -1) {
        close(fd_socket);
        return -1;
    }
    ret = connect(fd_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret != 0) {
        close(fd_data);
        close(fd_socket);
        printf("connet falied\n");
        return -1;
    }

    size = recv(fd_socket, buff, MAX_LEN-1, 0);
    buff[size] = 0;
    //向ftp服务器发送登录使用的用户名
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "USER %s\r\n", user);
    // ftp_socket_send(fd_socket, "PASS shikejun\r\n");
    ftp_socket_send(fd_socket, cmd);
    ftp_socket_recv(fd_socket, buff);
    //向ftp服务器发送登录使用的密码
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "PASS %s\r\n", pwd);
    ftp_socket_send(fd_socket, cmd);
    ftp_socket_recv(fd_socket, buff);
    //返回服务器使用的操作系统
    ftp_socket_send(fd_socket, "SYST\r\n");
    ftp_socket_recv(fd_socket, buff);
    //指定数据类型，A=ASCII， E=EBCDIC， I=binary
    ftp_socket_send(fd_socket, "TYPE I\r\n");
    ftp_socket_recv(fd_socket, buff);
    //用被动方式向ftp请求连接，ftp返回一个端口，用于上传文件
    ftp_socket_send(fd_socket, "PASV\r\n");
    ftp_socket_recv(fd_socket, buff);

    ftp_get_data_port(buff, &data.sin_port);

    memset(file_name, 0, sizeof(file_name));
    if(upload_name==NULL||strlen(upload_name)<=1) {
        ftp_get_upload_file_name(upload_file, file_name);
    }
    else {
        strcpy(file_name,upload_name);
    }

    //进入ftp中的folder_name
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "CWD %s\r\n", folder_name);
    ftp_socket_send(fd_socket, cmd);
    ftp_socket_recv(fd_socket, buff);
	
    char *cmp_res = "550 Failed to change directory.";
    char buff_res[31];
    for (int i=0; i<31; i++) {
        buff_res[i] = buff[i];
    }
    //如果ftp中目前没有folder_name，则会返回一个550 Failed to change directory.
    if (!strcmp(buff_res, cmp_res)) {
        //如果不存在folder_name，则首先进行创建，然后在改变在ftp中工作目录为folder_name
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "MKD %s\r\n", folder_name);
        ftp_socket_send(fd_socket, cmd);
        ftp_socket_recv(fd_socket, buff);

        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "CWD %s\r\n", folder_name);
        ftp_socket_send(fd_socket, cmd);
        ftp_socket_recv(fd_socket, buff);
    }
    //在ftp中建立文件，名为file_name
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "STOR %s\r\n", file_name);
    ftp_socket_send(fd_socket, cmd);
    //通过fd_data进行文件上传
    ret = connect(fd_data, (struct sockaddr *)&data, sizeof(data));
    if (ret != 0) {
        printf("connet falied\n");
        close(fd_data);
        close(fd_socket);
        return -1;
    }

    ftp_socket_recv(fd_socket, buff);
    //打开本地文件
    int fd = open(upload_file, O_RDONLY);
    if (fd == -1) {
        printf("open: \n");
        close(fd_data);
        close(fd_socket);
        return -1;
    }
    //进行流的操作，read读取本地文件数据后再send到ftp中
    while ((ret = read(fd, buff, sizeof(buff))) > 0) {
        send_ret = send(fd_data, buff, ret, 0);
        if(send_ret<=0) {
            break;
        }
        //usleep(30*1000);
    }
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "QUIT\r\n");
    ftp_socket_send(fd_socket, cmd);
    close(fd);
    close(fd_data);

    ftp_socket_recv(fd_socket, buff);

    close(fd_socket);

    printf("ftp_upload [%s] [%s] result: %s\n",upload_file,upload_name,buff);

    return 0;
}
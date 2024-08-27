#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <poll.h>


#define DIR_LEN 128
#define LINE_LEN 2048
#define LINE2_LEN 128
#define WORKSPACE_COUNT 9
#define YUCK_BUTTON_LEN 128

typedef struct 
{
  int active_workspace;
  int *workspaces_status;
}workspace_info_t;

volatile sig_atomic_t action = 0;

int change_workspace(int);
void signal_handler(int);
workspace_info_t *get_active_workspace(char*);
void print_boxes(const workspace_info_t*);
int workspace_change(char*);

workspace_info_t* wd = NULL;
workspace_info_t* w = NULL;

char socket_dir[DIR_LEN] = { 0 };
char socket2_dir[DIR_LEN] = { 0 };


/*
 * Change to given workspace
 */
int
change_workspace(int workspace)
{

  char* xdgrdir = getenv("XDG_RUNTIME_DIR");
  char* hyprinstsig = getenv("HYPRLAND_INSTANCE_SIGNATURE");
  snprintf(socket_dir,DIR_LEN,"%s/hypr/%s/.socket.sock",
           xdgrdir,hyprinstsig);

  int sockfd, rc = 0;
  struct sockaddr_un address;

  sockfd = socket(AF_LOCAL,SOCK_STREAM,0);
  
  if (sockfd == -1)
  {
    perror("Socket error");
    return 2;
  }

  memset(&address,0,sizeof(address));
  address.sun_family = AF_UNIX;
  strncpy(address.sun_path, socket_dir, sizeof(address.sun_path)-1);
  
  if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) == -1)
  {
    perror("connect error");
    return 2;
  }
  
  const char ws[] = "dispatch workspace";
  char msg[32] = { 0 };
  snprintf(msg,strlen(ws)+3,"%s %d",ws,workspace);
  write(sockfd, msg, strlen(msg)+1);
  char* buffer = calloc(LINE_LEN,sizeof(char));
  rc = read(sockfd,buffer,LINE_LEN);


  if(rc == -1)
  {
    printf("Error while trying to read socket");
    perror("read");
    return 2;
  }
  close(sockfd);

  return 0;
}

/*
 * check socket2 message if there is a change on workspaces
 *
 * message will read out "workspace>>" if there is
*/
int
workspace_change(char* line)
{
  const char identifier[] = "workspace>>";
  if (strncmp(identifier,line,strlen(identifier)))
    return 0;
  else
    return 1;
}
void
signal_handler(int signum)
{
  if(signum == SIGINT)
  {
    exit(0);
  }
  else if(signum == SIGUSR1)
  {
    action = 1;
  }
}

/*
 * print formatted yuck boxes
 */
void
print_boxes(const workspace_info_t* workspace_info)
{
  char** box_buttons = calloc(WORKSPACE_COUNT,sizeof(char*));

  for(int i = 0; i < WORKSPACE_COUNT; ++i)
  {
    
      box_buttons[i] = calloc(YUCK_BUTTON_LEN,sizeof(char));
      if (workspace_info->workspaces_status[i])
        snprintf(box_buttons[i],YUCK_BUTTON_LEN,
                 "(button :class \"occupied\" :onclick \"/home/dylandy/.config/eww/C_scripts/workspaces %d\" \"%d\")"
                 ,i+1,i+1);
      else 
        snprintf(box_buttons[i],YUCK_BUTTON_LEN,
                 "(button :class \"empty\" :onclick \"/home/dylandy/.config/eww/C_scripts/workspaces %d\" \"%d\")"
                 ,i+1,i+1);
  }

  
  free(box_buttons[workspace_info->active_workspace]);
  box_buttons[workspace_info->active_workspace] = calloc(YUCK_BUTTON_LEN,1);
  snprintf(box_buttons[workspace_info->active_workspace],YUCK_BUTTON_LEN,
           "(button :class \"active\" \"%d\")",
           workspace_info->active_workspace+1);
           
  
  printf("(box :class \"workspace-entry\"");
  for (int i = 0; i < WORKSPACE_COUNT; ++i)
  {
    printf("%s",box_buttons[i]);
    free(box_buttons[i]);
  }
  puts(")");
  fflush(stdout);
  
  free(box_buttons);
}

/*
 * extract current active workspace and occupied
 * workspaces
 */
workspace_info_t*
get_active_workspace(char* ws_info)
{
  workspace_info_t *workspace_info = malloc(sizeof(workspace_info_t));
  workspace_info->workspaces_status = calloc(WORKSPACE_COUNT+1,sizeof(int));
  int active_obtained_flag = 0;
  char* info = strsep(&ws_info," ");

  do 
  {
    if ((strncmp(info,"ID",2) == 0) && (!active_obtained_flag))
    {
      active_obtained_flag = 1;
      workspace_info->active_workspace = atoi(strsep(&ws_info," ")) - 1;
    }
    else if ((strncmp(info,"ID",2) == 0) && (active_obtained_flag))
    {
      int occupied_workspace = atoi(strsep(&ws_info," ")) - 1; 
      workspace_info->workspaces_status[occupied_workspace] = 1;
    }
    info = strsep(&ws_info," ");
  } while(info);
  
  return workspace_info;
}

int
main(int argc, char** argv)
{
  // TODO: Implement buttons to change workspace
  
  
  if(argc > 1) // if clicked change workspace to clicked button
  {
    //check if argument is 1-9
    if((int)argv[1][0] > 57 || (int)argv[1][0] < 48)
    {
      fprintf(stdout,"Usage: workspace [WORKSPACE NUMBER]\n Enter a number\n");
        return 1;
    }

    int workspace = atoi(argv[1]);
    change_workspace(workspace);

    return 0;
  }

  // signal handler
  signal(SIGINT,signal_handler);

  // 

  // getting socket directory
  w = NULL;

  char* xdgrdir = getenv("XDG_RUNTIME_DIR");
  char* hyprinstsig = getenv("HYPRLAND_INSTANCE_SIGNATURE");
  snprintf(socket_dir,DIR_LEN,"%s/hypr/%s/.socket.sock",
           xdgrdir,hyprinstsig);
  snprintf(socket2_dir,DIR_LEN,"%s/hypr/%s/.socket2.sock",
           xdgrdir,hyprinstsig);

  int sockfd, rc = 0;
  struct sockaddr_un address;

  pid_t pid = fork();

  if (pid == 0)
  {
    sockfd = socket(AF_LOCAL,SOCK_STREAM,0);
    
    if (sockfd == -1)
    {
      perror("Socket error");
      return 2;
    }

    memset(&address,0,sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, socket2_dir, sizeof(address.sun_path)-1);
    
    if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
      perror("connect error");
      return 2;
    }

    struct pollfd fds[1];

    fds[0].fd = sockfd;
    fds[0].events = POLL_IN;

    char* buffer = calloc(LINE2_LEN,sizeof(char));
    while(1)
    {
      int poll_result = poll(fds,1,5000);
      if(poll_result > 0)
      {
        if (fds[0].revents & POLL_IN)
        {
          rc = read(sockfd,buffer,LINE2_LEN);
          if(strlen(buffer))
            if(workspace_change(buffer) == 0)
              kill(getppid(),SIGUSR1);
        }
      }
    }

      free(buffer);

    if (rc == -1)
    {
      puts("Error while trying to read socket");
      perror("read");
      return -1;
    }

    close(sockfd);

  } else
  { // parent process, responsible for producing yuck code
    signal(SIGUSR1,signal_handler);
    while(1)
    {
      pause();
      if(action)
      {
        sockfd = socket(AF_LOCAL,SOCK_STREAM,0);
        
        if (sockfd == -1)
        {
          perror("Socket error");
          return 2;
        }

        memset(&address,0,sizeof(address));
        address.sun_family = AF_UNIX;
        strncpy(address.sun_path, socket_dir, sizeof(address.sun_path)-1);
        
        if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) == -1)
        {
          perror("connect error");
          return 2;
        }

        
        const char msg[] = "[[BATCH]]/activeworkspace;workspaces";
        write(sockfd, msg, strlen(msg)+1);
        char* buffer = calloc(LINE_LEN,sizeof(char));
        rc = read(sockfd,buffer,LINE_LEN);

        if(rc == -1)
        {
          printf("Error while trying to read socket");
          perror("read");
          return 2;
        }

        wd = get_active_workspace(buffer);

        free(buffer);

        if(!w)
        {
          goto print;
        }

        if((memcmp(wd->workspaces_status,w->workspaces_status,
                   WORKSPACE_COUNT*sizeof(int)) != 0) 
                   || (wd->active_workspace != w->active_workspace))
        {
          free(w->workspaces_status);
          free(w);
        print:
          w = wd;
          print_boxes(w);
        }
        else if(wd)
        {
          free(wd->workspaces_status);
          free(wd);
        }

        close(sockfd);
        action = 0;
      }
    }
  }
  return 0;
}

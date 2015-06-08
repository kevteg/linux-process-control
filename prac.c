#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>

#define COLOR_ROJO            "\x1b[31m"
#define COLOR_NORMAL          "\x1b[0m"
#define COLOR_VERDE           "\x1b[32m"
#define COLOR_AZUL            "\x1b[34m"
#define COLOR_NOTIFICACION    "\x1b[47m"
#define opc                   6
#define clear()               printf("\033[H\033[J") //truquito tonto para que pareciera que se limpiara la pantalla
#define total_opc             6
#define tecla_enter           10 //mas bien es nueva linea, pero así lo recordaré
#define tecla_arriba          'A'
#define tecla_abajo           'B'
//En c puro no existen:
#define true                  1
#define false                 0
#define codigo_salida         5
#define codigo_lista_procesos 10
struct procesos
{
  char duenyo[20];
  int  PID;
};

int mostrarOpcion(int n_opc);
int getch(void);
int menu(int n_opc);
int ejecutarProceso(char *comando_completo);
int lista_archivos(char *directorio, int op, struct procesos *proc);
char* nombre_usuario(char *directorio);
int buscar_uid(char *linea);
int cantidad_proc(char *directorio);
int superListaProcesos(int n_opc, int *cantidad);
int control_teclas(int f, int *n_opc, int *menu_c);

int main(int argc, char** argv){
  int n_opc  = 1; //La opción en que comienza
  int menu_c = 0;
  int retorno_mostrarOpcion;
  int sel; //la letra que se presiono
  int mostrar_menu = true; //cuando alguien clickeó enter en una opción
  while(true){
    clear();
    if(mostrar_menu){
      mostrar_menu = control_teclas(menu_c, &n_opc, &menu_c);
    }else{ //Se ha entrado a alguna opción
      if(!menu_c){
        retorno_mostrarOpcion = mostrarOpcion(n_opc);
        if(retorno_mostrarOpcion == codigo_salida)
            return 0;
        else if(retorno_mostrarOpcion == codigo_lista_procesos)
            menu_c = 1;
      }
      mostrar_menu = true;
    }
  }
  return 0;
}
int control_teclas(int f, int *n_opc, int *menu_c){
  int sel; //la letra que se presiono
  int pid = 0;
  int cantidad;
  int primer;
  if(!f){
    primer = 1;
    menu(*n_opc);
    cantidad = total_opc;
  }else{
    pid = superListaProcesos(*n_opc, &cantidad);
    primer = 0;
  }
  sel = getch();
  if (sel == '\033') { // Para el esc que es el primer dato
    getch(); // para no usar [
    switch(getch()) { // el dato que se busca
      case 'A':
        *n_opc = (*n_opc > primer)?(*n_opc - 1):*n_opc;
      break;
      case 'B':
        *n_opc = (*n_opc < cantidad - 1)?(*n_opc + 1):*n_opc;
      break;
    }
  }else if(sel == tecla_enter){
    if(f){
      printf(COLOR_NOTIFICACION);
      printf(COLOR_ROJO);
      //printf ("\033[%d;%dH",20, 50);
      *menu_c = 0;
      if(!kill(pid, SIGKILL))
        printf(">Proceso %d asesinado >:) (Click en cualquier tecla)", pid);
      else
        printf(">No se ha podido asesinar el proceso :( (Click en cualquier tecla)");
      getch();
      *n_opc = 1;
    }else{
      *menu_c = *menu_c;
      return false;
    }
    printf(COLOR_NORMAL);
  }
  return true;
}
int mostrarOpcion(int n_opc){
  char comando_completo[100];
  int retorno = 0;
  int n_ = 1;
  char *opciones[opc] = {"\tGestión de procesos\n",
                      " 1) Ejecutar nuevo proceso\n"
                      " Escriba la ruta del proceso: ",
                      " 2) Lista de procesos\n",
                      " 3) Matar algún proceso\n",
                      " 4) Créditos\n"
                      " Hecho por: Kevin Hernández - ci 21256207\n"
                      " Presione una tecla para continuar",
                      " 5) Salir\n"
                      " Presione una tecla para salir"
                    };

  fflush(stdin);
  printf(COLOR_AZUL);
  printf("%s", opciones[0]);
  printf(COLOR_NORMAL);
  printf("%s", opciones[n_opc]);
  switch(n_opc){
    case 1:
      scanf("%99[^\n]",comando_completo); //el [^\n] es para que me deje leer espacios a 99 caracteres
      retorno = ejecutarProceso(comando_completo);
    break;
    case 2:
      retorno = lista_archivos("/proc", 0, NULL);
      getch();
    break;
    case 3:
      printf(COLOR_ROJO " Para ver mejor la lista maximice la consola.\n"COLOR_NORMAL);
      printf(" Presione tecla para continuar");
      getch();
      return codigo_lista_procesos;
    break;
    case 4:
      getch();
    break;
    case 5:
      getch();
      return 5;
    break;
  }

  return retorno;
}
int menu(int n_opc){
  int index;
  char *texto[opc] = {"\tGestión de procesos\n",
                    " 1) Ejecutar nuevo proceso\n",
                    " 2) Listar procesos\n",
                    " 3) Matar algún proceso\n",
                    " 4) Créditos\n",
                    " 5) Salir\n"
                    };
  for(index = 0; index < opc; index++){
    if(index == n_opc)
      printf(COLOR_VERDE);
    else if(!index)
      printf(COLOR_AZUL);
    printf("%s", texto[index]);
    printf(COLOR_NORMAL);
  }
}
int ejecutarProceso(char *comando_completo){
  pid_t pid;
  int index_com, num_arg = 1;
  char **argumentos;
  int index_arg  = 0;
  int letras_arg = 0;

  for(index_com = 0; index_com < strlen(comando_completo); index_com++)
    if(comando_completo[index_com] == ' ')
      num_arg++;
      argumentos = malloc((num_arg + 1)*sizeof(char)); //Creo el char * con el número de los argumentos
      for(index_com = 0; index_com < num_arg; index_com++)
        argumentos[index_com] = malloc(100*sizeof(char)); //Y luego a cada uno le reservo 20 chars
      /*guardo los argumentos*/
      for(index_com = 0; index_com < strlen(comando_completo); index_com++){
        if(comando_completo[index_com] == ' '){
          //argumentos[index_arg][letras_arg++] = '\0';
          letras_arg = 0;
          index_arg++;
        }else
          argumentos[index_arg][letras_arg++] = comando_completo[index_com];
        }
        argumentos[num_arg] = NULL;
        getch();
        pid = fork();
        if(!pid) {// Soy el hijo
          printf("\n\tEjecución del proceso %s (PID %d): \n",argumentos[0], getpid());
          execv(argumentos[0], argumentos);
          return 5; //Para que hijo se salga al terminar
        }else if(pid == -1)
        printf("Error: fork erróneo");
  return 0; // Padre
}
int buscar_uid(char *linea){
  char str[100];
  strcpy(str, linea);
  char * pch;
  int uid = -1;
  pch = strtok (str," \t");
  //el segundo número luego de uid es el atributo que necesito
  if(!strcmp(pch, "Uid:")){
    pch = strtok (NULL, " \t");
    uid = atoi(pch);
  }
return uid;
}
char* nombre_usuario(char *directorio){
  FILE * status;
  status = fopen (directorio,"r");
  char linea [100];
  int uid;
  struct passwd *pws;
  char *nombre = "";

  if(status != NULL){
    while(true){
      fgets (linea , 100 , status);
      uid = buscar_uid(linea);
      if(uid >= 0){
        pws = getpwuid(uid);
        nombre = pws->pw_name;
        break;
      }else if(feof(status)){
        break;
      }
    }
    fclose(status);
  }
  return nombre;
}

int lista_archivos(char *directorio, int op, struct procesos *proc){
      //Debe ser porque hay que revobinar el directorio o algo asi u.u
      DIR *dp;
      struct dirent *ep;
      int i = 0, numero_elementos_columnas = 30;
      int x;
      int y = -20;
      char* nombre;
      char *q;

      dp = opendir (directorio);
      if(dp != NULL){

        while (ep = readdir (dp))
          if(isdigit(ep->d_name[0])){ //Si hay un número (creo que siempre) debería se un proceso
            char direc[20];
            //Creo el string para leer luego el UID de status
            strcpy(direc, directorio);
            strcat(direc, "/");
            strcat(direc, ep->d_name);
            strcat(direc, "/status");
            nombre = nombre_usuario(direc); //FUNCIONA!
            if(!op){
              if(!(i % numero_elementos_columnas)){
                x  = 4;
                y += 20;
                printf (" \033[%d;%dH", x-1, y);
                printf ("PID:   Dueño:\n");
              }
              printf (" \033[%d;%dH", x, y);
              x += 1;
              printf ("%s    %8s", ep->d_name, nombre);
            }else{
              strcpy(proc[i].duenyo, nombre);
              proc[i].PID    = atoi(ep->d_name);
            }
            i++;
          }

          (void) closedir (dp);
        }
        else
          printf("Error: directorio %s errado", directorio);
        if(!op)
          printf("\033[%d;%dH Presione una tecla para continuar\n", 35, 0);
return 0;
}
int cantidad_proc(char *directorio){
  //No se cómo mas contar :(
  DIR *dp;
  dp = opendir (directorio);
  struct dirent *ep;
  int n = 0;
  if(dp != NULL){
    while (ep = readdir (dp))
      if(isdigit(ep->d_name[0]))
        n++;
      (void) closedir (dp);
    }
      return n;
}
int superListaProcesos(int n_opc, int *cantidad){
  int index;
  int posicion = 0;
  *cantidad = cantidad_proc("/proc");
  struct procesos *proc;
  //printf ("\nPID:  \t Dueño:\n");
  int x;
  int y = -20;
  int numero_elementos_columnas = 30;
  proc = malloc((*cantidad)*sizeof(struct procesos));
  lista_archivos("/proc", 1, proc);
  printf(COLOR_AZUL);
  printf("\tGestión de procesos\n");
  printf(COLOR_NORMAL);
  printf("3) Seleccione proceso de la lista:");

  for(index = 0; index < *cantidad; index++){
    if(!(index % numero_elementos_columnas)){
      x  = 4;
      y += 20;
      printf (" \033[%d;%dH", x-1, y);
      printf ("PID:   Dueño:\n");
    }
    printf (" \033[%d;%dH", x, y);
    x += 1;
    if(index == n_opc)
      printf(COLOR_ROJO);
    printf ("%d   %8s", proc[index].PID, proc[index].duenyo);
    printf(COLOR_NORMAL);
  }
  printf(COLOR_AZUL);
  printf("\033[%d;%dH Seleccione usando las flechas (arriba y abajo)\n", 35, 0);
  printf("Los procesos se mantendran actualizados\n");
  printf(COLOR_ROJO);
  printf("Para matar oprima enter\n");
  printf(COLOR_NORMAL);
  return proc[n_opc].PID;
}

/*Esto es equivalente al gecth que esta dentro de conio.h*/
int getch(void) {
  struct termios oldattr, newattr;
  int ch;
  tcgetattr( STDIN_FILENO, &oldattr );
  newattr = oldattr;
  newattr.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
  return ch;
}

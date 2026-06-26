#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
  No deberia por ningún motivo ser insuficiente,
  teniendo en cuenta que esto no pretende ser
  un editor de texto.
*/
#define BUFFER 1024
#define MAX_TAREA BUFFER / 2
//MBFP es por MAX BUFFER FULL PATH para que no quede largo
#define MBFP MAX_TAREA / 2
#define RESTO 5 // Valor a restar al bufer por los agregados (COMPLETA o INCOMPLETA) + "\0"
//El nombre es por BUFFER EFECTIVO, pero quedaba muy largo.
#define BUFFEF BUFFER - RESTO 
#define INCOMPLETA " [ ]"


/* Por el momento este programita va a ser solo para linux */
#define ARCHIVO ".tareas"
#define PATH ".config/"

// Estas lineas son las que van a separar en el archivo las tareas.
#define TAREAS_COMPLETAS "# COMPLETAS\n"
#define TAREAS_INCOMPLETAS "# INCOMPLETAS\n"
#define MAX_LE 15
//No es lo mas optimo pero hacer una table hash me parecio mucho
#define COMANDOS ((const char*[])  {"-add","-li","-rm","-rst","-lc"})

  



typedef struct {
  char *lista[MAX_TAREA];
  int cantidad;
} tarea_t;

//Mi problema para nombrar es grande.
/* La idea es tener tanto tareas completadas como incompletas
 asi de querese poder resetear las completas, tendria que ver tambien
si mas adelante quiero agregar "Tareas complejar"*/
typedef struct {
  tarea_t incompleta;
  tarea_t completa;
} estado_t;

void agregar_tarea(char cad[])
{
  fgets(cad,BUFFEF,stdin);
  int salto = strlen(cad);
  cad[salto - 1] = '\0';
  strncat(cad,INCOMPLETA,sizeof(&cad) - strlen(cad) - 1);
  strncat(cad,"\n",sizeof(&cad) - strlen(cad) - 1);
}

void completar_tarea(char cad[])
{
  /*el fin de linea es \0 strlen cuenta los caracteres incluso ese
  al restarte 2 quedo posicionado en el espacio"*/
  int pos = strlen(cad) - 2;
  cad[pos] = 'X';
}


/*FIXME: Tanto agregar_tarea como
  completar_tarea ni agregan de manera
  correcta el estado si la longitud de la
  es inferior a ocho.
*/

long size_file(FILE *f)
{
  if (f == NULL) exit(1);
  //Voy al final del archivo
  int fs = fseek(f,0L,SEEK_END);
  (void) fs;
  //guardo la longitud del archivo
  long n = ftell(f);
  if (n < 0) {
    perror("ftell");
  }
  //Me posicikono al comienzo del archivo
  fseek(f,0L,SEEK_SET);
  return n;
}

long cargar_file(FILE *f,unsigned char **file_buffer)
{

  long size = size_file(f);
  //Si size es cero significa que el archivo acaba de ser creado
  //No hayu nada que leer.
  if (size > 0) {
  *file_buffer = (unsigned char*)malloc(size + 1);
  size_t ret = fread(*file_buffer,size,1,f);
  (void)ret;
  } 
  return size;
}


int existe(tarea_t tareas,char *cad)
{
  int ret = 0;
  int i = 0;
  while (i < tareas.cantidad && strcoll(tareas.lista[i],cad) != 0) i++;

  //Si no llego al total es porque encontro una cadena que es igual.
  if (i < tareas.cantidad) ret = 1;


  return ret;

}

//FIXME: No se deberia poder agregar una tarea dos veces.
void agregar_en_lista(tarea_t *tareas, char *cad)
{

  if (!existe(*tareas,cad)){
    tareas->lista[tareas->cantidad] = strdup(cad);
    if (tareas->lista[tareas->cantidad] == NULL) {
      fprintf(stderr,"MALLOC ERROR: No se pudo reserver memoria\n");
      exit(1);
    }
    tareas->cantidad++;
  }
}

//Aca simplemente se hace un desplazo de todos los elementos restantes del array.
void eliminar_tarea(tarea_t *tarea, int pos)
{
  int posR = pos - 1;
  if (posR >= 0 && posR < tarea->cantidad) {
    for (int i = posR; i < tarea->cantidad -1; i++){
      tarea->lista[i] = tarea->lista[i + 1];
    }
    tarea->cantidad--;
  } 
 
}

void resetear_tarea(estado_t *tareas, int pos)
{
  /*Vamos a explicar esto: tareas->incompleta.cantidad siempre esta +1 por encima
  del contador, los indices empiezan en 0, por lo que uso cantidad como indice
  Ademas tengo pos, que aparece como la posicion real + 1
  por lo que tengo que restarle 1 a pos para que de ael indice correcto.u
  */
  tareas->incompleta.lista[tareas->incompleta.cantidad] = tareas->completa.lista[pos - 1];
  tareas->incompleta.cantidad++;
  eliminar_tarea(&tareas->completa,pos);
}




void cargar_listas(estado_t *tareas, FILE *f)
{
  char lineaDeEstado[MAX_LE];
  char linea[MAX_TAREA];
  while (fgets(linea,MAX_TAREA,f) != NULL) {

    if (linea[0] == '#') {
      strcpy(lineaDeEstado,linea);
    } else {
      if (strcoll(lineaDeEstado,TAREAS_COMPLETAS)) {
        agregar_en_lista(&tareas->completa,linea);
      } else {
        agregar_en_lista(&tareas->incompleta,linea);
      }
    }
  }

}
  
void imp_lista(tarea_t lista)
{
  for (int i = 0; i < lista.cantidad; i++){
    printf("%d - %s",i + 1,lista.lista[i]);
  }
}

int selec_lista_imp(estado_t tareas)
{

  int nl = -1;
  if (tareas.completa.cantidad > 0 && tareas.incompleta.cantidad > 0) {
    printf("Seleccione la lista de la cual desea eliminar una tarea\n\t");
    printf("[0] INCOMPLETAS - [1] COMPLETAS\n");
    scanf("%d",&nl);
    //Por el momento lo vamos a hacer asi, la idea es que funcione, 
    //pero mas adelante habra que ver como hacer algo parecido a less,
    //cuando haya una cantidad de tarea grande
    if (nl == 0) {
      imp_lista(tareas.incompleta);
    } else if (nl == 1) {
      imp_lista(tareas.completa);
    }

  } else {
    tarea_t lista;
    lista.cantidad = 0;

    if (tareas.incompleta.cantidad > 0) {
      lista = tareas.incompleta;
      nl = 0;
    }
    else if(tareas.completa.cantidad > 0) {
      lista = tareas.completa;
      nl = 1;
    }
    if (lista.cantidad > 0) imp_lista(lista);

  }

  return nl;
}




void ejecutar_comando(estado_t *tareas, char *comando)
{
  //Esto es bastante sucio pero por el momento va a funcionar
  //La desgracia es que cada vez que agregue un comendo el if else va a se4r mas grande
  char cad[BUFFER];
  int nt;
  if (strcoll(comando,COMANDOS[0]) == 0 ) {
    agregar_tarea(cad);
    agregar_en_lista(&tareas->incompleta,cad);
    //{"add","ls","rm","rst","lc"})
    imp_lista(tareas->incompleta);
  } else if (strcoll(comando,COMANDOS[1]) == 0) {
    imp_lista(tareas->incompleta);
  } else if (strcoll(comando,COMANDOS[2]) == 0) {
    int nl = selec_lista_imp(*tareas);
    if (nl != -1) {

      printf("Seleccione tarea a eliminar: \n");

      tarea_t *lista = NULL;
      if (nl == 0) lista = &tareas->incompleta;
      else if (nl == 1) lista = &tareas->completa;

      scanf("%d",&nt);
      if (nt > 0 && nt <= lista->cantidad) {
        printf("Tareas eliminada: %s\n",lista->lista[nt -1]);
        eliminar_tarea (lista,nt);

      }
    } else if (strcoll(comando,COMANDOS[3]) == 0) {
      imp_lista(tareas->completa);
      printf("Seleccione la tarea a resetear(1..%d):\n",tareas->completa.cantidad);
      scanf("%d",&nt);
      resetear_tarea(tareas,nt);
      imp_lista(tareas->incompleta);
    } else if (strcoll(comando,COMANDOS[4]) == 0) {
      imp_lista(tareas->completa);
    }
  }
}


int guardar_file(FILE *f, tarea_t tareas, char *tipo)
{
  int n;
  //Muy "a mano" pero se que va a funcionar.
  fwrite(tipo,sizeof(char),strlen(tipo),f);
  for (int i = 0; i < tareas.cantidad; i++) {
    n = fwrite(tareas.lista[i],sizeof(char),strlen(tareas.lista[i]),f);
  }
  return n;
}


int main(int argc, char **argv)
{
  char full_path[256];
  estado_t tareas;
  tareas.completa.cantidad = 0;
  tareas.incompleta.cantidad = 0;
  //acá armo la ruta absoluta
  snprintf(full_path,sizeof(full_path),"%s/%s%s",getenv("HOME"),PATH,ARCHIVO);
  FILE *f = fopen(full_path,"r+");
  //Otra cosa que ahora no estoy teniendo en cuenta es que el archivo si exista pero
  //que el path sea incorrecto, voy a poner un FIXME pero esto en linux deberia funcionar.
  if (f == NULL) {
    f = fopen(full_path,"w");
    if (f == NULL) {
      //Puede que sea redundante pero no se me ocurre una mejopr manera
      //La idea es "Si el archivo no existe lo creo, si no lo puedo crear mando error"
      perror("fopen (r)");
      exit(1);
    }


  }
  if (size_file(f) == 0) {
    printf("No se han cargado tareas\n");
  } else {
    cargar_listas(&tareas,f);
  }

  if (argc == 2) ejecutar_comando(&tareas,argv[1]);
  else if (argc > 2) fprintf(stderr, "ERROR: Demasiados argumentos\n");
  guardar_file(f,tareas.incompleta,TAREAS_INCOMPLETAS);
  guardar_file(f,tareas.completa,TAREAS_COMPLETAS);
  fclose(f);
  return 0;
}


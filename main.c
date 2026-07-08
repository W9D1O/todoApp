#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "tarea.h"
#include "archivo.h"

//MBFP es por MAX BUFFER FULL PATH para que no quede largo
#define MBFP MAX_TAREA / 2



//No es lo mas optimo pero hacer una table hash me parecio mucho
#define COMANDOS ((const char*[])  {"-add","-li","-rm","-rst","-lc","-ct"})

#define UNO 49



//Mi problema para nombrar es grande.
/* La idea es tener tanto tareas completadas como incompletas
 asi de querese poder resetear las completas, tendria que ver tambien
si mas adelante quiero agregar "Tareas complejar"*/





  

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
  if (strcmp(comando,COMANDOS[0]) == 0 ) {
    agregar_tarea(cad);
    agregar_en_lista(&tareas->incompleta,cad);
    //{"add","ls","rm","rst","lc"})
    imp_lista(tareas->incompleta);
  } else if (strcmp(comando,COMANDOS[1]) == 0) {
    imp_lista(tareas->incompleta);
  } else if (strcmp(comando,COMANDOS[2]) == 0) {
    int nl = selec_lista_imp(*tareas);
    if (nl != -1) {

      printf("Seleccione tarea a eliminar: \n");

      tarea_t *lista = NULL;
      if (nl == 0) lista = &tareas->incompleta;
      else if (nl == 1) lista = &tareas->completa;

      scanf("%d",&nt);
      if (nt > 0 && nt <= lista->cantidad) {
        printf("Tareas eliminada: %s\n",lista->lista[nt -1]);
        eliminar_tarea (lista,nt,true);

      }
    }
  }
  else if (strcmp(comando,COMANDOS[3]) == 0) {
    imp_lista(tareas->completa);
    printf("Seleccione la tarea a resetear(1..%d):\n",tareas->completa.cantidad);
    scanf("%d",&nt);
    rc_tarea(&tareas->incompleta,&tareas->completa,nt);
    imp_lista(tareas->incompleta);
  } else if (strcmp(comando,COMANDOS[4]) == 0) {
    imp_lista(tareas->completa);
  } else if (strcmp(comando,COMANDOS[5]) == 0) {
    imp_lista(tareas->incompleta);
    printf("Seleccione la tarea completada(1..%d):\n",tareas->incompleta.cantidad);
    scanf("%d",&nt);
    rc_tarea(&tareas->completa,&tareas->incompleta,nt);
    imp_lista(tareas->completa);
  }
}




void menu(estado_t *tareas)
{
  char c;
  do {
    do {
      printf("Seleccione la opcion deseada o presione 'q' para salir: \n");
      printf("\t1 - Agregar nueva tarea \n"); 
      printf("\t2 - Listar tareas pendientes\n"); 
      printf("\t3 - Eliminar tarea \n"); 
      printf("\t4 - Restaurar tarea \n"); 
      printf("\t5 - Listar tareas completadas \n"); 
      printf("\t6 - Marcar tarea completada \n"); 
      scanf(" %c",&c);
    } while (c < '1' || (c > '6' && c != 'q'));

    if (c != 'q') {
      //Para convertir una char dek 0..9 se le resta 48 a eso le sumo 1 para que corresponda con los indices
      char *coman =  COMANDOS[c - UNO];
      printf(" %s\n",coman); 
      ejecutar_comando(tareas,coman);
      printf("Presione '0' para volver al menu anterior o presione otra tecla para salir: \n");
      scanf(" %c",&c);
    }
  } while (c == '0');

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

  long sf = size_file(f);
  if (sf > 0) {
    cargar_listas(&tareas,f);
    fclose(f);
    //Esto se agrego porque pasabe que si el archivo era menor que el original
    // dejaba restos del anterior y hacia parece que eliminar no funcionaba.
    f = fopen(full_path,"w");
    if (f == NULL) {
      perror("fopen (r)");
      exit(1);
    }
  }

  if (argc == 2) ejecutar_comando(&tareas,argv[1]);
  else if (argc > 2) fprintf(stderr, "ERROR: Demasiados argumentos\n");
  else if (argc == 1) menu(&tareas);
  guardar_file(f,tareas.incompleta,TAREAS_INCOMPLETAS);
  guardar_file(f,tareas.completa,TAREAS_COMPLETAS);
  fclose(f);
  return 0;
}


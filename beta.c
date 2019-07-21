#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INF 999999
typedef enum { false, true} bool;

typedef struct Nivel{
	int id;
	int x;
	int y;
	int vidas;
	bool monstruo;
	int llave;
	int puerta;
	int peso;
	struct Nivel *next;
}Nivel;

typedef struct Piso{
	Nivel **nivel;
}Piso;

FILE *abrir(){
	FILE *fp;
	char archivo[50];
	printf("Ingresar nombre de archivo: ");
	scanf("%s",archivo);
	fp = fopen(archivo,"r");
	while(!fp){
		printf("Error! no se encuentra el archivo\nIngresar nuevamente: ");
		scanf("%s",archivo);
		fp = fopen(archivo,"r");
	}
	printf("Archivo leído correctamente\n");
	return fp;
}

void *tokenize(int *coord,char *linea){
	char *tok;
	int i;
	tok = strtok(linea," \0\n");
	i = 0;
	while(tok!=NULL){
		coord[i] = atoi(tok);
		i++;
		tok = strtok(NULL," ");
	}	
}

Nivel *generarAdyacencia(int piso,int x,int y,int ancho, int largo){
	Nivel *head = NULL,*casilla;
	int xx[] = {0,0,-1,1};	//"dirección" en la que se puede mover en eje x
	int yy[] = {-1,1,0,0};	//lo mismo para eje y
	int i,a,b;
	for(i = 0; i < 4; i++){
		a = x + xx[i];
		b = y + yy[i];
		if(a < 0 || a >= ancho || b < 0 || b >= largo){
			continue;
		}
		casilla = (Nivel*)malloc(sizeof(Nivel));
		casilla->id = piso;
		casilla->peso = 1;
		casilla->x = a;
		casilla->y = b;
		casilla->puerta = -1;
		casilla->next = head;
		head = casilla;
	}
	return head;
}

Nivel **crearAdyacencias(Nivel **lvl,int piso,int largo,int ancho){
	int i,j;
	for(i = 0; i < ancho; i++){
		for(j = 0; j < largo; j++){
			lvl[i][j].next = generarAdyacencia(piso,i,j,ancho,largo);
		}
	}
	return lvl;
}

Nivel **crearNivel(int piso,int largo,int ancho){
	Nivel **lvl;
	int i,j;
	lvl = (Nivel**)malloc(ancho*sizeof(Nivel*));
	for (i = 0; i < ancho; ++i){
		lvl[i] = (Nivel*)malloc(largo*sizeof(Nivel));
	}
	for(i = 0; i < ancho; i++){
		for(j = 0; j < largo; j++){
			lvl[i][j].id = piso;
			lvl[i][j].x = i;
			lvl[i][j].y = j;
			lvl[i][j].monstruo = false;
			lvl[i][j].peso = 1;
			lvl[i][j].llave = -1;
			lvl[i][j].puerta = -1;
		}
	}
	lvl = crearAdyacencias(lvl,piso,largo,ancho);
	return lvl;
}

Piso *crearPisos(int pisos,int largo,int ancho){
	Piso *piso;
	piso = (Piso*)malloc(pisos*sizeof(Piso));
	int i;
	for(i = 0; i < pisos; ++i){
		piso[i].nivel = crearNivel(i,largo,ancho);
	}
	return piso;
}


void quitarArco(Nivel **nodo,int i,int j){
	Nivel *temp = *nodo, *prev;
	if(temp!=NULL){
		if (temp->x == i && temp->y == j){
			*nodo = temp->next;
			free(temp);
			return; 
		}
	}

	if(temp == NULL) return;
	while(temp->next != NULL){
		if(temp->next->x == i && temp->next->y == j){
			prev = temp->next;
			temp->next = temp->next->next;
			free(prev);
			return;
		}
		temp = temp->next;
	}
}

Piso *crearPuertas(FILE *fp, char *linea,Piso *p,int **puertas,int *size){
	int coord[6],i,k=0,aux;
	while(fgets(linea,255,fp)){
		if(linea[0]>='a' && linea[0]<='z'){
			return p;
		}
		tokenize(coord,linea);

		for(i = 0; i < 6; i++){
			puertas[k][i] = coord[i];
		}
		k++;
		(*size)++;
		//Puertas horizontales
		if(coord[3] == coord[5]){
			if(coord[2]>coord[4]){ //swap de coordenadas para poder iterar correctamente
				aux = coord[2];
				coord[2] = coord[4];
				coord[4] = aux;
			}
			for(i = coord[2]; i < coord[4];i++){
				p[coord[1]].nivel[coord[5]-1][i].puerta = coord[0];
				p[coord[1]].nivel[coord[5]][i].puerta = coord[0];
				quitarArco(&p[coord[1]].nivel[coord[5]-1][i].next,coord[3],i);
				quitarArco(&p[coord[1]].nivel[coord[5]][i].next,coord[3]-1,i);
			}
		}
		//Puertas verticales
		else if(coord[2] == coord[4]){
			if(coord[3]>coord[5]){
				aux = coord[3];
				coord[3] = coord[5];
				coord[5] = aux;
			}
			for(i = coord[3]; i < coord[5]; i++){
				p[coord[1]].nivel[coord[5]-1][i].puerta = coord[0];
				p[coord[1]].nivel[coord[5]][i].puerta = coord[0];
				quitarArco(&p[coord[1]].nivel[i][coord[2]-1].next,i,coord[2]);
				quitarArco(&p[coord[1]].nivel[i][coord[2]].next,i,coord[2]-1);
			}
		}
	}
}

void anadirArco(Nivel **nodo,int id,int i,int j){
	Nivel *temp = *nodo,*aux = (Nivel*)malloc(sizeof(Nivel));
	aux->id = id;
	aux->x = i;
	aux->y = j;
	aux->peso = 1;
	aux->next = NULL;
	while(temp->next != NULL){
		temp = temp->next;
	}
	temp->next = aux;
}
Piso *crearParedes(FILE *fp,char *linea,Piso *p){
	int coord[5],i,aux;
	fgets(linea,100,fp);
	while(fgets(linea,100,fp)){
		if(linea[0]>='a' && linea[0]<='z') {
			return p;
		}
		tokenize(coord,linea);
		//Paredes horizontales
		if(coord[2] == coord[4]){
			if(coord[1]>coord[3]){ //swap de coordenadas para poder iterar correctamente
				aux = coord[1];
				coord[1] = coord[3];
				coord[3] = aux;
			}
			for(i = coord[1]; i < coord[3];i++){
				quitarArco(&p[coord[0]].nivel[coord[4]-1][i].next,coord[2],i);
				quitarArco(&p[coord[0]].nivel[coord[4]][i].next,coord[2]-1,i);
			}
		}
		//Paredes verticales
		else if(coord[1] == coord[3]){
			if(coord[2]>coord[4]){
				aux = coord[2];
				coord[2] = coord[4];
				coord[4] = aux;
			}
			for(i = coord[2]; i < coord[4]; i++){
				quitarArco(&p[coord[0]].nivel[i][coord[1]-1].next,i,coord[1]);
				quitarArco(&p[coord[0]].nivel[i][coord[1]].next,i,coord[1]-1);
			}
		}
	}
	return p;
}

Piso *ponerLlaves(FILE *fp,char *linea,Piso *p){
	int coord[4];
	while(fgets(linea,100,fp)){
		if(linea[0]>='a' && linea[0]<='z') {
			return p;
		}
		tokenize(coord,linea);
		p[coord[1]].nivel[coord[3]][coord[2]].llave = coord[0];
	}
	return p;
}

Piso *ponerMonstruos(FILE *fp,char *linea,Piso *p){
	int coord[4];
	while(fgets(linea,100,fp)){
		if(linea[0]>='a' && linea[0]<='z') {
			return p;
		}
		tokenize(coord,linea);
		p[coord[0]].nivel[coord[2]][coord[1]].peso = coord[3];
		p[coord[0]].nivel[coord[2]][coord[1]].monstruo = true;
		p[coord[0]].nivel[coord[2]][coord[1]].vidas = coord[3];
	}
	return p;
}

Piso *ponerPortales(FILE *fp,char *linea,Piso *p){
	int coord[6];
	while(fgets(linea,100,fp)){
		if(linea[0]>='a' && linea[0]<='z') {
			return p;
		}
		tokenize(coord,linea);
		anadirArco(&p[coord[0]].nivel[coord[2]][coord[1]].next,coord[3],coord[5],coord[4]);
		anadirArco(&p[coord[3]].nivel[coord[5]][coord[4]].next,coord[0],coord[2],coord[1]);
	}
	return p;
}

Piso *ponerEscaleras(FILE *fp,char *linea,Piso *p){
	int coord[3];
	while(fgets(linea,100,fp)){
		if(linea[0]>='a' && linea[0]<='z') {
			return p;
		}
		tokenize(coord,linea);
		anadirArco(&p[coord[0]].nivel[coord[2]][coord[1]].next,coord[0]+1,coord[2],coord[1]);
		anadirArco(&p[coord[0]+1].nivel[coord[2]][coord[1]].next,coord[0],coord[2],coord[1]);
	}
	return p;
}

Piso *quitarPuerta(Piso *p, int llave, int **puertas, int size){
	int i,j,aux;
	for(i = 0; i < size; i++){
		if(puertas[i][0] == llave){
			if(puertas[i][3] == puertas[i][5]){
				if(puertas[i][2]>puertas[i][4]){ //swap de coordenadas para poder iterar correctamente
					aux = puertas[i][2];
					puertas[i][2] = puertas[i][4];
					puertas[i][4] = aux;
				}
				for(j = puertas[i][2]; j < puertas[i][4];j++){
					int id = puertas[i][1];
					anadirArco(&p[id].nivel[puertas[i][5]-1][j].next,puertas[i][1],puertas[i][3],j);
					anadirArco(&p[id].nivel[puertas[i][5]][j].next,puertas[i][1],puertas[i][3]-1,j);
				}
			}
			//Puertas verticales
			else if(puertas[i][2] == puertas[i][4]){
				if(puertas[i][3]>puertas[i][5]){
					aux = puertas[i][3];
					puertas[i][3] = puertas[i][5];
					puertas[i][5] = aux;
				}
				for(j = puertas[i][3]; j < puertas[i][5]; j++){
					anadirArco(&p[puertas[i][1]].nivel[j][puertas[i][2]-1].next,puertas[i][1],j,puertas[i][2]);
					anadirArco(&p[puertas[i][1]].nivel[j][puertas[i][2]].next,puertas[i][1],j,puertas[i][2]-1);
				}
			}
		}
	}
	return p;
}

bool Vista(int puerta,int *pasada,int size);
Nivel minDistancia(bool ***visited, int ***costo, int pisos, int ancho, int largo);
void Dijkstra(Piso *p, Nivel inicio, Nivel final, int pisos, int largo, int ancho, int **puertas, int size_puertas);
int main(){
	while(1){
		Nivel Hrognan,salida;
		FILE *fp = abrir();
		Piso *piso;
		int i,j,k;
		int pisos,largo,ancho;
		printf("Ingresar cantidad de vidas de Hrognan: ");
		scanf("%d",&Hrognan.vidas);
		fscanf(fp,"%d",&pisos);
		fscanf(fp,"%d%d",&largo,&ancho);
		piso = crearPisos(pisos,largo,ancho);
		fscanf(fp,"%d%d%d",&Hrognan.id,&Hrognan.x,&Hrognan.y);
		fscanf(fp,"%d%d%d",&salida.id,&salida.x,&salida.y);
		char linea[100];
		int **puertas = (int**)malloc(100*sizeof(int*));	//matriz con las puertas en la mazmorra, necesario para abrir puertas en dijkstra
															//en caso de tener que abrir una/s(se debe hacer el proceso inverso a la función crearPuertas)
		int **llaves = (int**)malloc(100*sizeof(int*));		//matriz para guardar las llaves que posiblemente sean necesarias para abrir una puerta haciendo dijkstra
		int size_puertas = 0, size_llaves = 0; //cantidad de puertas que hay en la mazmorra para iterar sobre matriz
		for(i = 0; i < 100; i++){
			puertas[i] = (int*)malloc(6*sizeof(int));
			llaves[i] = (int*)malloc(4*sizeof(int));
		}
		piso = crearParedes(fp,linea,piso);
		piso = crearPuertas(fp,linea,piso,puertas,&size_puertas);
		piso = ponerLlaves(fp,linea,piso);
		piso = ponerMonstruos(fp,linea,piso);
		piso = ponerPortales(fp,linea,piso);
		piso = ponerEscaleras(fp,linea,piso);
		Dijkstra(piso,Hrognan,salida,pisos,largo,ancho,puertas,size_puertas);
		printf("¿Desea agregar otro caso? Y/N\n");
		getchar();
		char c;
		scanf("%c",&c);
		if(c == 'n' || c == 'N') break;
	}
	return 0;
}

Nivel minDistancia(bool ***visited, int ***costo, int pisos, int ancho, int largo){
	int min = INF,i,j,k;
	Nivel min_coord;
	for(i = 0; i < pisos; i++){
		for(j = 0; j < ancho; j++){
			for(k = 0; k < largo; k++){
				if(visited[i][j][k] == false && costo[i][j][k] < min){
					min = costo[i][j][k];
					min_coord.id = i;
					min_coord.x = j;
					min_coord.y = k;
				}
			}
		}
	}
	return min_coord;
}

//función para ver si es que una puerta por la que pasamos no ha sido encontrada de nuevo dentro
//del recorrido, esto para evitar agregar puertas con el mismo id al array puertapasada[]
bool Vista(int puerta,int *pasada,int size){
	int i;
	for(i = 0; i < size; i++){
		if(pasada[i] == puerta) return true;
	}
	return false;
}

void Dijkstra(Piso *p, Nivel inicio, Nivel final, int pisos,int largo,int ancho,int **puertas, int size_puertas){
	bool ***visited = (bool***)malloc(ancho*(sizeof(bool**)));
	int ***costo = (int***)malloc(ancho*(sizeof(int**)));
	Nivel ***parent = (Nivel***)malloc(ancho*(sizeof(Nivel**)));
	int i,j,k;
	int llaves[1000][2], size_ll = 0; //arreglo de llaves con size por si se ocupa una, se aumenta su tamaño
	for(i = 0; i < pisos; i++){
		visited[i] = (bool**)malloc(ancho*sizeof(bool*));
		costo[i] = (int**)malloc(ancho*sizeof(int*));
		parent[i] = (Nivel**)malloc(ancho*sizeof(Nivel*));
		for(j = 0; j < ancho; j++){
			visited[i][j] = (bool*)malloc(largo*sizeof(bool));
			costo[i][j] = (int*)malloc(largo*sizeof(int));
			parent[i][j] = (Nivel*)malloc(largo*sizeof(Nivel));
		}
	}
	Nivel dummy,casilla,*aux;
	dummy.id = dummy.x = dummy.y = -1;
	for(i = 0; i < pisos; i++){
		for(j = 0; j < ancho; j++){
			for(k = 0; k < largo; k++){
				costo[i][j][k] = INF;
				visited[i][j][k] = false;
				parent[i][j][k] = dummy;
			}
		}
	}
	costo[inicio.id][inicio.y][inicio.x] = 0;
	for(i = 0; i < pisos; i++){
		for(j = 0; j < ancho; j++){
			for(k = 0; k < largo; k++){
				//se calcula la casilla que tiene la minima distancia, para la primera iteración será inicio,
				//para las siguientes se verán los vecinos de inicio, y así. Se va calculando la distancia desde
				//u a N(u), donde se obtiene la casilla con menor distancia en N(u)
				casilla = minDistancia(visited,costo,pisos,ancho,largo);
				visited[casilla.id][casilla.x][casilla.y] = true;
				aux = p[casilla.id].nivel[casilla.x][casilla.y].next;
				//Se ven los vecinos de la casilla en la que estamos actualmente
				while(aux != NULL){
					if(visited[aux->id][aux->x][aux->y] == false && costo[casilla.id][casilla.x][casilla.y]+p[aux->id].nivel[aux->x][aux->y].peso < costo[aux->id][aux->x][aux->y]){
						if(p[aux->id].nivel[aux->x][aux->y].vidas < inicio.vidas && p[aux->id].nivel[aux->x][aux->y].monstruo){
							int w;
							for(w = 0; w < p[aux->id].nivel[aux->x][aux->y].vidas; w++){
								inicio.vidas--;
							}
						}
						else if(p[aux->id].nivel[aux->x][aux->y].vidas >= inicio.vidas && p[aux->id].nivel[aux->x][aux->y].monstruo){
							costo[aux->id][aux->x][aux->y] = INF;
							visited[aux->id][aux->x][aux->y] = true;
							visited[casilla.id][casilla.x][casilla.y] = false;
							aux = aux->next;
							continue;
						}

						costo[aux->id][aux->x][aux->y] = costo[casilla.id][casilla.x][casilla.y]+p[aux->id].nivel[aux->x][aux->y].peso; //aux->peso;
						parent[aux->id][aux->x][aux->y] = casilla;
						if(p[aux->id].nivel[aux->x][aux->y].llave != -1){
							p = quitarPuerta(p,p[aux->id].nivel[aux->x][aux->y].llave,puertas,size_puertas);
							llaves[size_ll][0] = p[aux->id].nivel[aux->x][aux->y].llave;
							llaves[size_ll][1] = 0; //0 quiere decir que no ha sido utilizada
							size_ll++;
						}	
					}
					aux = aux->next;
				}
			}
		}
	}
	Nivel camino;
	camino.id = final.id;
	camino.x = final.y;
	camino.y = final.x;
	Nivel path[1000];
	int index = 0;
	int puertapasada[100];
	int sizepuertapasada = 0;
	if(parent[final.id][final.y][final.x].id == -1){
		printf("No es posible llegar :(\n");
		return;
	}
	while(camino.id != -1){
		path[index] = camino;
		aux = p[camino.id].nivel[camino.x][camino.y].next;
		camino = parent[camino.id][camino.x][camino.y];
		if(camino.id == -1){
			index++;
			path[index] = camino;
			break;
		}
		index++;
		int puerta = p[camino.id].nivel[camino.x][camino.y].puerta;
		while(aux != NULL){
			for(i = 0; i < size_ll; i++){					int llave = p[aux->id].nivel[aux->x][aux->y].llave;
				if(llave == llaves[i][0] && llaves[i][1] != 1 ){ //llaves[i][1] indica si fue tomada, 1=tomada
					llaves[i][1] = 1;
					camino = p[aux->id].nivel[aux->x][aux->y];
					break;
				}
			}
			aux = aux->next;
		}
	}
	printf("Camino mas corto\n");
	for(i = index-1; i >= 0; i--){
		printf("[%d,%d,%d],",path[i].id,path[i].x,path[i].y);
	}
	printf("\n");
}
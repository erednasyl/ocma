#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STR 50

//ações disponíveis
char *ACTION[4] = {
  "LEFT",
  "RIGHT",
  "UP",
  "DOWN"
};

//armazena coordenadas
typedef struct {
  int x, y;
} Point;

//guarda informações do mapa
typedef struct {
  int **area;
  int portQnt;
  Point *portPosition;
} Map;

//guarda informações relevantes de cada jogador
typedef struct {
  bool haveGoal;
  Point goal;
  Point curPosition;
} Player;

//lê dados do mapa
Map readMapData(int h, int w){
  int read;
  Map map;
  Point port;
  map.portQnt = 0;
  //aloca espaço para a matriz do mapa
  map.area = (int**)calloc(h, sizeof(int*));
  for (int i = 0; i < h; i++){
      map.area[i] = (int*)calloc(w, sizeof(int));
  }
  //aloca espaço para um porto
  map.portPosition = malloc(sizeof(Point));

  for (int i = 0; i < h; i++) {   
    for (int j = 0; j < w; j++) {
      scanf("%i", &read);
      //atribui valor a cada posição da matriz
      map.area[i][j] = read;
      //armazena e realoca a localização do(s) porto(s)
      if (read == 1){
        port.y = j;
        port.x = i;
        map.portPosition = realloc(map.portPosition, (sizeof(Point) * map.portQnt) + sizeof(Point));
        map.portPosition[map.portQnt] = port;
        map.portQnt++;
      }
    }
  }
  return map;
}

//lê dados desse bot em específico
Player readPlayerData(char* myId) {
  Player player;
  Point position;

  char id[MAX_STR];
  int n, x, y;

  // lê os dados dos bots
  scanf(" BOTS %i", &n);
  // o " " antes de BOTS é necessário para ler o '\n' da linha anterior
  for (int i = 0; i < n; i++) {
    scanf("%s %i %i", id, &position.x, &position.y);
    if (strcmp(id, myId) == 0){
      player.curPosition = position;
    }
  }
  return player;
}

void findHorizontalPath(Point curPosition, Point goal, FILE* log, bool wasBusy){
  if(wasBusy){
    //sugerir novo movimento aleatório
    printf("%s\n", ACTION[rand() % 4]);
    return;
  }
  fprintf(log, "Procurando caminho horizontal para o porto\n");
  if (curPosition.y - goal.y < 0){
    printf("RIGHT\n");
  }
  else if (curPosition.y - goal.y > 0){
    printf("LEFT\n");
  }
  else if (curPosition.y - goal.y == 0){
    return;
  }
}

void findVerticalPath(Point curPosition, Point goal, FILE* log, bool wasBusy){
  if(wasBusy){
    //sugerir novo movimento aleatório
    printf("%s\n", ACTION[rand() % 4]);
    return;
  }
  fprintf(log, "Procurando caminho vertical para o porto\n");
  if (curPosition.x > goal.x){
    printf("UP\n");
  }
  else if (curPosition.x < goal.x){
    printf("DOWN\n");
  }
}

void movimento (int xPos, int yPos, int** map, int mapH, int mapV, int* capacity, bool wasBusy){

  if(wasBusy){
    //sugerir novo movimento aleatório
    printf("%s\n", ACTION[rand() % 4]);
    return;
  }
  
  if (((map[xPos][yPos] % 10) > 1) && (*capacity < 10)){
    printf("FISH\n");
    return;
  }

  int vmaxvalue = 50;
  int vmaxpos = 0;
  int hmaxvalue = 50;
  int hmaxpos = 0;

  //VERTICAL
  for (int i = 0; i < mapV-1; i++){
    if (((map[i][yPos] % 10) > 1)){
      if (abs(i - xPos) < vmaxvalue){vmaxvalue = abs(i - xPos); vmaxpos = i;}
    }
  }

  //HORIZONTAL
  for (int i = 0; i < mapH-1; i++){
    if ((map[xPos][i] % 10) > 1){
      if (abs (i - yPos) < hmaxvalue){hmaxvalue= abs (i - yPos); hmaxpos = i;}
    }
  }

  bool betterChooseHorizontal = vmaxvalue > hmaxvalue;
  if (betterChooseHorizontal){
    if (hmaxpos > yPos){
      printf("RIGHT\n");
    }
    else {
      printf("LEFT\n");
    }
  }
  else {
    if (vmaxpos > xPos){
      printf("DOWN\n");
    }
    else {
      printf("UP\n");
    }
  }
}

Point findBestPort (Map map, int xPos, int yPos){
  Point bestPortATM = map.portPosition[0];
  int curPortDistance = abs(yPos - map.portPosition[0].y) + abs(xPos - map.portPosition[0].x);

  //itera toda a lista de portos e verifica qual é o mais próximo para retornar
  for (int i = 0; i < map.portQnt; i++){
    if (abs(yPos - map.portPosition[i].y) + abs(xPos - map.portPosition[i].x) < curPortDistance){
      curPortDistance = abs(yPos - map.portPosition[i].y) + abs(xPos - map.portPosition[i].x);
      bestPortATM = map.portPosition[i];
    }
  }
  return bestPortATM;
}

int main() {
  bool wasBusy = false;
  int contRodadas = 0;
  int capacity = 0; //armazena capacidade

  char line[MAX_STR];   // dados temporários
  char myId[MAX_STR];   // identificador do bot em questão

  setbuf(stdin, NULL);   // stdin, stdout e stderr não terão buffers
  setbuf(stdout, NULL);  // assim, nada é "guardado temporariamente"
  setbuf(stderr, NULL);

  // === INÍCIO DA PARTIDA ===
  FILE * log;
  log = fopen("./log.txt", "a");
  fprintf(log, "NOVO JOGO\n");
  int h, w;
  scanf("AREA %i %i", &h, &w);  // lê a dimensão da área de pesca: altura (h) x largura (w)
  scanf(" ID %s", myId);        // ...e o id do bot

  Map map; //mapa do jogo
  Player player; //jogador

  while (1) {
    contRodadas++;
    fprintf(log, "RODADA %d\n", contRodadas);

    map = readMapData(h, w); //atualiza dados do mapa
    player = readPlayerData(myId); //atualiza dados do bot

    bool justSold = false;
    int xPos = player.curPosition.x;
    int yPos = player.curPosition.y;

    fprintf(log, "Coordenadas de %s: x %d, y %d\n",myId, yPos, xPos);
    fprintf(log, "Capacidade: %dkg\n", capacity);

    if (capacity > 9)
      player.haveGoal = true;
    else player.haveGoal = false;

    if (player.haveGoal == true){

      player.goal = findBestPort(map, xPos, yPos);

      //não está alinhado na horizontal
      if (yPos != player.goal.y){
        findHorizontalPath(player.curPosition, player.goal, log, wasBusy);
      }
      //está alinhado na horizontal, mas não na vertical
      else if ((yPos == player.goal.y) && (xPos != player.goal.x)){ 
        findVerticalPath(player.curPosition, player.goal, log, wasBusy);
      }
      //está no porto, pode vender!
      else if (yPos == player.goal.y && xPos == player.goal.x){
        if (capacity > 0) {
          printf("SELL\n");
          capacity = 0;
          justSold = true; //impede que um movimento seja efetuado nesse turno
          player.haveGoal = false;
        }
      }
    }

    if (player.haveGoal == false && justSold == false){
      movimento(xPos, yPos, map.area, w, h, &capacity, wasBusy);
    }

    scanf("%s", line);

    fprintf(log, "Retorno do OCMA: %s\n", line);
    fprintf(log, "\n");

    if (strcmp(line, "BUSY") == 0){
      wasBusy = true;
    }
    if (strcmp(line, "DONE") == 0){
      wasBusy = false;
    }
    if (strcmp(line, "MULLET") == 0 || strcmp(line, "SNAPPER") == 0 || strcmp(line, "SEABASS") == 0){
        capacity++;
    }
  }
  fclose(log);

  return 0;
}
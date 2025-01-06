
typedef struct {
  int maxX;
  int maxY;
  float pVpred; // 80
  float pVzad; //30
  float pVlavo; //56
  float pVpravo; //21
  int x;
  int y;
  int k;
  int reps;
//  pthread_mutex_t mutex;
  int ** mapa;
  float **statPocetKrokov;
  float **dostalSaDoStredu;


}SIMPAM;

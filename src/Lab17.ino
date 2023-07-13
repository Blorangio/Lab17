#include "MQTT.h"

SYSTEM_THREAD(ENABLED);

#define NorthGo 0
#define NorthSlow 1
#define EastGo 2
#define EastSlow 3

#define GREEN 0
#define YELLOW 1
#define RED 2

int currentState = 0;
volatile int input = 0;

struct State
{
  int output;
  unsigned long waitTime;
  int nextState[4];
};

// Finate State Machine keys:

/*
  4 states:         OUTPUT              KEY
    NorthGo - North Green, East Red      0
    NorthSlow - North Yellow, East Red   1
    EastGo - North Red, East Green       2
    EastSlow - North Red, East Yellow    3
*/

/*
  Inputs:                  KEY
    North High, East Low:   0
    North High, East High:  1
    North Low, East Low:    2
    North Low, East High:   3
*/

void callback(char *topic, byte *payload, unsigned int length);

MQTT client("lab.thewcl.com", 1883, callback);

typedef struct State State_t;

State_t fsm[4] =
    {
        {NorthGo, 5000, {NorthGo, EastSlow, NorthGo, EastSlow}},
        {NorthSlow, 5000, {NorthGo, NorthGo, NorthGo, NorthGo}},
        {EastGo, 5000, {EastGo, EastGo, NorthSlow, NorthSlow}},
        {EastSlow, 5000, {EastGo, EastGo, EastGo, EastGo}}};

void setup()
{
  client.subscribe("FSMfentonOutput");
}
void loop()
{
  if (client.isConnected())
  {
    client.loop();
  }
  else
  {
    client.connect(System.deviceID());
    client.subscribe("FSMfentonOutput");
  }
  delay(fsm[currentState].waitTime);
  currentState = fsm[currentState].nextState[input];
  client.publish("FSMfenton", (String)(fsm[currentState].output));
}

void callback(char *topic, byte *payload, unsigned int length)
{
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;

  input = atoi(p);
}
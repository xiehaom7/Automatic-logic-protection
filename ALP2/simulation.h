#pragma once
#include "simulation.h"
#include "config.h"

typedef enum { RANDOM, SEQUENCE, RESET } Gen_mode;
typedef enum { SA1, SA0, FLIP } Fault_mode;

typedef struct {
	unsigned				uLogicOne;
	unsigned				uLogicZero;
	unsigned				uInjection;
	unsigned				uPropagation;
	unsigned				uSimulation;
	unsigned				uAffection;
}StatNode;

class simulation
{
public:
	simulation();
	~simulation();
};


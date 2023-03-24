#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"
using namespace std;

struct State
{
	int row;
	int col;
	Orientacion orientation;
	bool well_situated;
	bool has_bikini;
	bool has_sneakers;
};

class ComportamientoJugador : public Comportamiento
{
public:
	ComportamientoJugador(unsigned int size) : Comportamiento(size)
	{
		current_state.row = current_state.col = 99;
		current_state.orientation = Orientacion::norte;
		current_state.well_situated = false;
		current_state.has_bikini = current_state.has_sneakers = false;

		last_action = actIDLE;

		target_found = false;

		// Inicializar precipicio mapaResultado
		initPrecipiceLimit();
		// Inicializar mapa auxiliar
		initMap(map, 2*size, '?');
	}

	ComportamientoJugador(const ComportamientoJugador &comport) : Comportamiento(comport) {}
	~ComportamientoJugador() {}

	Action think(Sensores sensors);
	int interact(Action accion, int valor);

private:

	// ...............................................................
	State current_state;
	Action last_action;
	bool target_found;

	vector<vector<unsigned char>> map;					// Mapa auxiliar

	// ...............................................................
	void initPrecipiceLimit();
	void initMap(vector<vector<unsigned char>> &map, int size, unsigned char value);

	void updateState(const Sensores &sensors);
	void updateCurrentState();
	void updateMapaResultado(const Sensores &sensors);
	
	void vision(vector<vector<unsigned char>> & mapa, Sensores sensores);
	int targetInVision(const Sensores &sensors, unsigned char target);
	
	Action move(Sensores sensors);
	void moveToTarget(const Sensores &sensors, unsigned char target);

	int batteryCostForward(unsigned char cell);
	int batteryCostTurnSL_SR(unsigned char cell);
	int batteryCostTurnBL_BR(unsigned char cell);

	
};

#endif

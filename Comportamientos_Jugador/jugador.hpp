#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include <cmath>

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

		move_left = (last_action == actTURN_BL || last_action == actTURN_SL);
		move_right = (last_action == actTURN_BR || last_action == actTURN_SR);
		move_forward = last_action == actFORWARD;

		// Inicializar precipicio mapaResultado
		initPrecipiceLimit();

		// Inicializar mapa auxiliar
		initMap(map, 2*size, (unsigned char)'?');

		// Inicializar mapa visitas
		initMap(cell_visits, 2*size, (int)0);

		// Inicializar mapa potenciales
		initMap(potencial_map, 2*size, (double)0.0);

		// Inicializar mapa coste bateria
		initMap(battery_cost_map, 2*size, (int)0);

	}

	ComportamientoJugador(const ComportamientoJugador &comport) : Comportamiento(comport) {}
	~ComportamientoJugador() {}

	Action think(Sensores sensors);
	int interact(Action accion, int valor);

private:

	// ...................... VARIABLES .............................

	const double VISIT_PENALTY_FACTOR = 1.0;
	const double BATTERY_COST_FACTOR = 1.0;
	const double UNVISITED_ATTRACTION = 100.0;

	State current_state;
	Action last_action;

	bool target_found;

	bool move_left;
	bool move_right;
	bool move_forward;

	vector<vector<unsigned char>> map;					// Mapa auxiliar
	vector<vector<int>> cell_visits;						// Mapa visitas
	vector<vector<double>> potencial_map;				// Mapa potenciales
	vector<vector<int>> battery_cost_map;				// Mapa coste bateria

	// ...................... FUNCIONES .............................

	void initPrecipiceLimit();
	
	template <typename T> void initMap(vector<vector<T>> &map, int size, T value);
	template <typename T> void fillMap(vector<vector<T>> &map, T value);

	void updateState(const Sensores &sensors);
	void updatePositionOrientation();
	void updateMapaResultado(const Sensores &sensors);
	
	vector<vector<double>> calculate_potencials(const vector<vector<int>> &map, const vector<vector<int>> visits_cell, const vector<vector<int>> battery_cost);

	void vision(vector<vector<unsigned char>> & mapa, Sensores sensores);
	int targetInVision(const Sensores &sensors, unsigned char target);
	
	Action move(Sensores sensors);
	void moveToTarget(const Sensores &sensors, unsigned char target);

	int batteryCostForward(unsigned char cell);
	int batteryCostTurnSL_SR(unsigned char cell);
	int batteryCostTurnBL_BR(unsigned char cell);

	
};

#endif

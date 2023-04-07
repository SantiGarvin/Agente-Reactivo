#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include <vector>
#include <cmath>
#include <limits>

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

struct BatteryCost
{
	int forward;
	int turnSL_SR;
	int turnBL_BR;
};

struct MapCell
{
	pair<int, int> position;
	unsigned char terrain_type;
	unsigned char entity_type;

	int times_visited;
	BatteryCost battery_cost;
	double potential;
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

		wall_detected = false;

		last_action = actIDLE;
		reset_counter = 0;
		loop_counter = 0;

		// Inicializar precipicio mapaResultado
		initPrecipiceLimit();

		// Reservar memoria para mapa auxiliar
		map.resize(2 * MAX_SIZE_MAP, vector<MapCell>(2 * MAX_SIZE_MAP));

		// Inicializar mapa auxiliar
		initMap(2 * MAX_SIZE_MAP);
	}

	ComportamientoJugador(const ComportamientoJugador &comport) : Comportamiento(comport) {}
	~ComportamientoJugador() {}

	Action think(Sensores sensors);
	int interact(Action accion, int valor);

private:
	// ...................... CONSTANTES .............................

	const int MAX_SIZE_MAP = 100;

	// ATRACCION
	const double ATTRACTION_TARGET_CELL = 10000;
	const double ATTRACTION_UNVISITED_CELL = 1000;

	// REPULSION
	const double PENALTY_WALL_PRECIPICE = -1000000;

	const double PENALTY_VISIT_FACTOR = 5.0;
	const double PENALTY_BATTERY_COST_FACTOR = 2.0;


	const int LOOP_DETECTION_THRESHOLD = 5;

	// ...................... VARIABLES .............................

	State current_state;
	Action last_action;

	int reset_counter;
	int loop_counter;

	bool move_left;
	bool move_right;
	bool move_forward;

	bool wall_detected;

	////// debug
	int counter = 0;

	// ...................... MAPA .............................

	vector<vector<MapCell>> map; // Mapa auxiliar

	vector<MapCell> vision;
	vector<vector<MapCell>> local_area;
	vector<pair<int, int>> position_history;

	// ...................... FUNCIONES .............................

	void initPrecipiceLimit();
	void initMap(int size);

	void updateState(const Sensores &sensors);
	void updatePositionOrientation();

	void updateTerrain(MapCell &cell, unsigned char terrain_type = '?');

	int batteryCostForward(unsigned char cell);
	int batteryCostTurnSL_SR(unsigned char cell);
	int batteryCostTurnBL_BR(unsigned char cell);

	int worstBatteryCost(BatteryCost battery_cost);

	void updateBatteryCost(MapCell &cell);
	void updatePotential(MapCell &cell);

	void updateMapWithVision(vector<vector<MapCell>> &mapa, Sensores sensores, bool update_mapaResultado = false);
	void updateMap();
	void recenterMap(vector<vector<MapCell>> &original_map, int size, const int row_offset, const int col_offset);

	vector<vector<MapCell>> getLocalArea(int size);

	void updatePositionHistory();
	bool isLooping();

	// bool wallPrecipiceAround(int size);

	Action move(Sensores sensors);
};

#endif

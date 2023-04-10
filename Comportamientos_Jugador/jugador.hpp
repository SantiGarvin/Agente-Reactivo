#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

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

		turn_counter = 0;

		last_action = actIDLE;
		reset_counter = 0;
		loop_counter = 0;

		second_turn_pending = false;
		battery_charged = false;

		// Inicializar precipicio mapaResultado
		initPrecipiceLimit();

		// Reservar memoria para mapa auxiliar
		map.resize(2 * MAX_SIZE_MAP, vector<MapCell>(2 * MAX_SIZE_MAP));

		// Inicializar mapa auxiliar
		initMap(map);
	}

	ComportamientoJugador(const ComportamientoJugador &comport) : Comportamiento(comport) {}
	~ComportamientoJugador() {}

	Action think(Sensores sensors);
	int interact(Action accion, int valor);

private:
	// ...................... CONSTANTES .............................

	const int MAX_SIZE_MAP = 100;
	const int TOTAL_BATTERY = 5000;
	const int MAX_BATTERY_CHARGE = 20;
	const int LOOP_DETECTION_THRESHOLD = 5;

	// ATRACCION
	const double ATTRACTION_TARGET_CELL = 10000;
	const double ATTRACTION_UNVISITED_CELL = 5000;

	// REPULSION
	const double PENALTY_WALL_PRECIPICE = -1000000;
	const double PENALTY_VILLAGER_WOLF = -1000000;
	const double PENALTY_BIKINI_SNEAKERS = -1000000;

	const double PENALTY_VISIT_FACTOR = 100.0;
	const double PENALTY_BATTERY_COST_FACTOR = 100.0;

	// ...................... VARIABLES .............................

	State current_state;
	Action last_action;

	int reset_counter;
	int loop_counter;

	int turn_counter;
	bool second_turn_pending;

	bool battery_charged;

	////// debug
	int counter = 0;

	// ...................... MAPA .............................

	vector<vector<MapCell>> map; // Mapa auxiliar

	vector<MapCell> vision;
	vector<vector<MapCell>> local_area;
	vector<pair<int, int>> position_history;

	// ...................... FUNCIONES .............................

	void initPrecipiceLimit();
	void initMap(vector<vector<MapCell>> &_map);

	void updateState(const Sensores &sensors);
	void updatePositionOrientation();

	int batteryCostForward(unsigned char cell);
	int batteryCostTurnSL_SR(unsigned char cell);
	int batteryCostTurnBL_BR(unsigned char cell);

	int worstBatteryCost(BatteryCost battery_cost);

	void updateTerrain(MapCell &cell, unsigned char terrain_type = '?');
	void updateEntity(MapCell &cell, unsigned char entity_type = '_');
	void updatePosition(MapCell &cell, int row, int col);
	void updateBatteryCost(MapCell &cell);
	void updatePotential(MapCell &cell, const Sensores &sensors);

	void updateMap(const Sensores &sensors);
	void updateResultMap(vector<vector<MapCell>> &aux_map, int row_offset, int col_offset, Orientacion orientation);
	void updateMapWithVision(vector<vector<MapCell>> &_map, const Sensores &sensors, bool update_mapaResultado = false);
	
	void applyOffset(vector<vector<MapCell>> &original_map, int row_offset, int col_offset);

	vector<vector<MapCell>> getLocalArea(int size);

	void updatePositionHistory();
	bool isLooping();

	Action followPotential();
	Action followRightWall();
	Action move();

	bool wallDetected();
};

#endif

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

		last_action = actIDLE;
		target_found = false;
		move_left = move_right = move_forward = false;
		reset_counter = 0;

		// Inicializar precipicio mapaResultado
		initPrecipiceLimit();

		map.resize(2 * MAX_SIZE_MAP, vector<MapCell>(2 * MAX_SIZE_MAP));

		// Inicializar mapas auxiliares
		initMap(2 * MAX_SIZE_MAP);
	}

	ComportamientoJugador(const ComportamientoJugador &comport) : Comportamiento(comport) {}
	~ComportamientoJugador() {}

	Action think(Sensores sensors);
	int interact(Action accion, int valor);

private:
	// ...................... CONSTANTES .............................

	const double VISIT_PENALTY_FACTOR = 1.0;
	const double BATTERY_COST_FACTOR = 1.0;
	const double UNVISITED_ATTRACTION = 100.0;

	const double TARGET_ATTRACTION = 100000.0;
	const double UNKNOWN_CELL_ATTRACTION = 0;

	const double WALL_PRECIPICE_PENALTY = -100000.0;

	const int MAX_INT = numeric_limits<int>::max();
	const int MIN_INT = numeric_limits<int>::min();

	const int MAX_SIZE_MAP = 100;

	// ...................... VARIABLES .............................

	State current_state;
	Action last_action;

	int reset_counter;

	bool target_found;

	bool move_left;
	bool move_right;
	bool move_forward;

	vector<vector<MapCell>> map; // Mapa auxiliar
	// vector<vector<int>> cell_visit_map;				// Mapa visitas
	// vector<vector<BatteryCost>> battery_cost_map; 	// Mapa coste bateria
	// vector<vector<double>> potential_map; 			// Mapa potenciales

	vector<double> potential;
	// ................. FUNCIONES PLANTILLA .........................

	template <typename T>
	void recenterMap(vector<vector<T>> &original_map, int size, const int row_offset, const int col_offset)
	{
		vector<vector<T>> new_map(size, vector<T>(size));

		for (int i = 0; i < size; ++i)
		{
			for (int j = 0; j < size; ++j)
			{
				int new_row = i + row_offset;
				int new_col = j + col_offset;

				if (new_row >= 0 && new_row < size && new_col >= 0 && new_col < size)
					new_map[new_row][new_col] = original_map[i][j];
			}
		}

		original_map = new_map;
	}

	// ...................... FUNCIONES .............................

	void initPrecipiceLimit();
	void initMap(int size);

	void updateState(const Sensores &sensors);
	void updatePositionOrientation();
	// void updateMapaResultado(const Sensores &sensors);

	BatteryCost batteryCost(unsigned char cell);
	int bestBatteryCost(BatteryCost battery_cost);

	vector<vector<double>> potentials(const vector<vector<MapCell>> &_map);

	vector<MapCell> updateMapWithVision(vector<vector<MapCell>> &mapa, Sensores sensores, bool update_mapaResultado = false);
	int targetInVision(const Sensores &sensors, unsigned char target);

	Action move(Sensores sensors);
	void moveToTarget(const Sensores &sensors, unsigned char target);

	int batteryCostForward(unsigned char cell);
	int batteryCostTurnSL_SR(unsigned char cell);
	int batteryCostTurnBL_BR(unsigned char cell);
};

#endif

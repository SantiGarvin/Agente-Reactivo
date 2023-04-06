#include "../Comportamientos_Jugador/jugador.hpp"
#include <iostream>

using namespace std;

Action ComportamientoJugador::think(Sensores sensors)
{
	Action action = actIDLE;

	// cout << "Posicion: fila " << sensors.posF << " columna " << sensors.posC << " ";
	// switch (sensors.sentido)
	// {
	// case 0:
	// 	cout << "Norte" << endl;
	// 	break;
	// case 1:
	// 	cout << "Noreste" << endl;
	// 	break;
	// case 2:
	// 	cout << "Este" << endl;
	// 	break;
	// case 3:
	// 	cout << "Sureste" << endl;
	// 	break;
	// case 4:
	// 	cout << "Sur " << endl;
	// 	break;
	// case 5:
	// 	cout << "Suroeste" << endl;
	// 	break;
	// case 6:
	// 	cout << "Oeste" << endl;
	// 	break;
	// case 7:
	// 	cout << "Noroeste" << endl;
	// 	break;
	// }
	// cout << "Terreno: ";
	// for (int i = 0; i < sensors.terreno.size(); i++)
	// 	cout << sensors.terreno[i];
	// cout << endl;

	// cout << "Superficie: ";
	// for (int i = 0; i < sensors.superficie.size(); i++)
	// 	cout << sensors.superficie[i];
	// cout << endl;

	// cout << "Colisión: " << sensors.colision << endl;
	// cout << "Reset: " << sensors.reset << endl;
	// cout << "Vida: " << sensors.vida << endl << endl;

	updateState(sensors);

	if (current_state.well_situated)
		updateMapWithVision(map, sensors, true);
	else
		updateMapWithVision(map, sensors);

	// Decide la accion a realizar basado en el estado actual
	action = move(sensors);

	// Guarda la ultima accion realizada
	last_action = action;

	// move_left = (last_action == actTURN_BL || last_action == actTURN_SL);
	// move_right = (last_action == actTURN_BR || last_action == actTURN_SR);
	// move_forward = last_action == actFORWARD;

	return action;
}

void ComportamientoJugador::initPrecipiceLimit()
{
	const int TAM_PRECIP = 3;

	for (int i = 0; i < mapaResultado.size(); i++)
	{
		for (int j = 0; j < TAM_PRECIP; j++)
		{
			mapaResultado[i][j] = 'P';
			mapaResultado[j][i] = 'P';
			mapaResultado[i][mapaResultado.size() - j - 1] = 'P';
			mapaResultado[mapaResultado.size() - j - 1][i] = 'P';
		}
	}
}

void ComportamientoJugador::initMap(int size)
{
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			map[i][j].position = make_pair(i, j);
			map[i][j].terrain_type = '?';
			map[i][j].entity_type = '?';
			map[i][j].times_visited = 0;
			map[i][j].battery_cost = {0, 0, 0};
			map[i][j].potential = 0.0;
		}
	}
}

void ComportamientoJugador::updateState(const Sensores &sensors)
{
	updatePositionOrientation();

	// Reinicio de la simulacion
	if (sensors.reset)
	{
		if (sensors.nivel != 0)
		{
			current_state = {99, 99, Orientacion::norte, false, false, false};
		}
		else
		{
			current_state = {sensors.posF, sensors.posC, sensors.sentido, true, false, false};
		}

		initMap(2 * MAX_SIZE_MAP);
		reset_counter++;
		last_action = actIDLE;
	}

	// Nivel 0
	if (sensors.nivel == 0)
	{
		current_state.well_situated = true;
		current_state.row = sensors.posF;
		current_state.col = sensors.posC;
		current_state.orientation = sensors.sentido;
	}

	// Si no esta bien situado y cae en una casilla de posicionamiento, se actualiza la posicion
	if (sensors.terreno[0] == 'G' and !current_state.well_situated)
	{
		int row_offset = current_state.row - sensors.posF;
		int col_offset = current_state.col - sensors.posC;

		current_state.row = sensors.posF;
		current_state.col = sensors.posC;
		current_state.orientation = sensors.sentido;
		current_state.well_situated = true;

		// Copia del mapa de la simulacion al mapa de la practica
		recenterMap(map, mapaResultado.size(), row_offset, col_offset);
	}

	if (sensors.terreno[0] == 'K')
		current_state.has_bikini = true;

	if (sensors.terreno[0] == 'D')
		current_state.has_sneakers = true;
}

void ComportamientoJugador::updatePositionOrientation()
{
	if (last_action != actIDLE)
	{
		switch (last_action)
		{
		case actFORWARD:
			switch (current_state.orientation)
			{
			case norte:
				current_state.row--;
				break;
			case noreste:
				current_state.row--;
				current_state.col++;
				break;
			case este:
				current_state.col++;
				break;
			case sureste:
				current_state.row++;
				current_state.col++;
				break;
			case sur:
				current_state.row++;
				break;
			case suroeste:
				current_state.row++;
				current_state.col--;
				break;
			case oeste:
				current_state.col--;
				break;
			case noroeste:
				current_state.row--;
				current_state.col--;
				break;
			}
			break;
		case actTURN_SL: // giro a la izquierda 45 grados
			current_state.orientation = static_cast<Orientacion>((current_state.orientation + 7) % 8);
			break;
		case actTURN_SR: // giro a la derecha 45 grados
			current_state.orientation = static_cast<Orientacion>((current_state.orientation + 1) % 8);
			break;
		case actTURN_BL: // giro a la izquierda 135 grados
			current_state.orientation = static_cast<Orientacion>((current_state.orientation + 6) % 8);
			break;
		case actTURN_BR: // giro a la derecha 135 grados
			current_state.orientation = static_cast<Orientacion>((current_state.orientation + 2) % 8);
			break;
		}
	}
}

// void ComportamientoJugador::updateMapaResultado(const Sensores &sensors)
// {
// 	if (current_state.well_situated)
// 	{
// 		int row_offset = current_state.row - sensors.posF;
// 		int col_offset = current_state.col - sensors.posC;

// 		for (int i = 0; i < mapaResultado.size(); i++)
// 		{
// 			for (int j = 0; j < mapaResultado.size(); j++)
// 			{
// 				if (mapaResultado[i][j] == '?')
// 				{
// 					mapaResultado[i][j] = map[row_offset + i][col_offset + j];
// 				}
// 			}
// 		}
// 	}
// }

void ComportamientoJugador::updateTerrain(MapCell &cell, unsigned char terrain_type)
{
	cell.terrain_type = terrain_type;
}

void ComportamientoJugador::updateBatteryCost(MapCell &cell)
{
	if (cell.terrain_type != '?')
	{
		cell.battery_cost.forward = batteryCostForward(cell.terrain_type);
		cell.battery_cost.turnSL_SR = batteryCostTurnSL_SR(cell.terrain_type);
		cell.battery_cost.turnBL_BR = batteryCostTurnBL_BR(cell.terrain_type);
	}
}

int ComportamientoJugador::worstBatteryCost(BatteryCost battery_cost)
{
	return max(max(battery_cost.forward, battery_cost.turnSL_SR), battery_cost.turnBL_BR);
}

void ComportamientoJugador::updatePotential(MapCell &cell)
{
	double attraction = 0;

	if (cell.terrain_type == '?')
		attraction = ATTRACTION_UNKNOWN_CELL;
	else if (cell.terrain_type == 'M' || cell.terrain_type == 'P')
		attraction = PENALTY_WALL_PRECIPICE;
	else if ((cell.terrain_type == 'K' && !current_state.has_bikini) || (cell.terrain_type == 'D' && !current_state.has_sneakers))
		attraction = ATTRACTION_TARGET_CELL;
	else
	{
		double visit_penalty = log(1 + cell.times_visited) * PENALTY_VISIT_FACTOR;
		double battery_cost_penalty = worstBatteryCost(cell.battery_cost) * PENALTY_BATTERY_COST_FACTOR;

		if (cell.times_visited > 0)
			attraction = -visit_penalty - battery_cost_penalty;
		else
			attraction = ATTRACTION_UNVISITED_CELL - battery_cost_penalty;
	}
	cell.potential = attraction;
}

Action ComportamientoJugador::move(Sensores sensors)
{
	Action action = actIDLE;

	double left_potential = vision[1].potential;
	double front_potential = vision[2].potential;
	double right_potential = vision[3].potential;

	updatePositionHistory();

	if (isLooping())
	{
		action = actTURN_BL;
	}
	else
	{
		if (front_potential >= left_potential && front_potential >= right_potential)
		{
			if (front_potential > 0)
				action = actFORWARD;
			else
				action = actTURN_BL;
		}
		else if (left_potential >= right_potential)
			action = actTURN_SL;
		else if (right_potential > left_potential)
			action = actTURN_SR;
		else
			action = actTURN_BL;
	}
	return action;
}

void ComportamientoJugador::updateMapWithVision(vector<vector<MapCell>> &_map, Sensores sensores, bool update_mapaResultado)
{
	vector<MapCell> cells;

	int row = current_state.row;
	int col = current_state.col;
	int index = 0;

	// Actualiza el mapa de terreno
	updateTerrain(_map[row][col], sensores.terreno[index]);

	if (update_mapaResultado)
		mapaResultado[row][col] = sensores.terreno[index];

	// Actualiza el mapa de visitas
	_map[row][col].times_visited++;

	// Aniade la celda actual a la lista de celdas
	cells.push_back(_map[row][col]);

	const int DIM = 4; // dimension de la vision
	switch (sensores.sentido)
	{
	case 0: // vision Norte
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row - i;
				int col = current_state.col + j;

				index++;

				if (update_mapaResultado)
					mapaResultado[row][col] = sensores.terreno[index];

				updateTerrain(_map[row][col], sensores.terreno[index]);

				cells.push_back(_map[row][col]);
			}
		}
		break;
	case 1: // vision Noreste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row;
				int col = current_state.col;

				if (j <= 0)
				{
					row -= i;
					col += i + j;
				}
				else
				{
					row += j - i;
					col += i;
				}
				index++;

				if (update_mapaResultado)
					mapaResultado[row][col] = sensores.terreno[index];

				updateTerrain(_map[row][col], sensores.terreno[index]);

				cells.push_back(_map[row][col]);
			}
		}
		break;
	case 2: // vision Este
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row + j;
				int col = current_state.col + i;

				index++;

				if (update_mapaResultado)
					mapaResultado[row][col] = sensores.terreno[index];

				updateTerrain(_map[row][col], sensores.terreno[index]);

				cells.push_back(_map[row][col]);
			}
		}
		break;
	case 3: // vision Sureste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row;
				int col = current_state.col;

				if (j <= 0)
				{
					row += i + j;
					col += i;
				}
				else
				{
					row += i;
					col += i - j;
				}
				index++;

				if (update_mapaResultado)
					mapaResultado[row][col] = sensores.terreno[index];

				updateTerrain(_map[row][col], sensores.terreno[index]);

				cells.push_back(_map[row][col]);
			}
		}
		break;
	case 4: // vision Sur
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row + i;
				int col = current_state.col - j;

				index++;

				if (update_mapaResultado)
					mapaResultado[row][col] = sensores.terreno[index];

				updateTerrain(_map[row][col], sensores.terreno[index]);

				cells.push_back(_map[row][col]);
			}
		}
		break;
	case 5: // vision Suroeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row;
				int col = current_state.col;

				if (j <= 0)
				{
					row += i;
					col -= i + j;
				}
				else
				{
					row -= j - i;
					col -= i;
				}
				index++;

				if (update_mapaResultado)
					mapaResultado[row][col] = sensores.terreno[index];

				updateTerrain(_map[row][col], sensores.terreno[index]);

				cells.push_back(_map[row][col]);
			}
		}
		break;
	case 6: // vision Oeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row - j;
				int col = current_state.col - i;

				index++;

				if (update_mapaResultado)
					mapaResultado[row][col] = sensores.terreno[index];

				updateTerrain(_map[row][col], sensores.terreno[index]);

				cells.push_back(_map[row][col]);
			}
		}
		break;
	case 7: // vision Noroeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row;
				int col = current_state.col;

				if (j <= 0)
				{
					row -= j + i;
					col -= i;
				}
				else
				{
					row -= i;
					col += j - i;
				}
				index++;

				if (update_mapaResultado)
					mapaResultado[row][col] = sensores.terreno[index];

				updateTerrain(_map[row][col], sensores.terreno[index]);

				cells.push_back(_map[row][col]);
			}
		}
		break;
	}

	updateMap();

	// Obtenemos el area local del agente de dimension 7x7
	local_area = getLocalArea(3);

	// Actualizar el vector de celdas del campo de vision
	vision = cells;
}

void ComportamientoJugador::updateMap()
{
	for (int i = 0; i < map.size(); ++i)
	{
		for (int j = 0; j < map[i].size(); ++j)
		{
			if (map[i][j].terrain_type != '?')
				updateBatteryCost(map[i][j]);

			updatePotential(map[i][j]);
		}
	}
}

void ComportamientoJugador::recenterMap(vector<vector<MapCell>> &original_map, int size, const int row_offset, const int col_offset)
{
	vector<vector<MapCell>> map; // Mapa auxiliar

	vector<MapCell> vision;
	vector<pair<int, int>> position_history;

	{
		vector<vector<MapCell>> new_map(size, vector<MapCell>(size));

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
}

vector<vector<MapCell>> ComportamientoJugador::getLocalArea(int size)
{
	if (0 < size && size < 4)
	{
		// Obtener las coordenadas actuales del agente
		int currentX = current_state.row;
		int currentY = current_state.col;

		// Crear un vector de tamaño (2*size + 1) x (2*size + 1) con las casillas alrededor del agente
		int areaSize = 2 * size + 1;
		vector<vector<MapCell>> local_area(areaSize, vector<MapCell>(areaSize));

		for (int i = -size; i <= size; ++i)
		{
			for (int j = -size; j <= size; ++j)
			{
				int x = currentX + i;
				int y = currentY + j;

				// Verificar que las coordenadas estén dentro del mapa
				if (x >= 0 && x < 2*MAX_SIZE_MAP && y >= 0 && y < 2*MAX_SIZE_MAP)
					local_area[i + size][j + size] = map[y][x];
			}
		}
		return local_area;
	}
	else
		return vector<vector<MapCell>>(0, vector<MapCell>(0));
}

int ComportamientoJugador::batteryCostForward(unsigned char cell)
{
	switch (cell)
	{
	case 'A':
		return current_state.has_bikini ? 5 : 200;
	case 'B':
		return current_state.has_sneakers ? 1 : 100;
	case 'T':
		return 2;
	case 'P':
	case 'M':
	case '?':
		return 0;
	default:
		return 1;
	}
}

int ComportamientoJugador::batteryCostTurnSL_SR(unsigned char cell)
{
	switch (cell)
	{
	case 'A':
		return current_state.has_bikini ? 5 : 500;
	case 'B':
		return current_state.has_sneakers ? 3 : 1;
	case 'T':
		return 2;
	case 'P':
	case 'M':
	case '?':
		return 0;
	default:
		return 1;
	}
}

int ComportamientoJugador::batteryCostTurnBL_BR(unsigned char cell)
{
	switch (cell)
	{
	case 'A':
		return current_state.has_bikini ? 5 : 50;
	case 'B':
		return current_state.has_sneakers ? 1 : 3;
	case 'T':
		return 2;
	case 'P':
	case 'M':
	case '?':
		return 0;
	default:
		return 1;
	}
}

void ComportamientoJugador::updatePositionHistory()
{
	position_history.push_back(vision[0].position);

	if (position_history.size() > LOOP_DETECTION_THRESHOLD)
		position_history.erase(position_history.begin());
}

bool ComportamientoJugador::isLooping()
{
	if (position_history.size() < LOOP_DETECTION_THRESHOLD)
		return false;

	for (int i = 0; i < position_history.size() - 1; i++)
	{
		if (position_history[i] == position_history[position_history.size() - 1])
			return true;
	}

	return false;
}

int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}

#include "../Comportamientos_Jugador/jugador.hpp"
#include <iostream>

using namespace std;

Action ComportamientoJugador::think(Sensores sensors)
{
	Action action = actIDLE;

	// Actualiza el estado del agente
	updateState(sensors);

	// Actualiza el mapa con la vision del agente
	if (current_state.well_situated)
		updateMapWithVision(map, sensors, true);
	else
		updateMapWithVision(map, sensors);

	// Decide la accion a realizar basado en el estado actual
	action = move();

	// Guarda la ultima accion realizada
	last_action = action;

	////////////////////////////////////////////////////////////////////////
	// DEBUG															  //
	////////////////////////////////////////////////////////////////////////
	debug(true);

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

void ComportamientoJugador::initMap(vector<vector<MapCell>> &_map)
{
	int size = _map.size();

	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			_map[i][j].position = make_pair(i, j);
			_map[i][j].terrain_type = '?';
			_map[i][j].entity_type = '_';
			_map[i][j].times_visited = 0;
			_map[i][j].battery_cost = {0, 0, 0};
			_map[i][j].potential = 0;
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
			current_state = {99, 99, Orientacion::norte, false, false, false};
		else
			current_state = {sensors.posF, sensors.posC, sensors.sentido, true, false, false};

		initMap(map);
		reset_counter++;
		last_action = actIDLE;
	}

	// Nivel 0
	if (sensors.nivel == 0)
	{
		current_state.row = sensors.posF;
		current_state.col = sensors.posC;
		current_state.well_situated = true;
		current_state.orientation = sensors.sentido;
	}

	// Si no esta bien situado y cae en una casilla de posicionamiento, se actualiza la posicion
	if (sensors.terreno[0] == 'G' and !current_state.well_situated)
	{
		int row_offset = current_state.row - sensors.posF;
		int col_offset = current_state.col - sensors.posC;

		current_state.row = sensors.posF;
		current_state.col = sensors.posC;
		current_state.well_situated = true;
		current_state.orientation = sensors.sentido;

		// // Copia del mapa de la simulacion al mapa de la practica
		// applyOffset(map, row_offset, col_offset);
	}

	if (sensors.terreno[0] == 'K')
		current_state.has_bikini = true;

	if (sensors.terreno[0] == 'D')
		current_state.has_sneakers = true;
}

void ComportamientoJugador::updatePositionOrientation()
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
		current_state.orientation = static_cast<Orientacion>((current_state.orientation + 5) % 8);
		break;
	case actTURN_BR: // giro a la derecha 135 grados
		current_state.orientation = static_cast<Orientacion>((current_state.orientation + 3) % 8);
		break;
	default:
		break;
	}
}

int ComportamientoJugador::worstBatteryCost(BatteryCost battery_cost)
{
	return max(max(battery_cost.forward, battery_cost.turnSL_SR), battery_cost.turnBL_BR);
}

void ComportamientoJugador::updateTerrain(MapCell &cell, unsigned char terrain_type)
{
	cell.terrain_type = terrain_type;
}

void ComportamientoJugador::updateEntity(MapCell &cell, unsigned char entity_type)
{
	cell.entity_type = entity_type;
}

void ComportamientoJugador::updatePosition(MapCell &cell, int row, int col)
{
	cell.position = make_pair(row, col);
}

void ComportamientoJugador::updateBatteryCost(MapCell &cell)
{
	cell.battery_cost.forward = batteryCostForward(cell.terrain_type);
	cell.battery_cost.turnSL_SR = batteryCostTurnSL_SR(cell.terrain_type);
	cell.battery_cost.turnBL_BR = batteryCostTurnBL_BR(cell.terrain_type);
}

void ComportamientoJugador::updatePotential(MapCell &cell, const Sensores &sensors)
{
	double attraction = 0;

	bool battery_charge = cell.terrain_type == 'X' && sensors.bateria < TOTAL_BATTERY * 0.05 && (200 <= sensors.vida && sensors.vida <= 300);
	bool target_sneakers_bikini = (cell.terrain_type == 'K' && !current_state.has_bikini) || (cell.terrain_type == 'D' && !current_state.has_sneakers);
	bool target_position = cell.terrain_type == 'G' && !current_state.well_situated;

	if (cell.entity_type == 'a' || cell.entity_type == 'l')
		attraction = PENALTY_VILLAGER_WOLF;
	else if (cell.terrain_type == 'M' || cell.terrain_type == 'P')
		attraction = PENALTY_WALL_PRECIPICE;
	else if ((cell.terrain_type == 'A' && !current_state.has_bikini) || (cell.terrain_type == 'B' && !current_state.has_sneakers))
		attraction = PENALTY_BIKINI_SNEAKERS;
	else if (battery_charge || target_sneakers_bikini || target_position)
		attraction = ATTRACTION_TARGET_CELL;
	else
	{
		double visit_penalty = log(1 + cell.times_visited) * PENALTY_VISIT_FACTOR;
		double battery_cost_penalty = worstBatteryCost(cell.battery_cost) * PENALTY_BATTERY_COST_FACTOR;

		if (cell.times_visited > 0)
			attraction -= visit_penalty;
		else
			attraction += ATTRACTION_UNVISITED_CELL;

		attraction -= battery_cost_penalty;
	}
	cell.potential = round(attraction);
}

Action ComportamientoJugador::move()
{
	Action action = actIDLE;

	// if (wallDetected())
	// 	action = followRightWall();
	// else
	action = followPotential();
	// action = actFORWARD;
	return action;
}

Action ComportamientoJugador::followPotential()
{
	Action action = actIDLE;

	double current_potential = vision[0].potential;
	double left_potential = vision[1].potential;
	double front_potential = vision[2].potential;
	double right_potential = vision[3].potential;

	if (front_potential == PENALTY_WALL_PRECIPICE && left_potential == PENALTY_WALL_PRECIPICE && right_potential == PENALTY_WALL_PRECIPICE)
		action = actTURN_BL;
	else
	{
		if (right_potential > front_potential && right_potential >= left_potential && right_potential >= current_potential)
			action = actTURN_SR;
		else if (front_potential >= right_potential && front_potential >= left_potential && front_potential >= current_potential)
			action = actFORWARD;
		else if (left_potential > front_potential && left_potential >= right_potential && left_potential >= current_potential)
			action = actTURN_SL;
		// else if(current_potential > front_potential && current_potential > left_potential && current_potential > right_potential)
		// 	action = actIDLE;
		else
			action = actTURN_BL;
	}

	return action;
}

Action ComportamientoJugador::followRightWall()
{
	Action action = actIDLE;

	bool wall_left = vision[1].terrain_type == 'M';
	bool wall_front = vision[2].terrain_type == 'M';
	bool wall_right = vision[3].terrain_type == 'M';

	bool wall_front_left = wall_front && wall_left;
	bool wall_front_right = wall_front && wall_right;
	bool wall_left_right = wall_left && wall_right;
	bool wall_left_front_right = wall_left && wall_front && wall_right;

	bool orientation_NSEW = static_cast<Orientacion>(current_state.orientation % 2 == 0);

	if (second_turn_pending)
	{
		if (last_action == actTURN_SL)
			action = actTURN_SL;
		else
			action = actTURN_SR;

		second_turn_pending = false;
	}
	else
	{
		if (orientation_NSEW)
		{
			if (wall_left_front_right)
			{
				action = actTURN_SL;
				second_turn_pending = true;
			}
			else if (wall_front_right)
				action = actTURN_SL;
			else if (wall_front && !wall_right)
				action = actTURN_SR;
			else if (wall_right)
			{
				action = actTURN_SL;
				second_turn_pending = true;
			}
			else
				action = actFORWARD;
		}
		else
		{
			if (wall_front_left)
				action = actTURN_SR;
			else if (wall_front && !wall_left)
				action = actTURN_SL;
			else if (wall_left)
			{
				action = actTURN_SR;
				second_turn_pending = true;
			}
			else
				action = actFORWARD;
		}
	}

	return action;
}

bool ComportamientoJugador::wallDetected()
{
	bool wall_left = vision[1].terrain_type == 'M';
	bool wall_front = vision[2].terrain_type == 'M';
	bool wall_right = vision[3].terrain_type == 'M';

	return wall_left || wall_front || wall_right;
}

void ComportamientoJugador::updateMapWithVision(vector<vector<MapCell>> &_map, const Sensores &sensors, bool update_mapaResultado)
{
	vector<MapCell> cells;

	int row = current_state.row;
	int col = current_state.col;
	int index = 0;

	// Actualiza el mapa de terreno
	updateTerrain(_map[row][col], sensors.terreno[index]);

	if (update_mapaResultado)
		mapaResultado[row][col] = sensors.terreno[index];

	// Actualiza el mapa de visitas
	_map[row][col].times_visited++;

	// Aniade la celda actual a la lista de celdas
	cells.push_back(_map[row][col]);

	index++;

	const int DIM = 4; // dimension de la vision
	switch (sensors.sentido)
	{
	case 0: // vision Norte
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row - i;
				int col = current_state.col + j;

				if (row >= 0 && row < _map.size() && col >= 0 && col < _map[0].size())
				{
					if (update_mapaResultado && mapaResultado[row][col] == '?')
						mapaResultado[row][col] = sensors.terreno[index];

					updateTerrain(_map[row][col], sensors.terreno[index]);
					updateEntity(_map[row][col], sensors.superficie[index]);

					index++;

					cells.push_back(_map[row][col]);
				}
			}
		}
		break;
	case 1: // vision Noreste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row - i;
				int col = current_state.col + i;

				if (j < 0)
				{
					col += j;
				}
				else if (j > 0)
				{
					row += j;
				}

				if (row >= 0 && row < _map.size() && col >= 0 && col < _map[0].size())
				{
					if (update_mapaResultado && mapaResultado[row][col] == '?')
						mapaResultado[row][col] = sensors.terreno[index];

					updateTerrain(_map[row][col], sensors.terreno[index]);
					updateEntity(_map[row][col], sensors.superficie[index]);

					index++;

					cells.push_back(_map[row][col]);
				}
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

				if (row >= 0 && row < _map.size() && col >= 0 && col < _map[0].size())
				{
					if (update_mapaResultado && mapaResultado[row][col] == '?')
						mapaResultado[row][col] = sensors.terreno[index];

					updateTerrain(_map[row][col], sensors.terreno[index]);
					updateEntity(_map[row][col], sensors.superficie[index]);

					index++;

					cells.push_back(_map[row][col]);
				}
			}
		}
		break;
	case 3: // vision Sureste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row + i;
				int col = current_state.col + i;

				if (j < 0)
				{
					row += j;
				}
				else if (j > 0)
				{
					col -= j;
				}

				if (row >= 0 && row < _map.size() && col >= 0 && col < _map[0].size())
				{
					if (update_mapaResultado && mapaResultado[row][col] == '?')
						mapaResultado[row][col] = sensors.terreno[index];

					updateTerrain(_map[row][col], sensors.terreno[index]);
					updateEntity(_map[row][col], sensors.superficie[index]);

					index++;

					cells.push_back(_map[row][col]);
				}
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

				if (row >= 0 && row < _map.size() && col >= 0 && col < _map[0].size())
				{
					if (update_mapaResultado && mapaResultado[row][col] == '?')
						mapaResultado[row][col] = sensors.terreno[index];

					updateTerrain(_map[row][col], sensors.terreno[index]);
					updateEntity(_map[row][col], sensors.superficie[index]);

					index++;

					cells.push_back(_map[row][col]);
				}
			}
		}
		break;
	case 5: // vision Suroeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row + i;
				int col = current_state.col - i;

				if (j < 0)
				{
					col -= j;
				}
				else if (j > 0)
				{
					row -= j;
				}

				if (row >= 0 && row < _map.size() && col >= 0 && col < _map[0].size())
				{
					if (update_mapaResultado && mapaResultado[row][col] == '?')
						mapaResultado[row][col] = sensors.terreno[index];

					updateTerrain(_map[row][col], sensors.terreno[index]);
					updateEntity(_map[row][col], sensors.superficie[index]);

					index++;

					cells.push_back(_map[row][col]);
				}
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

				if (row >= 0 && row < _map.size() && col >= 0 && col < _map[0].size())
				{
					if (update_mapaResultado && mapaResultado[row][col] == '?')
						mapaResultado[row][col] = sensors.terreno[index];

					updateTerrain(_map[row][col], sensors.terreno[index]);
					updateEntity(_map[row][col], sensors.superficie[index]);

					index++;

					cells.push_back(_map[row][col]);
				}
			}
		}
		break;
	case 7: // vision Noroeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int row = current_state.row - i;
				int col = current_state.col - i;

				if (j < 0)
				{
					row -= j;
				}
				else if (j > 0)
				{
					col += j;
				}

				if (row >= 0 && row < _map.size() && col >= 0 && col < _map[0].size())
				{
					if (update_mapaResultado && mapaResultado[row][col] == '?')
						mapaResultado[row][col] = sensors.terreno[index];

					updateTerrain(_map[row][col], sensors.terreno[index]);
					updateEntity(_map[row][col], sensors.superficie[index]);

					index++;

					cells.push_back(_map[row][col]);
				}
			}
		}
		break;
	}

	updateMap(sensors);
	updateVision(sensors);

	// Obtenemos el area local del agente de dimension 5x5
	local_area = getLocalArea(2);

	// Actualizar el vector de celdas del campo de vision
	vision = cells;
}

void ComportamientoJugador::updateVision(const Sensores &sensors)
{
	for (MapCell c : vision)
	{
		updateBatteryCost(c);
		updatePotential(c, sensors);
	}
}

void ComportamientoJugador::updateMap(const Sensores &sensors)
{
	for (int i = 0; i < map.size(); ++i)
	{
		for (int j = 0; j < map[i].size(); ++j)
		{
			updatePosition(map[i][j], i, j);
			updateBatteryCost(map[i][j]);
			updatePotential(map[i][j], sensors);
		}
	}
}

// void ComportamientoJugador::updateResultMap(vector<vector<MapCell>> &aux_map, int row_offset, int col_offset, Orientacion orientation)
// {
// 	int size = aux_map.size();

// 	for (int i = 0; i < size; ++i)
// 	{
// 		for (int j = 0; j < size; ++j)
// 		{
// 			int new_row = i;
// 			int new_col = j;

// 			switch (orientation)
// 			{
// 			case 0: // Norte
// 				new_row = i - row_offset;
// 				new_col = j - col_offset;
// 				break;
// 			case 1: // Noreste
// 				new_row = i - row_offset + col_offset;
// 				new_col = j - col_offset - row_offset;
// 				break;
// 			case 2: // Este
// 				new_row = i + col_offset;
// 				new_col = j - row_offset;
// 				break;
// 			case 3: // Sureste
// 				new_row = i + row_offset + col_offset;
// 				new_col = j - col_offset + row_offset;
// 				break;
// 			case 4: // Sur
// 				new_row = i + row_offset;
// 				new_col = j + col_offset;
// 				break;
// 			case 5: // Suroeste
// 				new_row = i + row_offset - col_offset;
// 				new_col = j + col_offset + row_offset;
// 				break;
// 			case 6: // Oeste
// 				new_row = i - col_offset;
// 				new_col = j + row_offset;
// 				break;
// 			case 7: // Noroeste
// 				new_row = i - row_offset - col_offset;
// 				new_col = j + col_offset - row_offset;
// 				break;
// 			default:
// 				new_row = i;
// 				new_col = j;
// 				break;
// 			}

// 			if (new_row >= 0 && new_row < size && new_col >= 0 && new_col < size)
// 			{
// 				mapaResultado[new_row][new_col] = static_cast<unsigned char>(aux_map[i][j].terrain_type);
// 			}
// 		}
// 	}
// }

void ComportamientoJugador::applyOffset(vector<vector<MapCell>> &_map, int row_offset, int col_offset)
{
	int size = _map.size() / 2;

	for (int i = 0; i < size; ++i)
	{
		int new_row = i + row_offset;
		for (int j = 0; j < size; ++j)
		{
			int new_col = j + col_offset;
			if (new_row >= 0 && new_row < size && new_col >= 0 && new_col < size)
			{
				if (mapaResultado[i][j] == '?')
					mapaResultado[i][j] = _map[new_row][new_col].terrain_type;
			}
		}
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
				if (x >= 0 && x < 2 * MAX_SIZE_MAP && y >= 0 && y < 2 * MAX_SIZE_MAP)
					local_area[i + size][j + size] = map[x][y];
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
	position_history.push_back(make_pair(current_state.row, current_state.col));

	if (position_history.size() > LOOP_DETECTION_THRESHOLD)
		position_history.erase(position_history.begin());
}

bool ComportamientoJugador::isLooping()
{
	if (position_history.size() < LOOP_DETECTION_THRESHOLD)
		return false;

	for (int i = 0; i < position_history.size(); i++)
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

int cellWidth(const MapCell &cell)
{
	stringstream cell_content;
	cell_content << "[" << cell.terrain_type << "," << cell.potential << "]";
	return cell_content.str().length();
}

string cellString(int index, const MapCell &cell)
{
	stringstream cell_content;
	cell_content << index << ":[" << cell.terrain_type << "," << cell.potential << "]";
	return cell_content.str();
}

void ComportamientoJugador::debug(bool debug, bool mapa)
{
	if (debug)
	{
		cout << flush;

		cout << "FILA: " << current_state.row << "   COL: " << current_state.col << endl;
		cout << "ORIENTACION: ";
		switch (current_state.orientation)
		{
		case 0:
			cout << "Norte";
			break;
		case 1:
			cout << "Noreste";
			break;
		case 2:
			cout << "Este";
			break;
		case 3:
			cout << "Sureste";
			break;
		case 4:
			cout << "Sur";
			break;
		case 5:
			cout << "Suroeste";
			break;
		case 6:
			cout << "Oeste";
			break;
		case 7:
			cout << "Noroeste";
			break;
		}
		cout << endl
			 << "LAST ACTION: ";
		switch (last_action)
		{
		case 0:
			cout << "actFORWARD";
			break;
		case 1:
			cout << "actTURN_SL";
			break;
		case 2:
			cout << "actTURN_SR";
			break;
		case 3:
			cout << "actTURN_BL";
			break;
		case 4:
			cout << "actTURN_BR";
			break;
		case 5:
			cout << "actIDLE";
			break;
		}

		cout << endl
			 << endl;
		cout << "Vision:" << endl;

		int row = 3;	// El número de filas en el triángulo invertido
		int spaces = 4; // El número de espacios entre las celdas

		for (int i = row - 1; i >= 0; i--)
		{
			// Calcular el índice de inicio y el número de celdas en la fila actual
			int start_index = i == 2 ? vision.size() - 5 : (i == 1 ? 3 : 0);
			int num_cells = i == 2 ? 5 : (i == 1 ? 3 : 1);

			// Calcular el ancho total de las celdas en la fila actual
			int total_width = 0;
			for (int j = 0; j < num_cells; j++)
			{
				total_width += cellString(start_index + j, vision[start_index + j]).length();
			}
			total_width += (num_cells - 1) * spaces;

			// Agregar espacios iniciales para centrar las filas
			if (i < 2)
			{
				int top_row_width = 0;
				for (int j = 0; j < 5; j++)
				{
					top_row_width += cellString(vision.size() - 5 + j, vision[vision.size() - 5 + j]).length();
				}
				top_row_width += 4 * spaces;

				int padding = (top_row_width - total_width) / 2;
				for (int j = 0; j < padding; j++)
				{
					cout << " ";
				}
			}

			// Agregar las celdas
			for (int j = 0; j < num_cells; j++)
			{
				int index = start_index + j;
				cout << cellString(index, vision[index]);

				// Agregar espacios entre las celdas
				if (j < num_cells - 1)
				{
					for (int k = 0; k < spaces; k++)
					{
						cout << " ";
					}
				}
			}

			// Imprimir la fila
			cout << endl << endl;
		}
		if (mapa)
		{
			cout << "Mapa Auxiliar:";
			cout << endl;
			cout << flush;
			int i = 0;
			for (int i = 0; i < 200; ++i)
			{
				for (int j = 0; j < 200; ++j)
				{
					MapCell c = map[i][j];
					if (c.position.first == current_state.row && c.position.second == current_state.col)
						cout << "O";
					else
						cout << "(" << c.position.first << "," << c.position.second << ") ";
					cout << c.terrain_type;
					if (c.position.second == 99)
						cout << " | ";
				}
				if (i == 99)
				{
					cout << endl;
					for (int i = 0; i < 200; i++)
						cout << "_";
				}
				cout << endl;
			}
		}
		else
		{
			cout << "Mapa Local:";
			for (int i = 0; i < local_area.size(); i++)
			{
				cout << endl
					 << endl;

				// Imprimir los tipos de casilla
				for (int j = 0; j < local_area[i].size(); j++)
				{
					// Calcular el ancho máximo de las celdas en la columna actual
					int max_width = 0;
					for (int k = 0; k < local_area.size(); k++)
					{
						max_width = max(max_width, cellWidth(local_area[k][j]));
					}

					// Incrementar el ancho máximo en 7 para dejar un espacio de 7 caracteres entre celdas
					max_width += 7;

					stringstream cell_content;
					if (i == 2 && j == 2)
						cell_content << "{";
					else
						cell_content << "[";
					cell_content << local_area[i][j].terrain_type << "," << local_area[i][j].potential;
					if (i == 2 && j == 2)
						cell_content << "}";
					else
						cell_content << "]";

					cout << setw(max_width) << left << cell_content.str();
				}

				cout << endl;

				// Imprimir las posiciones
				for (int j = 0; j < local_area[i].size(); j++)
				{
					// Calcular el ancho máximo de las celdas en la columna actual
					int max_width = 0;
					for (int k = 0; k < local_area.size(); k++)
					{
						max_width = max(max_width, cellWidth(local_area[k][j]));
					}

					// Incrementar el ancho máximo en 7 para dejar un espacio de 7 caracteres entre celdas
					max_width += 7;

					stringstream cell_content;
					cell_content << "(" << local_area[i][j].position.first << "," << local_area[i][j].position.second << ")";

					cout << setw(max_width) << left << cell_content.str();
				}

				cout << endl;
			}
		}
		cout << flush;
	}
}
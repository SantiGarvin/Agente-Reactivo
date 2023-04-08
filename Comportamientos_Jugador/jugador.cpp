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

	// Actualiza la posicion y orientacion del agente
	updatePositionOrientation();

	////////////////////////////////////////////////////////////////////////
	// DEBUG															  //
	////////////////////////////////////////////////////////////////////////

	cout << flush;

	cout << "VISION : ";
	for (int i = 0; i < vision.size() - 7; i++)
	{
		cout << i << ":[" << vision[i].terrain_type << "," << vision[i].potential << "]   ";
		if (i == 0 || i == 3)
			cout << "||   ";
	}

	cout << endl
		 << "ORIENTACION: ";
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

	for (int i = 0; i < local_area.size(); i++)
	{
		cout << endl
			 << endl;
		for (int j = 0; j < local_area[i].size(); j++)
		{
			cout << std::setw(15);
			if (i == 2 && j == 2)
				cout << "[[";
			cout << "[" << local_area[i][j].terrain_type << "," << local_area[i][j].potential << "]";
			if (i == 2 && j == 2)
				cout << "]]";
			cout << setw(15);
		}
		cout << endl;
	}
	cout << endl
		 << endl
		 << endl
		 << endl;
	cout << flush;

	////////////////////////////////////////////////////////////////////////
	// DEBUG															  //
	////////////////////////////////////////////////////////////////////////

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
			map[i][j].entity_type = '_';
			map[i][j].times_visited = 0;
			map[i][j].battery_cost = {0, 0, 0};
			map[i][j].potential = 0;
		}
	}
}

void ComportamientoJugador::updateState(const Sensores &sensors)
{
	// Reinicio de la simulacion
	if (sensors.reset)
	{
		if (sensors.nivel != 0)
			current_state = {99, 99, Orientacion::norte, false, false, false};
		else
			current_state = {sensors.posF, sensors.posC, sensors.sentido, true, false, false};

		initMap(2 * MAX_SIZE_MAP);
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
			current_state.orientation = static_cast<Orientacion>((current_state.orientation + 5) % 8);
			break;
		case actTURN_BR: // giro a la derecha 135 grados
			current_state.orientation = static_cast<Orientacion>((current_state.orientation + 3) % 8);
			break;
		}
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

void ComportamientoJugador::updateBatteryCost(MapCell &cell)
{
	cell.battery_cost.forward = batteryCostForward(cell.terrain_type);
	cell.battery_cost.turnSL_SR = batteryCostTurnSL_SR(cell.terrain_type);
	cell.battery_cost.turnBL_BR = batteryCostTurnBL_BR(cell.terrain_type);
}

void ComportamientoJugador::updatePotential(MapCell &cell, const Sensores &sensors)
{
	double attraction = 0;

	bool battery_charge = sensors.bateria < TOTAL_BATTERY * 0.05 && (200 <= sensors.vida && sensors.vida <= 300);
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

				index++;

				if (update_mapaResultado)
					mapaResultado[row][col] = sensors.terreno[index];

				updateEntity(_map[row][col], sensors.superficie[index]);
				updateTerrain(_map[row][col], sensors.terreno[index]);

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
					mapaResultado[row][col] = sensors.terreno[index];

				updateEntity(_map[row][col], sensors.superficie[index]);
				updateTerrain(_map[row][col], sensors.terreno[index]);

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
					mapaResultado[row][col] = sensors.terreno[index];

				updateEntity(_map[row][col], sensors.superficie[index]);
				updateTerrain(_map[row][col], sensors.terreno[index]);

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
					mapaResultado[row][col] = sensors.terreno[index];

				updateEntity(_map[row][col], sensors.superficie[index]);
				updateTerrain(_map[row][col], sensors.terreno[index]);

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
					mapaResultado[row][col] = sensors.terreno[index];

				updateEntity(_map[row][col], sensors.superficie[index]);
				updateTerrain(_map[row][col], sensors.terreno[index]);

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
					mapaResultado[row][col] = sensors.terreno[index];

				updateEntity(_map[row][col], sensors.superficie[index]);
				updateTerrain(_map[row][col], sensors.terreno[index]);

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
					mapaResultado[row][col] = sensors.terreno[index];

				updateEntity(_map[row][col], sensors.superficie[index]);
				updateTerrain(_map[row][col], sensors.terreno[index]);

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
					mapaResultado[row][col] = sensors.terreno[index];

				updateEntity(_map[row][col], sensors.superficie[index]);
				updateTerrain(_map[row][col], sensors.terreno[index]);

				cells.push_back(_map[row][col]);
			}
		}
		break;
	}

	updateMap(sensors);

	// Obtenemos el area local del agente de dimension 5x5
	local_area = getLocalArea(2);

	// Actualizar el vector de celdas del campo de vision
	vision = cells;
}

void ComportamientoJugador::updateMap(const Sensores &sensors)
{
	for (int i = 0; i < map.size(); ++i)
	{
		for (int j = 0; j < map[i].size(); ++j)
		{
			updateBatteryCost(map[i][j]);
			updatePotential(map[i][j], sensors);
		}
	}
}

void ComportamientoJugador::rotateMap(vector<vector<MapCell>> &original_map, int angle)
{
	angle = angle % 360;

	vector<vector<MapCell>> new_map(original_map.size(), vector<MapCell>(original_map.size()));

	for (int i = 0; i < original_map.size(); ++i)
	{
		for (int j = 0; j < original_map.size(); ++j)
		{
			int new_row = i;
			int new_col = j;

			switch (angle)
			{
			case 45:
				new_row = i + j;
				new_col = j - i;
				break;
			case 90:
				new_row = j;
				new_col = original_map.size() - i - 1;
				break;
			case 135:
				new_row = original_map.size() - i - 1 - j;
				new_col = original_map.size() - i - 1 + j;
				break;
			case 180:
				new_row = original_map.size() - i - 1;
				new_col = original_map.size() - j - 1;
				break;
			case 225:
				new_row = original_map.size() - i - 1 + j;
				new_col = i - j;
				break;
			case 270:
				new_row = original_map.size() - j - 1;
				new_col = i;
				break;
			case 315:
				new_row = i - j;
				new_col = i + j;
				break;
			default:
				new_row = i;
				new_col = j;
				break;
			}

			new_map[new_row][new_col] = original_map[i][j];
		}
	}

	original_map = new_map;

	vector<MapCell> new_vision;

	for (int i = 0; i < vision.size(); ++i)
	{
		int new_row = vision[i].position.first;
		int new_col = vision[i].position.second;

		switch (angle)
		{
		case 45:
			new_row = vision[i].position.first + vision[i].position.second;
			new_col = vision[i].position.second - vision[i].position.first;
			break;
		case 90:
			new_row = vision[i].position.second;
			new_col = original_map.size() - vision[i].position.first - 1;
			break;
		case 135:
			new_row = original_map.size() - vision[i].position.first - 1 - vision[i].position.second;
			new_col = original_map.size() - vision[i].position.first - 1 + vision[i].position.second;
			break;
		case 180:
			new_row = original_map.size() - vision[i].position.first - 1;
			new_col = original_map.size() - vision[i].position.second - 1;
			break;
		case 225:
			new_row = original_map.size() - vision[i].position.first - 1 + vision[i].position.second;
			new_col = vision[i].position.first - vision[i].position.second;
			break;
		case 270:
			new_row = original_map.size() - vision[i].position.second - 1;
			new_col = vision[i].position.first;
			break;
		case 315:
			new_row = vision[i].position.first - vision[i].position.second;
			new_col = vision[i].position.first + vision[i].position.second;
			break;
		default:
			new_row = vision[i].position.first;
			new_col = vision[i].position.second;
			break;
		}

		new_vision.push_back(map[new_row][new_col]);
	}

	vision = new_vision;

	vector<pair<int, int>> new_position_history;

	for (int i = 0; i < position_history.size(); ++i)
	{
		int new_row = position_history[i].first;
		int new_col = position_history[i].second;

		switch (angle)
		{
		case 45:
			new_row = position_history[i].first + position_history[i].second;
			new_col = position_history[i].second - position_history[i].first;
			break;
		case 90:
			new_row = position_history[i].second;
			new_col = original_map.size() - position_history[i].first - 1;
			break;
		case 135:
			new_row = original_map.size() - position_history[i].first - 1 - position_history[i].second;
			new_col = original_map.size() - position_history[i].first - 1 + position_history[i].second;
			break;
		case 180:
			new_row = original_map.size() - position_history[i].first - 1;
			new_col = original_map.size() - position_history[i].second - 1;
			break;
		case 225:
			new_row = original_map.size() - position_history[i].first - 1 + position_history[i].second;
			new_col = position_history[i].first - position_history[i].second;
			break;
		case 270:
			new_row = original_map.size() - position_history[i].second - 1;
			new_col = position_history[i].first;
			break;
		case 315:
			new_row = position_history[i].first - position_history[i].second;
			new_col = position_history[i].first + position_history[i].second;
			break;
		default:
			new_row = position_history[i].first;
			new_col = position_history[i].second;
			break;
		}

		new_position_history.push_back(make_pair(new_row, new_col));
	}
	position_history = new_position_history;
}

int ComportamientoJugador::angleDifference(Orientacion first_orientation, Orientacion second_orientation){
	int first_angle = orientationToAngle(first_orientation);
	int second_angle = orientationToAngle(second_orientation);
	
	return abs(first_angle - second_angle) % 360;
}

int ComportamientoJugador::orientationToAngle(Orientacion orientation){
	switch(orientation){
		case norte:
			return 0;
		case noreste:
			return 45;
		case este:
			return 90;
		case sureste:
			return 135;
		case sur:
			return 180;
		case suroeste:
			return 225;
		case oeste:
			return 270;
		case noroeste:
			return 315;
		default:
			return 0;
	}
}

void ComportamientoJugador::recenterMap(vector<vector<MapCell>> &original_map, int angle, const int row_offset, const int col_offset)
{
	int size = original_map.size();
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

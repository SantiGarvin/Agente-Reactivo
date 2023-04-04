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
		vision(mapaResultado, sensors);
	else
		vision(map, sensors);

	// Decide la accion a realizar basado en el estado actual
	action = move(sensors);

	// Guarda la ultima accion realizada
	last_action = action;

	move_left = (last_action == actTURN_BL || last_action == actTURN_SL);
	move_right = (last_action == actTURN_BR || last_action == actTURN_SR);
	move_forward = last_action == actFORWARD;

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

void ComportamientoJugador::initMaps(int size){
		// Inicializar mapa auxiliar
		initMap(map, size, (unsigned char)'?');

		// Inicializar mapa visitas
		initMap(cell_visit_map, size, (int)0);

		// Inicializar mapa coste bateria
		initMap(battery_cost_map, size, (BatteryCost){0, 0, 0});

		// Inicializar mapa potenciales
		initMap(potencial_map, size, (double)0.0);
}

void ComportamientoJugador::updateState(const Sensores &sensors)
{
	updatePositionOrientation();

	// Reinicio de la simulacion
	if (sensors.reset)
	{
		current_state = {99, 99, Orientacion::norte, false, false, false};
		current_state.has_sneakers = false;
		current_state.has_bikini = false;

		reset_counter++;
		last_action = actIDLE;

		initMaps(2*MAX_SIZE_MAP);
	}

	// Nivel 0
	if (sensors.nivel == 0)
	{
		current_state.well_situated = true;
		current_state.row = sensors.posF;
		current_state.col = sensors.posC;
		current_state.orientation = sensors.sentido;

		initMaps(mapaResultado.size());
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
		recenterMaps(mapaResultado.size(), row_offset, col_offset);
	}

	if(sensors.terreno[0] == 'K')
		current_state.has_bikini = true;
	
	if(sensors.terreno[0] == 'D')
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

void ComportamientoJugador::recenterMaps(int size, int row_offset, int col_offset)
{
	recenterMap(map, size, row_offset, col_offset);
	recenterMap(cell_visit_map, size, row_offset, col_offset);
	recenterMap(battery_cost_map, size, row_offset, col_offset);
	recenterMap(potencial_map, size, row_offset, col_offset);
}

BatteryCost ComportamientoJugador::batteryCost(unsigned char cell)
{
	BatteryCost battery_cost;

	battery_cost.forward = batteryCostForward(cell);
	battery_cost.turnSL_SR = batteryCostTurnSL_SR(cell);
	battery_cost.turnBL_BR = batteryCostTurnBL_BR(cell);
}

int ComportamientoJugador::bestBatteryCost(BatteryCost battery_cost)
{
	return min(min(battery_cost.forward, battery_cost.turnSL_SR), battery_cost.turnBL_BR);
}

vector<vector<double>> ComportamientoJugador::potencials(const vector<vector<int>> &map, const vector<vector<int>> cell_visit, const vector<vector<BatteryCost>> battery_cost)
{
	int rows = map.size();
	int cols = map[0].size();

	vector<vector<double>> potentials(rows, vector<double>(cols, 0.0));

	double attraction;
	for (int row = 0; row < rows; ++row)
	{
		for (int col = 0; col < cols; ++col)
		{
			if (map[row][col] == 'M' || map[row][col] == 'P')
				attraction = WALL_PRECIPICE_PENALTY;
			else if((map[row][col] == 'K' && !current_state.has_bikini) || 
					(map[row][col] == 'D' && !current_state.has_sneakers))
				attraction = TARGET_ATTRACTION;
			else if(map[row][col] == '?')
				attraction = UNKNOWN_CELL_ATTRACTION;
			else{
				double visit_penalty = log(1 + cell_visit[row][col]) * VISIT_PENALTY_FACTOR;
				double battery_cost_penalty = bestBatteryCost(battery_cost[row][col]) * BATTERY_COST_FACTOR;

				if (cell_visit[row][col] > 0)
					attraction = -visit_penalty - battery_cost_penalty;
				else
					attraction = UNVISITED_ATTRACTION - battery_cost_penalty;
			}
			potentials[row][col] = attraction;
		}
	}

	return potentials;
}

void ComportamientoJugador::vision(vector<vector<unsigned char>> &mapa, Sensores sensores)
{
	mapa[current_state.row][current_state.col] = sensores.terreno[0];

	BatteryCost battery_cost = batteryCost(sensores.terreno[0]);

	// Actualiza el mapa de coste de bateria
	battery_cost_map[current_state.row][current_state.col] = battery_cost;

	// Actualiza el mapa de visitas
	cell_visit_map[current_state.row][current_state.col]++;

	const int DIM = 4; // dimension de la vision
	int index = 0;
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

				mapa[row][col] = sensores.terreno[index];
				battery_cost_map[row][col] = batteryCost(sensores.terreno[index]);
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

				mapa[row][col] = sensores.terreno[index];
				battery_cost_map[row][col] = batteryCost(sensores.terreno[index]);
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

				mapa[row][col] = sensores.terreno[index];
				battery_cost_map[row][col] = batteryCost(sensores.terreno[index]);
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

				mapa[row][col] = sensores.terreno[index];
				battery_cost_map[row][col] = batteryCost(sensores.terreno[index]);
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

				mapa[row][col] = sensores.terreno[index];
				battery_cost_map[row][col] = batteryCost(sensores.terreno[index]);
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

				mapa[row][col] = sensores.terreno[index];
				battery_cost_map[row][col] = batteryCost(sensores.terreno[index]);
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

				mapa[row][col] = sensores.terreno[index];
				battery_cost_map[row][col] = batteryCost(sensores.terreno[index]);
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

				mapa[row][col] = sensores.terreno[index];
				battery_cost_map[row][col] = batteryCost(sensores.terreno[index]);
			}
		}
		break;
	}
}

int ComportamientoJugador::targetInVision(const Sensores &sensors, unsigned char target)
{
	// Si el objetivo esta dentro del campo de vision, devolver su posicion
	for (int i = 0; i < sensors.terreno.size(); i++)
		if (sensors.terreno[i] == target)
			return i;

	return -1;
}

Action ComportamientoJugador::move(Sensores sensors)
{
	Action action = actIDLE;

	unsigned char left_cell = sensors.terreno[1];
	unsigned char front_cell = sensors.terreno[2];
	unsigned char right_cell = sensors.terreno[3];

	updateState(sensors);

	

	return action;
}

void ComportamientoJugador::moveToTarget(const Sensores &sensors, unsigned char target)
{
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
		return 0;
	default:
		return 1;
	}
}

int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}

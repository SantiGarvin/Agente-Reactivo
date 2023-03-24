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
	{
		vision(mapaResultado, sensors);
	}
	else
	{
		vision(map, sensors);
	}



	action = move(sensors);

	last_action = action;

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

void ComportamientoJugador::initMap(vector<vector<unsigned char>> &map, int size, unsigned char value)
{
	map.resize(size, vector<unsigned char>(size, value));
}

void ComportamientoJugador::updateState(const Sensores &sensors)
{
	updateCurrentState();

	// Reinicio de la simulacion
	if (sensors.reset)
	{
		current_state = {0, 0, Orientacion::norte, false, false, false};
		last_action = actIDLE;
	}

	if (sensors.nivel == 0)
	{
		current_state.well_situated = true;
		current_state.row = sensors.posF;
		current_state.col = sensors.posC;
	}

	// Si no esta bien situado y cae en una casilla de posicionamiento, se actualiza la posicion
	if (sensors.terreno[0] == 'G' and !current_state.well_situated)
	{
		current_state.row = sensors.posF;
		current_state.col = sensors.posC;
		current_state.orientation = sensors.sentido;
		current_state.well_situated = true;

		// Copia del mapa de la simulacion al mapa de la practica
		updateMapaResultado(sensors);
	}
	
}

void ComportamientoJugador::updateCurrentState()
{
	if(last_action != actIDLE){
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

void ComportamientoJugador::updateMapaResultado(const Sensores &sensors){
	if(current_state.well_situated){
		int diff_row = current_state.row - sensors.posF;
		int diff_col = current_state.col - sensors.posC;

		for(int i = 0; i < mapaResultado.size(); i++){
			for(int j = 0; j < mapaResultado.size(); j++){
				if(mapaResultado[i][j] == '?'){
					mapaResultado[i][j] = map[diff_row + i][diff_col + j];
				}
			}
		}
	}
}

void ComportamientoJugador::vision(vector<vector<unsigned char>> &mapa, Sensores sensores)
{
	mapa[current_state.row][current_state.col] = sensores.terreno[0];

	const int DIM = 4; // dimension de la vision
	int index = 1;
	switch (sensores.sentido)
	{
	case 0: // vision Norte
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int fila = current_state.row - i;
				int columna = current_state.col + j;

				mapa[fila][columna] = sensores.terreno[index++];
			}
		}
		break;
	case 1: // vision Noreste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int fila = current_state.row;
				int columna = current_state.col;

				if (j <= 0)
				{
					fila -= i;
					columna += i + j;
				}
				else
				{
					fila += j - i;
					columna += i;
				}
				mapa[fila][columna] = sensores.terreno[index++];
			}
		}
		break;
	case 2: // vision Este
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int fila = current_state.row + j;
				int columna = current_state.col + i;

				mapa[fila][columna] = sensores.terreno[index++];
			}
		}
		break;
	case 3: // vision Sureste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int fila = current_state.row;
				int columna = current_state.col;

				if (j <= 0)
				{
					fila += i + j;
					columna += i;
				}
				else
				{
					fila += i;
					columna += i - j;
				}
				mapa[fila][columna] = sensores.terreno[index++];
			}
		}
		break;
	case 4: // vision Sur
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int fila = current_state.row + i;
				int columna = current_state.col - j;

				mapa[fila][columna] = sensores.terreno[index++];
			}
		}
		break;
	case 5: // vision Suroeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int fila = current_state.row;
				int columna = current_state.col;

				if (j <= 0)
				{
					fila += i;
					columna -= i + j;
				}
				else
				{
					fila -= j - i;
					columna -= i;
				}
				mapa[fila][columna] = sensores.terreno[index++];
			}
		}
		break;
	case 6: // vision Oeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int fila = current_state.row - j;
				int columna = current_state.col - i;

				mapa[fila][columna] = sensores.terreno[index++];
			}
		}
		break;
	case 7: // vision Noroeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int fila = current_state.row;
				int columna = current_state.col;

				if (j <= 0)
				{
					fila -= j + i;
					columna -= i;
				}
				else
				{
					fila -= i;
					columna += j - i;
				}
				mapa[fila][columna] = sensores.terreno[index++];
			}
		}
		break;
	}
}

int ComportamientoJugador::targetInVision(const Sensores &sensors, unsigned char target)
{
	int position = -1;

	// Si el objetivo esta dentro del campo de vision, devolver su posicion
	for (int i = 0; i < sensors.terreno.size(); i++)
		if (sensors.terreno[i] == target)
			position = i;

	return position;
}

Action ComportamientoJugador::move(Sensores sensors)
{
	Action action = actIDLE;

	unsigned char right_cell = sensors.terreno[2];

	

	return action;
}

void ComportamientoJugador::moveToTarget(const Sensores &sensors, unsigned char target){
	
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
	default:
		return 1;
	}
}

int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}

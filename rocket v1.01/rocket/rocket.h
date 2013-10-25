#include<Windows.h>

const int fWidth = 700;				// Ширина поля
const int fHeight = 420;			// Висота поля
const int fX0 = 0;					// Координати початку поля відносно клієнтської області по осі Х
const int fY0 = 0;					// Координати початку поля відносно клієнтської області по осі Y
const int rWidth = 32;				// Ширина ракети
const int rHeight = 32;				// Висота ракети
const int gWidth  = 5;				// Ширина участка землі
const int gSize = fWidth / gWidth;  // Кількість ел. в масиві почви
const int gY = fHeight - gWidth;	// Початок замлі (потрібно для контролю за приземленням)
const int iMaxLandingSpeed = 10;	// Максимальна швидкість для вдачного приземлення

HWND hRocket;						// Хендл ракети
HWND hFireLeft[3] = {0};			// Хендл лівого вогню
HWND hFireRight[3] = {0};			// Хендл правого вогню
HWND hFireDown[3] = {0};			// Хендл вогню вниз
int groundArr[gSize] = {0};


class Rocket
{
private:
	int speed;
	int x_engine, y_engine;
	int fuel;
	int gravitation;

	RECT Cur_Pos, Prev_Pos;
public:
	Rocket(int diff, int gravitation);
	int Get_Fuel();
	int Get_X_engine();
	int Get_Y_engine();
	int Get_Speed();
	void Set_X_engine(int power);
	void Set_Y_engine(int power);
	RECT Get_Cur_Pos();	
	RECT Get_Prev_pos();
	void CalculateNext();
};

Rocket::Rocket(int difficult, int _gravitation)//    0 < difficult < 2
{
	speed = 0;
	x_engine = y_engine = 0;
	fuel = 100;
	difficult *= 20;
	fuel -= difficult;
	gravitation = _gravitation;

	Cur_Pos.left = fY0 + ((fWidth/2) - (rWidth/2));
	Cur_Pos.top = fX0;
	Cur_Pos.right = Cur_Pos.left + rWidth;
	Cur_Pos.bottom = Cur_Pos.top + rHeight;

	Prev_Pos = Cur_Pos;
}

int Rocket::Get_Fuel()
{
	return fuel;
}

int Rocket::Get_X_engine()
{
	return x_engine;
}
int Rocket::Get_Y_engine()
{
	return y_engine;
}

void Rocket::Set_X_engine(int power)
{
	if(power >= -100 && power <= 100)
	{
		x_engine = power;
		//fuel -= abs(x_engine) * 0.01; 
	}
	if(fuel <= 0)
	{
		x_engine = 0;
	}
	else
		fuel -= abs(x_engine/20);
}

void Rocket::Set_Y_engine(int power)
{
	if(power >= 0 && power <= 100)
	{
		y_engine = power;
		//fuel -= abs(y_engine) * 0.01; 
	}
	if(fuel <= 0)
	{
		y_engine = 0;
	}
	else
		fuel -= y_engine/20;
}

int Rocket::Get_Speed()
{
	return speed;
}

RECT Rocket::Get_Cur_Pos()
{
	return Cur_Pos;
}

RECT Rocket::Get_Prev_pos()
{
	return Prev_Pos;
}

void Rocket::CalculateNext()
{
	Prev_Pos = Cur_Pos;

	Cur_Pos.top += (gravitation - y_engine/5);
	Cur_Pos.bottom += (gravitation - y_engine/5);
	if( x_engine < 0 )
	{
		Cur_Pos.left += abs(x_engine/10);
		Cur_Pos.right += abs(x_engine/10);
	}
	else
	{
		Cur_Pos.left -= x_engine/10;
		Cur_Pos.right -= x_engine/10;
	}

	if( Cur_Pos.top < 0 )
	{
		Cur_Pos.top = 0;
		Cur_Pos.bottom = rHeight;
	}
	if( Cur_Pos.left < 0 )
	{
		Cur_Pos.left = 0;
		Cur_Pos.right = rWidth;
	}
	if( Cur_Pos.right > fWidth )
	{
		Cur_Pos.right = fWidth;
		Cur_Pos.left = fWidth - rWidth;
	}
	if( Cur_Pos.bottom > fHeight )
	{
		Cur_Pos.bottom = fHeight;
		Cur_Pos.top = fHeight - rHeight;
	}

	speed = ((abs(Cur_Pos.left - Prev_Pos.left) + abs(Cur_Pos.top - Prev_Pos.top))/2)*10;
}
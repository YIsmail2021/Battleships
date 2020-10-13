// BattleshipBot.cpp : Defines the entry point for the console application.
//
/*
* Author: Yonis Ismail (Student Number)
* Created: 05/04/2019
* Revised: 08/04/2019
* Description: Battleship
* User advice: None

*/
#include "stdafx.h"
#include <winsock2.h>
#include <math.h>

#pragma comment(lib, "wsock32.lib")

#define SHIPTYPE_BATTLESHIP	"0"
#define SHIPTYPE_FRIGATE	"1"
#define SHIPTYPE_SUBMARINE	"2"

#define STUDENT_NUMBER		"Yonis" // change later
#define STUDENT_FIRSTNAME	"Yonis"
#define STUDENT_FAMILYNAME	"Ismail"
#define MY_SHIP	SHIPTYPE_BATTLESHIP


#define IP_ADDRESS_SERVER	"127.0.0.1"



#define PORT_SEND	 1924 // We define a port that we are going to use.
#define PORT_RECEIVE 1925 // We define a port that we are going to use.


#define MAX_BUFFER_SIZE	500
#define MAX_SHIPS		200

#define FIRING_RANGE	100

#define MOVE_LEFT		-1
#define MOVE_RIGHT		 1
#define MOVE_UP			 1
#define MOVE_DOWN		-1
#define MOVE_FAST		 2
#define MOVE_SLOW		 1

#define NOT_FOUND		-1

SOCKADDR_IN sendto_addr;
SOCKADDR_IN receive_addr;

SOCKET sock_send;  // This is our socket, it is the handle to the IO address to read/write packets
SOCKET sock_recv;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer  [MAX_BUFFER_SIZE];



int myX;
int myY;
int myHealth;
int myFlag;
int myType;

int number_of_ships;
int shipX[MAX_SHIPS];
int shipY[MAX_SHIPS];
int shipHealth[MAX_SHIPS];
int shipFlag[MAX_SHIPS];
int shipType[MAX_SHIPS];

bool message = false;
char MsgBuffer [MAX_BUFFER_SIZE];

bool fire = false;
int fireX;
int fireY;

bool moveShip = false;
int moveX;
int moveY;

bool setFlag = true;
int new_flag = 0;

void send_message(char* dest, char* source, char* msg);
void fire_at_ship(int X, int Y);
void move_in_direction(int left_right, int up_down);
void set_new_flag(int newFlag);



/*************************************************************/
/********* Your tactics code starts here *********************/
/*************************************************************/

int up_down = MOVE_LEFT*MOVE_SLOW;
int left_right = MOVE_UP*MOVE_FAST;

int shipDistance[MAX_SHIPS];

int number_of_friends;
int friendX[MAX_SHIPS];
int friendY[MAX_SHIPS];
int friendHealth[MAX_SHIPS];
int friendFlag[MAX_SHIPS];
int friendDistance[MAX_SHIPS];
int friendType[MAX_SHIPS];

int number_of_enemies;
int enemyX[MAX_SHIPS];
int enemyY[MAX_SHIPS];
int enemyHealth[MAX_SHIPS];
int enemyFlag[MAX_SHIPS];
int enemyDistance[MAX_SHIPS];
int enemyType[MAX_SHIPS];

bool IsaFriend(int index)
{
	bool rc;

	rc = false;

	if (shipFlag[index] == 123)
	{
		rc = true;  // I have just seen my friend 123
	}

	return rc;
}


//*********************
/*
* Function: getNearestEnemy()
* Description : Find the nearest enemy in range
* Parameters: none
* Returns: nearestEnemy
* Warnings: none
*/

//Find the nearest enemy in range
int getNearestEnemy()
{
	int nearestEnemy = 0;
	int distanceEnemy = 1000; // Large number to start with so we always get an enemy

	for (int i = 0; i < number_of_enemies; i++)
	{

		if (enemyDistance[i] <= FIRING_RANGE)
		{
			if (enemyDistance[i] <= distanceEnemy) // Always true for first record
			{
				distanceEnemy = enemyDistance[i];
				nearestEnemy = i;
			}
		}
	}
	return nearestEnemy;
}
/*
* Function: evade
* Description : Evade in opposite direction from closest enemy
* Parameters: nearestEnemy
* Returns: none (void)
* Warnings: none
*/
//Evade in opposite direction from closest enemy
void evade(int nearestEnemy)
{
	int  newLeftRight = 0;
	int  newUpDown = 0;
	char moveMessage[100];

	strcpy_s(moveMessage, "Evading ");

	// Enemy is to my right
	if (enemyX[nearestEnemy] > myX)
	{
		strcat_s(moveMessage, "Left ");
		newLeftRight = MOVE_LEFT * MOVE_FAST;
	}

	// Enemy is to my left
	if (enemyX[nearestEnemy] < myX)
	{
		strcat_s(moveMessage, "Right ");
		newLeftRight = MOVE_RIGHT * MOVE_FAST;
	}

	// Enemy is above me
	if (enemyY[nearestEnemy] > myY)
	{
		strcat_s(moveMessage, " Down ");
		newUpDown = MOVE_DOWN * MOVE_FAST;
	}

	// Enemy is below me
	if (enemyY[nearestEnemy] < myY)
	{
		strcat_s(moveMessage, " Up ");
		newUpDown = MOVE_UP * MOVE_FAST;
	}
	printf("%s\n", moveMessage);

	move_in_direction(newLeftRight, newUpDown);

}
/*
* Function: getNearestFriend()
* Description : Get the nearest friend for your ship
* Parameters: none
* Returns: none (void)
* Warnings: none
*/
//Evade in opposite direction from closest enemy
int getNearestFriend()
{
	int nearestFriend = NOT_FOUND;
	int distanceFriend = 1000;
	for (int i = 0; i < number_of_friends; i++)
	{// find nearest friend 

		if (friendDistance[i] > 0)
		{
			if (friendDistance[i] < distanceFriend)
			{
				distanceFriend = friendDistance[i];
				nearestFriend = i;
			}
		}
	}
	return nearestFriend;
}


/*
* Function: join
* Description : join the closest friend
* Parameters: nearestEnemy
* Returns: none (void)
* Warnings: none
*/

void join(int nearestFriend)
{
	int  newLeftRight = 0;
	int  newUpDown = 0;
	char moveMessage[100];


	if (friendX[nearestFriend] > myX)
	{

		newLeftRight = MOVE_RIGHT * MOVE_FAST;
	}

	if (friendX[nearestFriend] < myX)
	{

		newLeftRight = MOVE_LEFT * MOVE_FAST;
	}

	if (friendX[nearestFriend] > myY)
	{

		newUpDown = MOVE_UP * MOVE_FAST;
	}

	if (friendX[nearestFriend] < myY)
	{

		newUpDown = MOVE_DOWN * MOVE_FAST;
	}

	move_in_direction(newLeftRight, newUpDown);

}

// Hit closest enemy in range and if they have more health then move away
void tactics2()
{
	int closestEnemy = 0;
	int closestFriend = 0;

// Then get the closest enemy and check his health
	closestEnemy = getNearestEnemy();

	// Only evade or fire if in range
	if ((enemyDistance[closestEnemy] >= 0) && (enemyDistance[closestEnemy] <= FIRING_RANGE))
	{
	for (int i = 0; i < number_of_ships; i++)
	{
		shipDistance[i] = (int)sqrt((double)((shipX[i] - shipX[0])*(shipX[i] - shipX[0]) + (shipY[i] - shipY[0])*(shipY[i] - shipY[0])));
		}

		printf("%s %d %s %d\n", "Firing on Enemy ", closestEnemy, " at distance ", enemyDistance[closestEnemy]);
		fire_at_ship(enemyX[closestEnemy], enemyY[closestEnemy]);
		if (enemyHealth[closestEnemy] > myHealth)
		{
			printf("%s %d %s %d\n", "Evading Enemy ", closestEnemy, " at distance ", enemyDistance[closestEnemy]);
			evade(closestEnemy);
		}



	}
	closestFriend = getNearestFriend();
	if (closestFriend != NOT_FOUND)
	{
		join(closestFriend);
	}
}



//***********************


/*
* Function: tactics
* Description: create tactics for the battleship by controlling movement
* Parameters: none (void) 
* Returns: none (void)
* Warnings: none
*/
void tactics()
{
	int i;

	if ( myY > 900) 
	{
		up_down = MOVE_DOWN*MOVE_SLOW;
	}

	if (myX < 200)
	{
		left_right = MOVE_RIGHT*MOVE_FAST;
	}

	if ( myY < 100)
	{
		up_down = MOVE_UP*MOVE_FAST;
	}

	if (myX > 800)
	{
		left_right = MOVE_LEFT*MOVE_SLOW;
	}

	for (i=0; i<number_of_ships; i++)
	{
		shipDistance[i] = (int)sqrt((double)((shipX[i]-shipX[0])*(shipX[i]-shipX[0]) + (shipY[i]-shipY[0])*(shipY[i]-shipY[0])));
	}
	
	tactics2();
	getNearestEnemy();
	
	getNearestFriend();
	
	


	number_of_friends = 0;
	number_of_enemies = 0;

	if (number_of_ships > 1)
	{
		for (i=1; i<number_of_ships; i++)
		{
			if (IsaFriend(i))
			{
				friendX[number_of_friends] = shipX[i];
				friendY[number_of_friends] = shipY[i];
				friendHealth[number_of_friends] = shipHealth[i];
				friendFlag[number_of_friends] = shipFlag[i];
				friendDistance[number_of_friends] = shipDistance[i];		
				friendType[number_of_friends] = shipType[i];		
				number_of_friends++;
			}
			else
			{
				enemyX[number_of_enemies] = shipX[i];
				enemyY[number_of_enemies] = shipY[i];
				enemyHealth[number_of_enemies] = shipHealth[i];
				enemyFlag[number_of_enemies] = shipFlag[i];
				enemyDistance[number_of_enemies] = shipDistance[i];		
				enemyType[number_of_enemies] = shipType[i];		
				number_of_enemies++;
			}
		}

		if (number_of_enemies > 0)
		{
			fire_at_ship(enemyX[0], enemyY[0]);
		}
	}

	char msg[100];
	sprintf_s(msg, "Im at %d %d", myX, myY);
	send_message("12345678", "23456789", msg);  // send my co-ordinates to myself 

	move_in_direction(left_right, up_down);
}

/*
* Function: messageReceived
* Description: Recieve a message about coordinates and where the friend is
* Parameters: msg
* Returns: none (void)
* Warnings: none (void)
*/
void messageReceived(char* msg)
{
	int X;
	int Y;

	printf("%s\n", msg);

	if (sscanf_s(msg, "Message 12345678, 23456789, Im at %d %d", &X, &Y) == 2)
	{
		printf("My friend is at %d %d\n", X, Y);
	}
}


void callevade(int nearestEnemy)
{
	evade(nearestEnemy);
}




/*************************************************************/
/********* Your tactics code ends here ***********************/
/*************************************************************/


/*
* Function: communicate_with_server
* Description : communicate with battleship server
* Parameters: none
* Returns: none (void)
* Warnings: none
*/
void communicate_with_server()
{
	char buffer[4096];
	int  len = sizeof(SOCKADDR);
	char chr;
	bool finished;
	int  i;
	int  j;
	int  rc;
	char* p;
	myHealth = 1000;

	sprintf_s(buffer, "Register  %s,%s,%s,%s", STUDENT_NUMBER, STUDENT_FIRSTNAME, STUDENT_FAMILYNAME, MY_SHIP, myHealth);
	sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));

	while (true)
	{
		if (recvfrom(sock_recv, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&receive_addr, &len) != SOCKET_ERROR)
		{
			p = ::inet_ntoa(receive_addr.sin_addr);

			if ((strcmp(IP_ADDRESS_SERVER, "127.0.0.1") == 0) || (strcmp(IP_ADDRESS_SERVER, p) == 0))
			{
				if (buffer[0] == 'M')
				{
					messageReceived(buffer);
				}
				else
				{
					i = 0;
					j = 0;
					finished = false;
					number_of_ships = 0;

					while ((!finished) && (i<4096))
					{
						chr = buffer[i];

						switch (chr)
						{
						case '|':
							InputBuffer[j] = '\0';
							j = 0;
							sscanf_s(InputBuffer,"%d,%d,%d,%d", &shipX[number_of_ships], &shipY[number_of_ships], &shipHealth[number_of_ships], &shipFlag[number_of_ships], &shipType[number_of_ships]);
							number_of_ships++;
							break;

						case '\0':
							InputBuffer[j] = '\0';
							sscanf_s(InputBuffer,"%d,%d,%d,%d", &shipX[number_of_ships], &shipY[number_of_ships], &shipHealth[number_of_ships], &shipFlag[number_of_ships], &shipType[number_of_ships]);
							number_of_ships++;
							finished = true;
							break;

						default:
							InputBuffer[j] = chr;
							j++;
							break;
						}
						i++;
					}

					myX = shipX[0];
					myY = shipY[0];
					myHealth = 1000;
					myFlag = shipFlag[0];
					myType = shipType[0];
				}

				tactics();
				tactics2();
				getNearestEnemy();
				getNearestFriend();

				if (message)
				{
					sendto(sock_send, MsgBuffer, strlen(MsgBuffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					message = false;
				}

				if (fire)
				{
					sprintf_s(buffer, "Fire %s,%d,%d", STUDENT_NUMBER, fireX, fireY);
					sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					fire = false;
				}

				if (moveShip)
				{
					sprintf_s(buffer, "Move %s,%d,%d", STUDENT_NUMBER, moveX, moveY);
					rc = sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					moveShip = false;
				}

				if (setFlag)
				{
					sprintf_s(buffer, "Flag %s,%d", STUDENT_NUMBER, new_flag);
					sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					setFlag = false;
				}

			}
		}
		else
		{
			printf_s("recvfrom error = %d\n", WSAGetLastError());
		}
	}

	printf_s("Student %s\n", STUDENT_NUMBER);
}
/*
* Function: send_message
* Description : send a message when message is true
* Parameters: dest, source, msg
* Returns: none (void)
* Warnings: none
*/


void send_message(char* dest, char* source, char* msg)
{
	message = true;
	sprintf_s(MsgBuffer, "Message %s,%s,%s,%s", STUDENT_NUMBER, dest, source, msg);
}
/*
* Function: fire_at_ship
* Description : Set new flags if setFlag is true
* Parameters: newFlag
* Returns: none (void)
* Warnings: none
*/

void fire_at_ship(int X, int Y)
{
	fire = true;
	fireX = X;
	fireY = Y;
}
/*
* Function: move_in_direction
* Description : Move in new direction if moveShip is true
* Parameters: X,Y
* Returns: none (void)
* Warnings: none
*/

void move_in_direction(int X, int Y)
{
	if (X < -2) X = -2;
	if (X >  2) X =  2;
	if (Y < -2) Y = -2;
	if (Y >  2) Y =  2;

	moveShip = true;
	moveX = X;
	moveY = Y;
}

/*
* Function: set_new_flag
* Description : Set new flags if setFlag is true 
* Parameters: newFlag
* Returns: none (void)
* Warnings: none
*/
void set_new_flag(int newFlag)
{
	setFlag = true;
	new_flag = newFlag;
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr = '\0';

	printf("\n");
	printf("Battleship Bots\n");
	printf("UWE Computer and Network Systems Assignment 2 (2016-17)\n");
	printf("\n");

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	//sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	//if (!sock)
	//{	
	//	printf("Socket creation failed!\n"); 
	//}

	sock_send = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock_send)
	{	
		printf("Socket creation failed!\n"); 
	}

	sock_recv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock_recv)
	{	
		printf("Socket creation failed!\n"); 
	}

	memset(&sendto_addr, 0, sizeof(SOCKADDR_IN));
	sendto_addr.sin_family = AF_INET;
	sendto_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	sendto_addr.sin_port = htons(PORT_SEND);

	memset(&receive_addr, 0, sizeof(SOCKADDR_IN));
	receive_addr.sin_family = AF_INET;
	//	receive_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	receive_addr.sin_addr.s_addr = INADDR_ANY;
	receive_addr.sin_port = htons(PORT_RECEIVE);

	int ret = bind(sock_recv, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	//	int ret = bind(sock_send, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	if (ret)
	{
		printf("Bind failed! %d\n", WSAGetLastError());  
	}

	communicate_with_server();

	closesocket(sock_send);
	closesocket(sock_recv);
	WSACleanup();

	while (chr != '\n')
	{
		chr = getchar();
	}

	return 0;
}


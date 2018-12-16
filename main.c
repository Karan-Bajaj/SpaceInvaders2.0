#include <lpc17xx.h>
#include <stdio.h>
#include <stdint.h>
#include "uart.h"
#include "GLCD.h"
#include <RTL.h>
#include <stdlib.h>
#include "bad-guy.c"
#include "Shooter.c"
#include "timer.h"
#include "bckgrnd.c"
#include "Laser.c"


OS_SEM sem1;
OS_SEM sem2;
OS_SEM sem3;


/**** TASK PROTOTYPES ****/
__task void moveShooter_tsk(void);
__task void shftObstacle_tsk(void);
__task void shootPew_tsk(void);
/**** END ****/

/**** TASK AND SEMAPHORES CREATED ****/
__task void start_tasks(void)
{
	os_sem_init(&sem1, 1);
	os_sem_init(&sem2, 0);
	os_sem_init(&sem3, 0);
	
	os_tsk_create(moveShooter_tsk,2);
	os_tsk_create(shftObstacle_tsk,1);
	os_tsk_create(shootPew_tsk,3);
	os_tsk_delete_self();
}
/**** END ****/

/***** DECLARE OUR STRUCT THAT DEFINES THE OBSTACLES****/
typedef struct Obstacles{
	
		uint32_t posX;
		uint32_t posY;
		uint32_t status;	
}Obstacles;
/***** END *****/

typedef struct Grid{
	int x;
	int y;
}Grid;

/**** GLOBAL VARIABLES NEEDED ****/
static Obstacles arrObst[9];
static Grid arrGrid[22][14];
struct Grid shootGrid[22];
static int gridX = 0;
static int gridY = 0;
static int laserShot=0;
float static previousTime = 0;
static int shooterPos=0;
static int shooterXLocation = 0;
static int countRight = 0;
static int countLeft = 0;
static int leftRightToggle = 0; // 0 means moving right 1 means moving left
static int downIndicator = 0;

static uint32_t temp; 
static uint32_t temp_old=1;
/**** END ****/


/**** GENERAL FUNCTION DEFINITIONS ****/

void obstacleSetup (Obstacles arrObst[]){
	int i;
	arrObst[0].posX = arrGrid[gridX][gridY].x;
	arrObst[0].posY = arrGrid[gridX][gridY].y;
	arrObst[1].posX = arrGrid[gridX+2][gridY].x;
	arrObst[1].posY = arrGrid[gridX+2][gridY].y;
	arrObst[2].posX = arrGrid[gridX+4][gridY].x;
	arrObst[2].posY = arrGrid[gridX+4][gridY].y;
	arrObst[3].posX = arrGrid[gridX][gridY+2].x;
	arrObst[3].posY = arrGrid[gridX][gridY+2].y;
	arrObst[4].posX = arrGrid[gridX+2][gridY+2].x;
	arrObst[4].posY = arrGrid[gridX+2][gridY+2].y;
	arrObst[5].posX = arrGrid[gridX+4][gridY+2].x;
	arrObst[5].posY = arrGrid[gridX+4][gridY+2].y;
	arrObst[6].posX = arrGrid[gridX][gridY+4].x;
	arrObst[6].posY = arrGrid[gridX][gridY+4].y;
	arrObst[7].posX = arrGrid[gridX+2][gridY+4].x;
	arrObst[7].posY = arrGrid[gridX+2][gridY+4].y;
	arrObst[8].posX = arrGrid[gridX+4][gridY+4].x;
	arrObst[8].posY = arrGrid[gridX+4][gridY+4].y;
	
//It only goes until 19 in X grid

	for(i=0; i<9;i++)									//Actually printing the blocks on the screen
	{
		arrObst[i].status = 1;
		//printf("%d,%d\n", arrObst[i].posX, arrObst[i].posY);
		GLCD_Bitmap(arrObst[i].posX,arrObst[i].posY,16,16, (unsigned char *)&BADSHIP_pixel_data);
	}	
		
}


void flashLEDs()
{
	int i;
	float time = timer_read();
	uint32_t testNum  = 255;
	
	LPC_GPIO2->FIODIR |= 0x0000007C;
			LPC_GPIO1->FIODIR |= 0xB0000000;
			LPC_GPIO1->FIOSET |= (((testNum & 0x80)>>7)<<28);
			LPC_GPIO1->FIOSET |= (((testNum & 0x40)>>6)<<29);
			LPC_GPIO1->FIOSET |= (((testNum & 0x20)>>5)<<31);
			LPC_GPIO2->FIOSET |= ((testNum & 0x1)<<6);
			LPC_GPIO2->FIOSET |= (((testNum & 0x2)>>1)<<5);
			LPC_GPIO2->FIOSET |= (((testNum & 0x4)>>2)<<4);
			LPC_GPIO2->FIOSET |= (((testNum & 0x8)>>3)<<3);
			LPC_GPIO2->FIOSET |= (((testNum & 0x10)>>4)<<2);
	
	
	for(i=0;i<1364182;i++){}
	
	LPC_GPIO1->FIOCLR |= 0xB0000000;
	LPC_GPIO2->FIOCLR |= 0x0000007C;
		
	for(i=0;i<1364182;i++){}
}


void shftObstLeft(Obstacles arrObst[])
	{
	/***********OBSTACLE LEFT SHIFT***********/
	int i = 0;
	float time = timer_read()/1E6;
	float deltaTime = time - previousTime;
	if(deltaTime >= 0.15){
		previousTime = time;
		for(i=0; i<9;i++)				//This prints the blocks in the new location
			{
					GLCD_Bitmap(arrObst[i].posX,arrObst[i].posY,16,16, (unsigned char *)&BACKGROUND_pixel_data);
					//printf("%d,%d\n", arrObst[i].posX, arrObst[i].posY);
			} 
		
				
		gridX -= 1;// Increment the moving Left for each thing by 1 pixel each time
	
			
		
		for(i=0;i<9;i++){
			if(i==0 || i ==3 || i == 6){
				arrObst[i].posX=arrGrid[gridX][gridY].x;
			}
		}
		
		for(i=0;i<9;i++){
			if(i==1 || i ==4 || i == 7){
				arrObst[i].posX=arrGrid[gridX+2][gridY].x;
			}
		}
		for(i=0;i<9;i++){
			if(i==2 || i ==5 || i == 8){
				arrObst[i].posX=arrGrid[gridX+4][gridY].x;
			}
		}
		
		for(i=0; i<9;i++)				//This prints the blocks in the new location
		{
			if(arrObst[i].status == 1){
					GLCD_Bitmap(arrObst[i].posX,arrObst[i].posY,16,16, (unsigned char *)&BADSHIP_pixel_data);
					//printf("%d,%d\n", arrObst[i].posX, arrObst[i].posY);
				}
		}
	countLeft -= 1;	
		if (countLeft == 0)
		{
			downIndicator = 1;
		}
		//printf("%d\n",downIndicator);
	/***********OBSTACLE SHIFT LEFT FINISHED***********/
	}
}
	
	
void shftObstRight(Obstacles arrObst[])
	{
	/***********OBSTACLE SHIFT RIGHT***********/
	int i = 0;
	float time = timer_read()/1E6;
	float deltaTime = time - previousTime;
	if(deltaTime >= 0.15){
		
		previousTime = time; // Rests the timer
		
		/*** REMOVES THE OLD IMAGES ****/
		for(i=0; i<9;i++)				//This prints the blocks in the new location
			{
					GLCD_Bitmap(arrObst[i].posX,arrObst[i].posY,16,16, (unsigned char *)&BACKGROUND_pixel_data);
					//printf("%d,%d\n", arrObst[i].posX, arrObst[i].posY);
			} 
		/*** END ****/
				
		gridX = gridX+1;// Increment the moving right for each thing by 1 pixel each time
	
			
		
		for(i=0;i<9;i++){
			if(i==0 || i ==3 || i == 6){
				arrObst[i].posX=arrGrid[gridX][gridY].x;
			}
		}
		
		for(i=0;i<9;i++){
			if(i==1 || i ==4 || i == 7){
				arrObst[i].posX=arrGrid[gridX+2][gridY].x;
			}
		}
		for(i=0;i<9;i++){
			if(i==2 || i ==5 || i == 8){
				arrObst[i].posX=arrGrid[gridX+4][gridY].x;
			}
		}
		
		for(i=0; i<9;i++)				//This prints the blocks in the new location
		{
			if(arrObst[i].status == 1){
					GLCD_Bitmap(arrObst[i].posX,arrObst[i].posY,16,16, (unsigned char *)&BADSHIP_pixel_data);
					//printf("%d,%d\n", arrObst[i].posX, arrObst[i].posY);
				}
		}
	countRight += 1;	
	if (countRight == 15)
	{
		downIndicator = 1;
	}
	//printf("%d\n",downIndicator);
	/***********OBSTACLE SHIFT RIGHT FINISHED***********/
	}
}
	
	
void shftObstDown(Obstacles arrObst[])
{
	/***********OBSTACLE SHIFT DOWN***********/
	
	int i = 0;
	
		for(i=0; i<9;i++)				//This prints the blocks in the new location
			{
				GLCD_Bitmap(arrObst[i].posX,arrObst[i].posY,16,16, (unsigned char *)&BACKGROUND_pixel_data);
			} 
		gridY = gridY+2;// Increment the moving down for each thing by 1 pixel each time
				
			
			for(i=0;i<3;i++){
				arrObst[i].posY=arrGrid[gridX][gridY].y;
			}
			
			for(i=3;i<6;i++){
				arrObst[i].posY=arrGrid[gridX][gridY+2].y;
				
			}
			for(i=6;i<9;i++){
				arrObst[i].posY=arrGrid[gridX][gridY+4].y;
				
			}
			
			for(i=0; i<9;i++)				//This prints the blocks in the new location
			{
				if(arrObst[i].status == 1){
					GLCD_Bitmap(arrObst[i].posX,arrObst[i].posY,16,16, (unsigned char *)&BADSHIP_pixel_data);
					//printf("%d,%d\n", arrObst[i].posX, arrObst[i].posY);
				}
			}
			if(leftRightToggle == 0)
			{
				countLeft = 15;
				leftRightToggle = 1;
			}
			else 
			{
				countRight = 0;
				leftRightToggle = 0;
			}
			

			for(i=0; i<9; i++){ // Check to see if any of the obstacles have reached the shooter
					if((arrObst[i].posY  >= 190) && (arrObst[i].status == 1)){
						GLCD_Clear(White);
						while(1){
							flashLEDs();
						}
					}
			}

			
			printf("%d\n",downIndicator);
	/***********OBSTACLE SHIFT DOWN FINISHED***********/
}


void moveShooter()
{
	//uint32_t temp; 
	//uint32_t temp_old;
																				//Setup for potentiometer
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= ((0x1)<<12);
		
	LPC_ADC->ADCR = (1 << 2) |   
									(4 << 8) |               
									(1 << 21);							
	LPC_ADC->ADCR |= ((0x1)<<24);
	//temp_old = 1;
	
	while((LPC_ADC->ADGDR & (0x1 << 31)) > 0)	//Until we aren't ready to read
	{}
		
	temp = (LPC_ADC->ADGDR) & 0xFFFF;				//Read in potentiometer value
	temp = temp>>4;
		
	//Screen size is around 240 x 320
	//Designed to minimize unnecessary blinking due to clearing screen, below
		
	if( ((temp<3000||temp>1500)&&abs(temp-temp_old)>15) || ((temp>3000||temp<1500)&&abs(temp-temp_old)>50) )
	{
		GLCD_Bitmap(shooterPos,shootGrid[1].y,16,16, (unsigned char *)&BACKGROUND_pixel_data);
		//GLCD_DisplayString(20,0,1,"                       ");
		shooterXLocation = temp/208;	
		shooterPos = shootGrid[shooterXLocation].x;
		
		GLCD_Bitmap(shooterPos,shootGrid[1].y,16,16, (unsigned char *)&SHOOTER_pixel_data); //Move the shooter to the location specified by the potentiometer. The 13.5 is found by guess and test
		temp_old=temp;
		//printf("%d\n", shooterPos);
	}
}


void pushRead (void){
	LPC_PINCON->PINSEL4    &= ~( 3 << 20 );
	LPC_GPIO2->FIODIR      &= ~( 1 << 10 );
	LPC_GPIOINT->IO2IntEnF |=  ( 1 << 10 );
	NVIC_EnableIRQ( EINT3_IRQn );
}


void shootLaser()
{
	
	int i;
	int j=0;
	float time = timer_read();
	int bottomIndex = 0;
	int bottomCoord = 0;	
	
	
	for(i=0;i<9;i++)
	{
		if((arrObst[i].posY>bottomCoord) && (arrObst[i].status ==1)){
			bottomCoord = arrObst[i].posY;
			bottomIndex = i;
		}		
	}
	
	j=13;
	
	//printf("%d...\n",bottomCoord);
	//printf("%d---\n",i);
	
	bottomCoord/=14;
	
	while(j>=bottomCoord) //or try bottomIndex
	{
		GLCD_Bitmap(shooterPos,arrGrid[shooterXLocation][j].y,16,16, (unsigned char *)&LASER_pixel_data);
		j--;
	}
	
	while(time<3000)
	{}
	
	j=13;
		
	while(j>=bottomCoord)
	{
		GLCD_Bitmap(shooterPos,arrGrid[shooterXLocation][j].y,16,16, (unsigned char *)&BACKGROUND_pixel_data);
		j--;
	}
	
	laserShot = 0;
	
}
	

void deleteObst()
{
	int i;
	int bottomIndex = 0;
	int bottomCoord = 0;
	for(i=0;i<9;i++)
	{
		if((arrObst[i].posY>bottomCoord) && (arrObst[i].status ==1)){
			bottomCoord = arrObst[i].posY;
			bottomIndex = i;
		}		
	}
	for(i=0;i<9;i++)
	{
		if((arrObst[i].posY == bottomCoord) && (arrObst[i].status ==1) && (arrObst[i].posX == shooterPos)){
			arrObst[i].status = 0;
			flashLEDs();
		}		
	}
}

/**** END ****/

/**** TASKS ARE DEFINED HERE ****/
__task void moveShooter_tsk(void){
	while(1){
		os_sem_wait(&sem1,0xffff);
		if (laserShot == 1){
			os_sem_send(&sem3);
		}
		else{
			moveShooter();
			os_sem_send(&sem2);
		}
}

}
__task void shftObstacle_tsk(void){
while(1)
{
	os_sem_wait(&sem2,0xffff);
	if (laserShot == 1)
	{
		os_sem_send(&sem3);
	}
	else
	{
		if(countRight < 15 && leftRightToggle == 0)
		{	
			shftObstRight(arrObst);//Put in a counter for shift right and left
		}
		else if(countLeft > 0 && leftRightToggle == 1)
		{
			shftObstLeft(arrObst);
		}
		else if (downIndicator == 1)
		{
			shftObstDown(arrObst);
			downIndicator = 0;
		}
	os_sem_send(&sem1);
	}
}
	}
//downIndicator == 1

__task void shootPew_tsk (void){
	int i;
	while(1){
		os_sem_wait(&sem3,0xffff);
			shootLaser();
			deleteObst();
		os_sem_send(&sem1);
		}
	}

/**** END ****/


/**** DEFINING THE INTERRUPT ****/
void EINT3_IRQHandler (void) {
	laserShot = 1;
  LPC_GPIOINT->IO2IntClr |=  (1 << 10); // Clear Interrupt
}
/**** END ****/


int main (void)
{
	/**** DELCARED VARIABLES FOR INITIALISATION ****/
	int a;
	int b;
	int xCor = 2;
	int yCor = 0;
	/**** END ****/
	
	/**** SETUP FOR LCD ****/
	printf("Initialise");
	GLCD_Init();
	GLCD_Clear(White);
	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Black);
	/**** END ****/
	
	/**** SETUP FOR LED ****/
	
	
	LPC_GPIO1->FIODIR |= 0xB0000000;
	LPC_GPIO2->FIODIR |= 0x0000007C;
	
	LPC_GPIO1->FIOCLR |= 0xB0000000;
	LPC_GPIO2->FIOCLR |= 0x0000007C;
	/**** END ****/
	
	timer_setup(); //Sets up timer
	pushRead(); // Sets up our interrupt
	
	/**** CREATES THE GRID SYSTEM ****/
	
	for(a = 0; a<14; a++){
		for(b = 0;b<22;b++){
			arrGrid[b][a].x = xCor;
			arrGrid[b][a].y = yCor;
			xCor += 16;
		}
		xCor = 2;
		yCor += 16;
	}
	xCor = 2;
	yCor = 228;
		for(b = 0;b<22;b++){
			shootGrid[b].x = xCor;
			shootGrid[b].y = yCor;
			xCor += 16;
		}
	/**** END ****/
	obstacleSetup(arrObst); // Initialises all Obstacles
	os_sys_init(start_tasks);
}
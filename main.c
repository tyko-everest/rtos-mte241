/*********************************************************/
/* Copyright - Jon Gutschon														   */
/*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <lpc17xx.h>
#include "GLCD.h"
#include "uart.h"
#include "graphics.h"

#include "kernel.h"
#include "blocking.h"

/*********************************************************/
/* Copyright Jon Gutschon																 */
/*********************************************************/

os_mutex_id_t snake;
//bool to track state of game
bool stopped;

const uint16_t width = 320;
const uint16_t height = 240;
const int8_t size = 20;

//coords of apple
uint16_t appleX = 0;
uint16_t appleY = 0;

enum dirs{stop, left,	up,	right, down};
enum dirs dir;

//define struct to represent single node of snake
typedef struct Node{
	int32_t x;
	int32_t y;
	struct Node *next;
} snakeNode_t;

snakeNode_t heap[256];
uint32_t heap_index = 0;

//pointers to head and tail
snakeNode_t *head = NULL;
snakeNode_t *tail = NULL;
int16_t prevTailX;
int16_t prevTailY;


//Helper functions
void placeApple() {	
	//rechoose coords for apple if in snake
	bool goodLocation = false;
	snakeNode_t *curr = tail;
	while(!goodLocation) {
		goodLocation = true;
		appleX = (rand()%(width/size))*size;
		appleY = (rand()%(height/size))*size;
		
		while(curr != NULL) {
			if (curr->x == appleX && curr->y == appleY) {
				goodLocation = false;
				curr = tail;
				break;
			}
			else
				curr = curr->next;
		}
	}
}

//reset position of snake and reset display
void reset() {
	os_acquire(snake);
	GLCD_Clear(Black);
	
	heap_index = 0;
	head = heap + heap_index++;
	tail = heap + heap_index++;
	
	head->x = width/2;
	head->y = height/2;
	tail->x = head->x-size;
	tail->y = head->y;
	head->next = NULL;
	tail->next = head;
	os_release(snake);
	
	dir = right;
	placeApple();
	

}

//draws snake before starting
void start() {
	reset();
	
	GLCD_DisplayString(1, 8, 1, (unsigned char *)"Snake");
	GLCD_DisplayString(2, 0, 1, (unsigned char *)"Push button to start");
	
	snakeNode_t *curr = tail;
	while(curr != NULL) {
		GLCD_Bargraph(curr->x, curr->y, size, size, 1024);
		curr = curr->next;
	}
	
	if ((LPC_GPIO2->FIOPIN & (0x01 << 10)) != 0)
		while((LPC_GPIO2->FIOPIN & (0x01 << 10)) != 0);
	stopped = false;
	reset();
}

//detects if head hits the snake
bool hitSelf() {
	snakeNode_t *curr = tail;
	while(curr != head) {
		if (head->x == curr->x && head->y == curr->y)
			return true;
		curr = curr->next;
	}
	return false;
}

void gameOver() {
	stopped = true;
	os_acquire(snake);
	GLCD_Clear(Black);
	
	//calculate score
	uint8_t score = 0;
	snakeNode_t *curr = tail;
	while(curr->next != head) {
		score++;
		curr = curr->next;
	}
	char scoreStr[] = "";
	sprintf(scoreStr, "Score: %d", score);
	
	GLCD_DisplayString(1, 5, 1, (unsigned char *)"GAME OVER");
	GLCD_DisplayString(3, 2, 1, (unsigned char *)"Push to restart");
	GLCD_DisplayString(4, 5, 1, (unsigned char *)scoreStr);
	
	//wait for button
	if ((LPC_GPIO2->FIOPIN & (0x01 << 10)) != 0)
		while((LPC_GPIO2->FIOPIN & (0x01 << 10)) != 0);
	stopped = true;
	
	os_release(snake);
}


//Threaded functions
//start/stop game with push button
void pushButton(void *arg) {
	while(1) {
		if((LPC_GPIO2->FIOPIN & (0x01 << 10)) == 0) {
			while((LPC_GPIO2->FIOPIN & (0x01 << 10)) == 0) {}
			stopped = !stopped;
		}
		os_yield();
	}
}

//poll for input from joystick to set direction
void readJoy(void *arg) {
	while(1) {
		while(!stopped) {
			//cant switch back 180 degrees to opposite direction
			if((LPC_GPIO1->FIOPIN & (0x01 << 23)) == 0 && dir != down)
				dir = up;
			else if((LPC_GPIO1->FIOPIN & (0x01 << 25)) == 0 && dir != up)
				dir = down;
			else if((LPC_GPIO1->FIOPIN & (0x01 << 26)) == 0 && dir != right)
				dir = left;
			else if((LPC_GPIO1->FIOPIN & (0x01 << 24)) == 0 && dir != left)
				dir = right;
		}
		os_yield();
	}
}

void moveSnek(void *arg) {
	int16_t dx = 0;
	int16_t dy = 0;
	
	while(1) {
		while(!stopped) {
			
			/**************************************************/
			// delay 1/16 of a second
			for(int i = 0; i < 750000; i++);
			/**************************************************/
			
			printf("%d, %d, %d\n", head->x, head->y, dir);
			//printf("%d, %d\n", prevTailX, prevTailY);
			
			if (hitSelf() || (head->x <= 0 && dir == left) || (head->x >= width-size && dir == right) || (head->y <= 0 && dir == up) || (head->y >= height-size && dir == down)) {
				gameOver();
				stopped = true;
				reset();
			}
			else {
				//save previous x,y of tail node so we dont need double linked list
				os_acquire(snake);
				prevTailX = tail->x;
				prevTailY = tail->y;
				//iterate through list and change coords to next coords
				snakeNode_t *curr = tail;
				while(curr != head) {
					curr->x = curr->next->x;
					curr->y = curr->next->y;
					curr = curr->next;
				}
				
				//check direction
				if (dir == up) {
					dx = 0;
					dy = -size;
				} else if (dir == down) {
					dx = 0;
					dy = size;
				} else if (dir == right) {
					dx = size;
					dy = 0;
				} else if (dir == left) {
					dx = -size;
					dy = 0;
				}
				//move head according to direction
				head->x += dx;
				head->y += dy;
				
				//increase length of snake
				if(head->x == appleX && head->y == appleY) {
					snakeNode_t *newNode = heap + heap_index++;
					newNode->next = tail;
					newNode->x = prevTailX;
					newNode->y = prevTailY;
					tail = newNode;
					
					placeApple();
				}
				os_release(snake);
			}
			
		}
		os_yield();
	}
}

void display(void *arg) {
	while(1) {
		//osDelay(osKernelGetTickFreq()/2);
		
		os_acquire(snake);
		snakeNode_t *curr = tail;
		while(curr != NULL) {
			GLCD_Bargraph(curr->x, curr->y, size, size, 1024);
			curr = curr->next;
		}
		GLCD_Bargraph(prevTailX, prevTailY, size, size, 0);
		drawApple(appleX, appleY);
		os_release(snake);
		
		os_yield();
	}
}

int main(void) {
	os_kernel_init();
	
	os_new_mutex(&snake, 0);
	
	GLCD_Init();
	
	head = heap + heap_index++;
	tail = heap + heap_index++;
	
	GLCD_SetTextColor(White);
	GLCD_SetBackColor(Black);
	start();
	
	printf("");
	os_add_task(readJoy, NULL, NULL);
	os_add_task(pushButton, NULL, NULL);
	os_add_task(moveSnek, NULL, NULL);
	os_add_task(display, NULL, NULL);
	os_kernel_start();
	while(1){}
}

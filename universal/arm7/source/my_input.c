#include <nds/arm7/input.h>
#include <nds/arm7/touch.h>
#include <nds/system.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>
#include <nds/ipc.h>
#include <nds/ndstypes.h>

void my_touchReadXY(touchPosition *touchPos);
bool my_touchPenDown();

enum{
	KEY_TOUCH = (1<<6),
	KEY_LID = (1<<7)
}Keys;

void my_inputGetAndSend(void){

	// static int sleepCounter = 0;

	touchPosition tempPos = {0};
	FifoMessage msg = {0};

	u16 keys= REG_KEYXY;


	if (!my_touchPenDown()) {
		keys |= KEY_TOUCH;
  	} else {
		keys &= ~KEY_TOUCH;
	}

	msg.SystemInput.keys = keys;

	if (!(keys & KEY_TOUCH)) {
		msg.SystemInput.keys |= KEY_TOUCH;

		my_touchReadXY(&tempPos);	

		if (tempPos.rawx && tempPos.rawy) {
			msg.SystemInput.keys &= ~KEY_TOUCH;
			msg.SystemInput.touch = tempPos;
		}
	}	

	/* if (keys & KEY_LID) 
		sleepCounter++;
	else
		sleepCounter = 0;

	//sleep if lid has been closed for 5 frames
	if (sleepCounter >= 5) 
	{
		systemSleep();
		sleepCounter = 0;
	} */

	msg.type = SYS_INPUT_MESSAGE; //set message type

	fifoSendDatamsg(FIFO_SYSTEM, sizeof(msg), (u8*)&msg);
}
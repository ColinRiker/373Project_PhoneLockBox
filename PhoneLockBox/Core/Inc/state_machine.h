#include "shared.h"

typedef enum {
	SFLAG_NULL,
	SFLAG_ACC_BOX_MOVED,
	SFLAG_ADC_INTERRUPT,
	SFLAG_ADC_MATCH,
	SFLAG_ROTENC_INTERRUPT, //if rotenc button is pressed
	SFLAG_ROTENC_ROTATED, //if rotenc dial is rotated
	SFLAG_TIMER_COMPLETE, //this will be set for whatever time we need dependent on the state
	SFLAG_NFC_PHONE_PRESENT,

	//added flags
	SFLAG_BOX_OPEN,
	SFLAG_BOX_CLOSED,
	SFLAG_AUDIO_VOL_HIGH, //is audio volume high
	SFLAG_AUDIO_MATCH //is there an audio match
} SFlag;





BoxMode runStateMachine(void);
void stateMachineInit(void);
//THESE NEED TO BE MOVED EVENTUALLY
BoxMode timerResolve(void);
BoxMode NFCResolve(void);

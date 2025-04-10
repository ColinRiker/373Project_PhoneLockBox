#include "shared.h"

typedef enum {
	SFLAG_NULL,
	SFLAG_ACC_BOX_MOVED,
	SFLAG_ADC_INTERRUPT,
	SFLAG_ADC_MATCH,
	SFLAG_ROTENC_INTERRUPT,
	SFLAG_ROTENC_ROTATED,
	SFLAG_TIMER_COMPLETE,
	SFLAG_NFC_PHONE_PRESENT,
} SFlag;





BoxMode runStateMachine(void);
void stateMachineInit(void);
//THESE NEED TO BE MOVED EVENTUALLY
BoxMode timerResolve(void);
BoxMode NFCResolve(void);

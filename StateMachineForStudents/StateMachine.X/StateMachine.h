/* 
 * File:   StateMachine.h
 * Author: anca
 *
 * Created on May 19, 2014, 7:13 PM
 */

#ifndef STATEMACHINE_H
#define	STATEMACHINE_H

#ifdef	__cplusplus
extern "C" {
#endif

    
typedef enum
{
    STATE_ONE = 0,
    STATE_TWO,
    STATE_THREE,
    STATE_MAX
} state_e;

typedef enum {
    STATE_OFF = 0,
    STATE_ON_VENT,
    STATE_ON_COOL,
    STATE_ON_HEAT
            
} state_clima;

#ifdef	__cplusplus
}
#endif

#endif	/* STATEMACHINE_H */


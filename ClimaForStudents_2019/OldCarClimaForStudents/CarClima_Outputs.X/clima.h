/* 
 * File:   clima.h
 * Author: Dragos
 *
 * Created on February 2, 2014, 11:03 PM
 */

#ifndef CLIMA_H
#define	CLIMA_H

#ifdef	__cplusplus
extern "C" {
#endif


typedef enum
{
    STATE_OFF = 0,
    STATE_ON_COOL,
    STATE_ON_HEAT,
    STATE_ON_VENT,
    STATE_MAX
} state_e;


#ifdef	__cplusplus
}
#endif

#endif	/* CLIMA_H */


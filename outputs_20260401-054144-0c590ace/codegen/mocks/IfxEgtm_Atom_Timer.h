/* IfxEgtm_Atom_Timer mock */
#ifndef IFXEGTM_ATOM_TIMER_H
#define IFXEGTM_ATOM_TIMER_H
#include "mock_egtm_atom_tmadc_consolidated.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* Types/structs */
typedef struct
{
    Ifx_Priority    isrPriority;       
    IfxSrc_Tos      isrProvider;       
    IfxEgtm_IrqMode irqMode;           
    IfxSrc_VmId     vmId;              
} IfxEgtm_Atom_Timer_Interrupt;

typedef struct
{
    boolean                      enabled;                  
    uint32                       triggerPoint;             
    IfxPort_OutputMode           outputMode;               
    IfxPort_PadDriver            outputDriver;             
    boolean                      risingEdgeAtPeriod;       
    boolean                      outputEnabled;            
    IfxEgtm_Atom_Timer_Interrupt interrupt;                
} IfxEgtm_Atom_Timer_Trigger;

typedef enum
{
    IfxEgtm_Atom_Timer_CountDir_up = 0,         
    IfxEgtm_Atom_Timer_CountDir_upAndDown = 1,  
    IfxEgtm_Atom_Timer_CountDir_down = 2        
} IfxEgtm_Atom_Timer_CountDir;

typedef struct
{
    Ifx_EGTM                   *egtm;                   
    Ifx_EGTM_CLS_ATOM          *atom;                   
    Ifx_EGTM_CLS_ATOM_AGC      *agc;                    
    IfxEgtm_Cluster             clsIndex;               
    IfxEgtm_Atom_Ch             timerChannel;           
    IfxEgtm_Atom_Ch             triggerChannel;         
    uint16                      channelsMask;           
    uint32                      offset;                 
    Ifx_EGTM_CLS_CDTM_DTM      *dtm;                    
    IfxEgtm_Dtm_Ch              dtmChannel;             
    uint32                      agcDisableUpdate;       
    uint32                      agcApplyUpdate;         
    uint32                      period;                 
    boolean                     triggerEnabled;         
    float32                     clockFreq;              
    IfxEgtm_Atom_Timer_CountDir countDir;               
} IfxEgtm_Atom_Timer;

typedef struct
{
    Ifx_EGTM                    *egtm;                 
    IfxEgtm_Cluster              cluster;              
    IfxEgtm_Atom_Ch              timerChannel;         
    IfxEgtm_Atom_ToutMap        *triggerOut;           
    IfxEgtm_Atom_Ch_ClkSrc       clock;                
    IfxEgtm_Dtm_ClockSource      dtmClockSource;       
    float32                      frequency;            
    float32                      minResolution;        
    float32                      startOffset;          
    IfxEgtm_Atom_Timer_CountDir  countDir;             
    IfxEgtm_Atom_Timer_Interrupt interrupt;            
    IfxEgtm_Atom_Timer_Trigger   trigger;              
    boolean                      initPins;             
} IfxEgtm_Atom_Timer_Config;

/* Functions (subset as required) */
boolean IfxEgtm_Atom_Timer_setFrequency(IfxEgtm_Atom_Timer *driver, float32 frequency);
void    IfxEgtm_Atom_Timer_setTrigger(IfxEgtm_Atom_Timer *driver, uint32 triggerPoint);
void    IfxEgtm_Atom_Timer_run(IfxEgtm_Atom_Timer *driver);

/* CPU IRQ helpers often referenced */
void IfxCpu_Irq_installInterruptHandler(void (*handler)(void), int vectabNum, int priority);

#endif /* IFXEGTM_ATOM_TIMER_H */

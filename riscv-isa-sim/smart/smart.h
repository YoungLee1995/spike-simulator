/*addedd by li yang, date 2025-07-02
 */
#ifndef _smart_h
#define _smart_h

class sim_t;
class mem_t;
class bus_t;

class sm_dispatch_t;
class sm_file_t;
class sm_main_t;

extern sm_main_t *g_sm_main;
extern void sm_start(sim_t *sim);

class sm_main_t
{
    sim_t *Sim;
    bus_t *Bus;
    sm_dispatch_t *Dispatch;
    sm_file_t *File;

public:
    sm_main_t(sim_t *sim);
    int tick();
};

class sm_npucore_exit_t
{
public:
    unsigned int Ecode;
    sm_npucore_exit_t(unsigned int ecode)
    {
        Ecode = ecode;
    }
};

class sm_npucore_barrier_t
{
public:
    unsigned int Ecode;
    sm_npucore_barrier_t(){};
   
};

class sm_npucore_full_t
{
public:
    unsigned int Ecode;
    sm_npucore_full_t(){};
};


#endif // _smart_h
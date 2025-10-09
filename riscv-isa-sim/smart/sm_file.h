/*addedd by li yang, date 2025-07-04
 */

#ifndef _sm_file_h_
#define _sm_file_h_
#include "abstract_device.h"
#include "sm_types.h"

class sim_t;
class simif_t;

#define SM_FILE_BLEN 0x100000

class sm_file_t : public abstract_device_t
{
    simif_t *Simif;
    U64 op;
    U64 out;
    U64 in1;
    U64 in2;
    U64 in3;

    char fbuff[SM_FILE_BLEN];

public:
    sm_file_t(simif_t *simif);
    ~sm_file_t();

    bool load(reg_t addr, size_t len, uint8_t *bytes);
    bool store(reg_t addr, size_t len, const uint8_t *bytes);
    reg_t size() { return 0x100000; }
    void do_op();
};

#endif
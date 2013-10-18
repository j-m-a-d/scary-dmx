#include "chaser_effect.h"
#include "utils.h" 
#include "dmx_controller.h"

void *yylval;

int main(int argc, char ** argv)
{
    chaser_t *chaser = NEW_CHASER(chaser);
    chaser_step_t *step = NEW_CHASER_STEP(step);
    dmx_channel_t chs[3] = {15,16,17};
    step->channels = channel_list_from_data(3, chs);
    step->value = 255;
    step->repeat = 5;
    step->speed = 200000;
    chaser->step = step;

    init_dmx();
    start_dmx();
    update_channel(19,255);
    chase(chaser);
    stop_dmx();
    destroy_dmx();
    FREE_CHASER(chaser);
    return 0;
}

#include "daisy_patch_sm.h"
#include "daisysp.h"

#define MAX_DELAY static_cast<size_t>(44100 * 1.f)
#define NUM_GRAINS 1
using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

typedef struct Grain
{
    bool triggered;
    signed long int triggered_counter;
    signed long int length;
    signed long int write_pos;
    signed long int read_pos;
    signed long int offset;
};

DaisyPatchSM patch;
Switch       button;
// Grain        grains[NUM_GRAINS];

Grain grain; 
float DSY_SDRAM_BSS grain_buffer_l[MAX_DELAY];
float DSY_SDRAM_BSS grain_buffer_r[MAX_DELAY];

// float DSY_SDRAM_BSS grain_buffers_l[NUM_GRAINS][MAX_DELAY];
// float DSY_SDRAM_BSS grain_buffers_r[NUM_GRAINS][MAX_DELAY];

float DSY_SDRAM_BSS buffer_l[MAX_DELAY];
float DSY_SDRAM_BSS buffer_r[MAX_DELAY];

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    static int button_pressed = 0;
    static int interval = 0;
    static long int sample_counter = 0;
    static signed long int write_pos = 0;
    static signed long int read_pos = 0;
    static signed long int offset = 0;
    
    patch.ProcessAllControls();
    button.Debounce();

    grain.length = patch.GetAdcValue(CV_1)*44100;
    interval = patch.GetAdcValue(CV_2)*64;
    // patch.WriteCvOut(2, (time_knob*5));
    // out[0][0] = in[0][0]; 
    // out[1][0] = in[1][0]; 
    //--- Write Head --- Always writing 

    if(write_pos > MAX_DELAY - 1) // stop overflow
    {
        write_pos = 0;
    }

    // offset = write_pos ;//- 4400; //TODO 10000 is selectable array offset (delay time)
    // if(offset < 0)
    // {
    //     // patch.WriteCvOut(2, (5.f));
    //     read_pos = offset + MAX_DELAY;
    // }
    // else
    // {
    //     // patch.WriteCvOut(2, (0.f));
    //     read_pos = offset;
    // }
    read_pos = write_pos;
    
    /* write head connected to input */
    buffer_l[write_pos] =  in[0][0]; 
    buffer_r[write_pos] =  in[1][0]; 

    /* increment write position */
    write_pos++;


    if(grain.write_pos > MAX_DELAY - 1) // stop overflow
    {
        grain.write_pos = 0;
    }
    grain_buffer_l[grain.write_pos] = buffer_l[read_pos];
    grain_buffer_r[grain.write_pos] = buffer_r[read_pos];
    grain.read_pos = grain.write_pos;
    grain.write_pos++;

    // out[0][0] = grain_buffer_l[grain.read_pos];
    // out[1][0] = grain_buffer_r[grain.read_pos];

    // /* write to grains */
    // for (int i = 0 ; i < NUM_GRAINS; i++)
    // {
    //     if(grains[i].write_pos > MAX_DELAY - 1) // stop overflow
    //     {
    //         grains[i].write_pos = 0;
    //     }
    //     grain_buffers_l[i][grains[i].write_pos] = buffer_l[read_pos];
    //     grain_buffers_r[i][grains[i].write_pos] = buffer_r[read_pos];
    //     grains[i].write_pos++;
    // }
    // /* write to outputs */
    // int output_l = 0;
    // int output_r = 0;
    // 
    // for (int i = 0 ; i < NUM_GRAINS; i++)
    // {
    // // grains[i].offset = grains[i].write_pos ; //- 10000; //TODO 10000 is selectable array offset (delay time)
    // // if(offset < 0)
    // // {
    // //     // patch.WriteCvOut(2, (5.f));
    // //     grains[i].read_pos = grains[i].offset + MAX_DELAY;
    // // }
    // // else
    // // {
    // //     // patch.WriteCvOut(2, (0.f));
    // //     grains[i].read_pos = grains[i].offset;
    // // }
    //     grains[i].read_pos = grains[i].write_pos;
    // 
    //     output_l += grain_buffers_l[i][grains[i].read_pos];
    //     output_r += grain_buffers_r[i][grains[i].read_pos];
    // }
    // out[0][0] = output_l;
    // out[1][0] = output_r;


    // /* output connected to read head   */
    // out[0][0] = buffer_l[read_pos];
    // out[1][0] = buffer_r[read_pos];

    if(sample_counter > (int)MAX_DELAY - 1) // stop overflow
    {
        sample_counter = 0;
    }

    if(sample_counter++ % ((int)MAX_DELAY/interval) == 0)
    {
       grain.triggered = 1;
    }

    if(grain.triggered)
    {
        grain.triggered = 0;
        grain.triggered_counter = grain.length;
    }

    if(grain.triggered_counter)
    {
        grain.triggered_counter--;
        patch.WriteCvOut(2, 5.f);
        out[0][0] = grain_buffer_l[grain.read_pos];
        out[1][0] = grain_buffer_r[grain.read_pos];
    }
    else
    {
        patch.WriteCvOut(2, 0.f);
    }


    if(button.RisingEdge())
    {
        if(button_pressed == 0)
        {
            button_pressed = 1;
            // patch.WriteCvOut(2, (5.f));
        }
        else if(button_pressed == 1)
        {
            button_pressed = 0; 
            // patch.WriteCvOut(2, (0.f));
        }
    }
}

int main(void)
{
    //grains[0].Init(&grain1_buffer_l[0], &grain1_buffer_r[0]);
    // for (int i = 0; i < NUM_GRAINS; i++)
    // {
    //     grains[i].write_pos = 0;
    //     grains[i].read_pos = 0;
    //     grains[i].offset = 0;
    // }
    grain.write_pos = 0;
    grain.read_pos = 0;
    grain.offset = 0;
    grain.triggered = 0;
    grain.triggered_counter = 0;
    patch.Init(); 
    patch.SetAudioBlockSize(1);
    button.Init(patch.D5);
    patch.StartAudio(AudioCallback);
    while(1) {}
}
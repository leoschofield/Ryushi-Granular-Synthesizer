#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

#define MAX_DELAY static_cast<size_t>(44100 * 1.f)

DaisyPatchSM patch;
Switch       button;
// DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delMems[2];
// DelayLine<float, 4410> DSY_SDRAM_BSS grain[2];
float DSY_SDRAM_BSS buffer_l[MAX_DELAY];
float DSY_SDRAM_BSS buffer_r[MAX_DELAY];

float delayTimeInSeconds = 0.1f; 
size_t delayTimeInSamples = static_cast<size_t>(delayTimeInSeconds * MAX_DELAY); 

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    static int button_pressed = 0;
    static signed long int write_pos = 0;
    static signed long int read_pos = 0;
    static signed long int offset = 0;
    patch.ProcessAllControls();
    button.Debounce();

    // float time_knob = patch.GetAdcValue(CV_1);
    // patch.WriteCvOut(2, (time_knob*5));
    // out[0][0] = in[0][0]; 
    // out[1][0] = in[1][0]; 
    //--- Write Head --- Always writing 

    if(write_pos > MAX_DELAY - 1) // stop overflow
    {
        write_pos = 0;
    }

    offset = write_pos - 10000; //TODO 10000 is selectable array offset (delay time)
    if(offset < 0)
    {
        // patch.WriteCvOut(2, (5.f));
        read_pos = offset + MAX_DELAY;
    }
    else
    {
        // patch.WriteCvOut(2, (0.f));
        read_pos = offset;
    }
    
    buffer_l[write_pos] =  in[0][0]; 
    buffer_r[write_pos] =  in[1][0]; 

    out[0][0] = buffer_l[read_pos];
    out[1][0] = buffer_r[read_pos];
    write_pos++;

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
    // delMems[0].Init();
    // delMems[0].SetDelay(delayTimeInSamples);
    // delMems[1].Init();
    // delMems[1].SetDelay(delayTimeInSamples);


    
    patch.Init(); 
    patch.SetAudioBlockSize(1);
    button.Init(patch.D5);
    patch.StartAudio(AudioCallback);
    while(1) {}
}
# Cut-The-Moog

This repository contains the source code of an audio VST3/AU plugin which emulates the behavior of the Lockhart Wavefolder and the Moog Filter.

The audio chain of the plugin consists on:

* An input section containing:
  * Trim pot -> controls the amount of input signal between -24dB and 24dB
* Wavefolder section containing:
  * Folding pot -> it controls the amount of folding, which inreases the overtones of the input signal
  * Offset pot -> it controls the amount of an input dc offset to the wavefolder. The more you increase the offset the more the signal clips
  * D/W pot -> it controls the amount of folding signal with respect to the clean one
* Filter section containing: 
  * Cutoff pot -> it controls the cutoff frequency of the Moog-based low pass filter
  * Morph pot -> it permits to change the tipe of filter, from Bessel low-pass to Butterworth low-pass, from Moog low-pass to Octave-Cat low-pass
  * Resonance pot -> it controls the resonance of the filter, which is the quality factor of the filter.
* Output section containing:
  * Output pot -> it controls the amount of output signal between -60dB and 6dB 

Download MacOS:
 * [AU plugin](https://drive.google.com/uc?export=download&id=1OroDlSELorLpR82vJHPjaWrzzKsgbA8W)
 * [VST3 plugin](https://drive.google.com/uc?export=download&id=1mCmWtuK-fVcT0dIMtwz-wgD231QbUyB8)
 * [VST plugin](https://drive.google.com/uc?export=download&id=1_3B7-AkmitZzfMehImO-j-4HgkrvAUio)

Download Windows:
 * [VST3 plugin](https://drive.google.com/uc?export=download&id=1Cms3PnxrrzkKDk5a6heP28QUBpxUAanJ)
 * [VST plugin](https://drive.google.com/uc?export=download&id=1l15Ir1IYiwXwX_kQgIk3VwuQKBnXTn25)

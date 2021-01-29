# CutTheMooog
[![Download Latest](https://img.shields.io/badge/download-latest-blue.svg)](https://github.com/maurizdeb/CutTheMoog/releases/latest)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-brightgreen.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate?hosted_button_id=W7VEU56ZXZXFA)

CutTheMoog is an audio plugin which emulates the behavior of the Lockhart Wavefolder and the Moog Filter.

CutTheMog is made and distributed by maurizdeb a.k.a. KarotaSound.

CutTheMoog is an open-source project. If you like it and you want KarotaSound to develop other plugins, please [DONATE HERE](https://www.paypal.com/donate?hosted_button_id=W7VEU56ZXZXFA).

Any comment, advice or contribution is kindly accepted! 

## Description

![Pic](https://github.com/maurizdeb/CutTheMoog/blob/master/Screenshot/Screenv1.0.png)

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

## Installation

You can download the latest release [here](https://github.com/maurizdeb/CutTheMoog/releases/latest).
CutTheMoog is available for macOs, Windows and Linux in the following formats:

* VST
* VST3
* AU

## License

CutTheMoog is licensed under the General Public License agreement.

# Open Movement file converter

## Obtaining the binary

### Binary (Windows)

A pre-built binary for Windows is available at: [OpenMovement GitHub](https://github.com/digitalinteraction/openmovement/blob/master/Downloads/AX3/AX3-Utils-Win-3.zip?raw=true).  Alternatively, if you have installed *OM-GUI*, you should find it at: `"%ProgramFiles(x86)%\Open Movement\OM GUI\Plugins\OmConvertPlugin\omconvert.exe"`.  

### Build (Windows)

Alternatively, on Windows, you can build an `omconvert.exe` binary in the current directory.  You will need *Visual Studio*, or the [Build Tools for Visual Studio](https://aka.ms/buildtools) (*All Downloads* / *Tools for Visual Studio* / *Build Tools for Visual Studio*).  <!-- Ensure you have the compilers installed: *Individual Components* / *Compilers, build tools, and runtimes* / *MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)* and *MSVC v143 - VS 2022 C++ x64/x86 Spectre-mitigated libs (Latest)*. --> Build with:

```cmd
powershell -Command "& {Invoke-WebRequest https://github.com/digitalinteraction/omconvert/archive/master.zip -o omconvert.build.zip ; Expand-Archive omconvert.build.zip ; del omconvert.build.zip ; omconvert.build/omconvert-master/src/omconvert/build.cmd ; copy omconvert.build/omconvert-master/src/omconvert/omconvert.exe . }"
```

Alternatively, you can clone this repository and run: `src/omconvert/build.cmd`

### Build (Non-Windows)

On other operating systems, such as Mac (*XCode* required), Linux, or WSL, you can use this single line command to build an `omconvert` binary in the current directory:

```bash
mkdir omconvert.build && curl -L https://github.com/digitalinteraction/omconvert/archive/master.zip -o omconvert.build/master.zip && unzip omconvert.build/master.zip -d omconvert.build && make -C omconvert.build/omconvert-master/src/omconvert && cp omconvert.build/omconvert-master/src/omconvert/omconvert .
```

Alternatively, you can clone this repository and run: `make -C src/omconvert`

## Analysis methods

For full help showing all of the algorithm command-line parameters, use the parameter `--help`.  Note that there is a default `-calibrate 1` option for auto-calibration. 

Several of the built-in analysis methods are described in more detail in [AX3-GUI: Analysis Toolbar](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI#analysis-toolbar).


### Vector Magnitude

The mean acceleration vector magnitude minus static gravity (over an epoch) is a commonly-used, simple to calculate, metric.  Typically known as *SVM-1* (Signal Vector Magnitude, typically averaging the *absolute* value of negative numbers)*, or the *ENMO* (Euclidean Norm Minus One, typically treating negative values as zero).

A band-pass filter is typically used to remove any slight miscalibration errors.

An example to produce an analysis of Signal Vector Magnitude minus one for 60-second epochs (the default), using a 4 Butterworth band-pass filter 0.5-20Hz.

```bash
omconvert datafile.cwa -svm-file datafile.svm.csv -svm-epoch 60 -svm-filter 1
```

To use the data without a filter, use `-svm-filter 0`.

For any interrupts, where there are no valid samples in the epoch, *NaN* can be emitted 
for the values:  use the parameter `-svm-extended`, where 1=emit zero, 2=empty cell, 3="nan".

To take the *absolute* of negative values (default) use `-svm-mode 0`, for each epoch this is:

```
SUM[ abs(sqrt(Xi^2 + Yi^2 + Zi^2) - 1) ] / count
```

...to treat negative values as *zero* use `-svm-mode 1`, for each epoch this is:

```
SUM[ max(sqrt(Xi^2 + Yi^2 + Zi^2) - 1, 0) ] / count
```


### Cut-Points

The cut-point algorithm takes the mean vector magnitude value mins one as an absolute value 
(controlled by the `-paee-filter` option as above) for one-minute intervals, then classifies them into 
intervals defined by *cut-points*.  These classifications are then output for the 
number of minute epochs defined by the `-paee-epoch` option.

```bash
omconvert datafile.cwa -paee-file datafile.paee.csv
```

The cut-point values are always based on the *mean* value. Some papers quote the summed 
values from the sample-rate and/or every second of the epoch, and these must be normalized. 
You can specify the cut-point value to use with the `-paee-model` option with the values below
(up to three values defining four intervals, and you may use divide operations to normalize the values):

From "Validation of the GENEA Accelerometer" by Esliger et al. (summed at 80Hz for 60 seconds), 
where Sedentary, Light (>= 1.5 METS, < 4 METS), Moderate (>= 4 METS, < 7 METS), Vigorous (>= 7 METS):

```
Esliger(40-63)-wristR:  386/80/60 440/80/60 2099/80/60
Esliger(40-63)-wristL:  217/80/60 645/80/60 1811/80/60
Esliger(40-63)-waist:    77/80/60 220/80/60 2057/80/60
```

Based on similar data, and as used in *GENEA* software calculation:

```
'wrist':                386/80/60 542/80/60 1811/80/60
```

From Schaefer et al ("Establishing and Evaluating Wrist Cutpoints for the GENEActiv Accelerometer in Youth")
(6-11 years) Sedentary, Light (>= 1.5 METS, < 3 METS), Moderate (>= 3 METS, < 6 METS), Vigorous (>= 6 METS):

```
Schaefer(6-11): 0.190 0.314 0.998
```

From Phillips et al ("Calibration of the GENEA accelerometer for assessment of physical activity intensity in children")
(8-14 years old). Sedentary, Light (>= 1.5 METS, < 3 METS), Moderate (>= 3 METS, < 6 METS), Vigorous (>= 6 METS):

```
Phillips(8-14)-wristR:   6/80 22/80 56/80
Phillips(8-14)-wristL:   7/80 20/80 60/80
Phillips(8-14)-hip:      3/80 17/80 51/80
```

From Roscoe et al ("Calibration of GENEActiv accelerometer wrist cut-points for the assessment of physical activity intensity of preschool aged children.")
(4-5 years old). Sedentary, Light (>= 2 METS , < 3 METS) , Moderate and above (>= 3 METS, < 6 METS), NO Vigorous

```
Roscoe(4-5)-wristND:     5.3/87.5 8.6/87.5
Roscoe(4-5)-wristD:      8.1/87.5 9.3/87.5
```

From Dillon et al ("Number of Days Required to Estimate Habitual Activity Using Wrist-Worn GENEActiv Accelerometer: A Cross-Sectional Study", 2016)
...cut points appear to be re-targeted and the same as from "Criterion validity and calibration of the GENEActiv accelerometer in adults" (2015).
Sedentary, Light (>= 1.5 METS, < 3 METS), Moderate (>= 3 METS, < 6 METS), Vigorous (>= 6 METS)

```
Dillon-wristD:  230/30/60 338/30/60 714/30/60
Dillon-wristND: 190/30/60 314/30/60 594/30/60
```

From Powell et al ("Simultaneous validation of five activity monitors for use in adult populations")
(mean age 39.9 +/-11.5 years). Sedentary, Light (>= 1.5 METS, < 3 METS), Moderate (>= 3 METS, < 6 METS), Vigorous (>= 6 METS):

```
Powell-wristD:           51/30/15 68/30/15 142/30/15
Powell-wristND:          47/30/15 64/30/15 157/30/15
```


### Wear-Time Validation

The wear-time calculation is based on the method by van Hees et al in PLos ONE 2011 6(7), 
"Estimation of Daily Energy Expenditure in Pregnant and Non-Pregnant Women Using a Wrist-Worn Tri-Axial Accelerometer".
Accelerometer non-wear time is estimated from the standard deviation and range of each accelerometer axis, 
calculated for consecutive blocks of 30 minutes.
A block was classified as non-wear time if the standard deviation was less than 3.0 mg 
(1 mg = 0.00981 m*s-2) for at least two out of the three axes,
or if the value range, for at least two out of three axes, was less than 50 mg.

An example to produce an analysis of wear-time:

```bash
omconvert datafile.cwa -wtv-file datafile.wtv.csv
```

### Sleep

This is an implementation of the "Estimation of Stationary Sleep-segments" approach from
Marko Borazio, Eugen Berlin, Nagihan Kucukyildiz, Philipp M. Scholl and Kristof Van Laerhoven:
"Towards a Benchmark for Wearable Sleep Analysis with Inertial Wrist-worn Sensing Units", 
ICHI 2014, Verona, Italy, IEEE Press, 2014.  Use: 

```bash
omconvert datafile.cwa -sleep-file datafile.sleep.csv
```

For more advanced uses, you may find the [omsummary](https://github.com/digitalinteraction/openmovement/blob/master/Downloads/AX3/omsummary.md) tool useful.


### AG Counts

There is an implementation of the method from *Frequency filtering and the aggregation of raw accelerometry into Actigraph counts.* (Jan Brond, Daniel Arvidsson, Lars Bo Andersen. 5th International Conference on Ambulatory Monitoring of Physical Activity and Movement, ICAMPAM June 2017) to convert the raw accelerometer values to *AG counts*.  The data must be resampled at 30Hz for this algorithm to work correctly:

```bash
omconvert datafile.cwa -resample 30 -interpolate-mode 1 -counts-epoch 1 -counts-file datafile.counts.csv
```

Where the `-counts-epoch` is in seconds.  The standard output is a simple .CSV file with the counts for the x/y/z axes, but you can also use the option `-csv-format:ag` to create an output file compatible with "ActiGraph(tm) ActiLife" software, and `-csv-format:agdt` to create an output file compatible with "ActiGraph(tm) ActiLife Data Table format".


### AG Raw

You can export a .CWA file at 30Hz in a .CSV file with a header that makes it compatible with "ActiGraph(tm) ActiLife" software, then use that software to calculate the counts.

```bash
omconvert datafile.cwa -resample 30 -interpolate-mode 1 -csv-format:ag -csv-file datafile.ag.csv
```

If you have the "pre-built binary for Windows" (from the link above, unzipped to a folder), then you can simply drag your `.CWA` file(s) over the batch file `_cwa-to-counts.cmd` to generate a 1 minute epoch counts summary.  Alternatively, use the file `_cwa-to-counts+raw.cmd` to also generate a 30 Hz raw export (which may take a while as it could be a large file) for use in ActiLife directly.


### Pedometer / Step Counter

There is an implementation of a step count algorithm for a wrist-worn triaxial accelerometer based on the method from *Design and Implementation of Practical Step Detection Algorithm for Wrist-Worn Devices* (Yunhoon Cho, Hyuntae Cho, and Chong-Min Kyung. IEEE Sensors Journal, vol. 16, no. 21, pp. 7720-7730, Nov.1, 2016. doi: 10.1109/JSEN.2016.2603163). The data must be resampled at 20Hz for this algorithm to work correctly:

```bash
omconvert datafile.cwa -resample 20 -interpolate-mode 1 -step-epoch 1 -step-file datafile.step.csv
```

Where the `-step-epoch` is in seconds.  The output file is a simple .CSV file with the timestamp of the start of the epoch (`YYYY-MM-DD hh:mm:ss`), step count for the epoch, and total cumulative step count.

If you have the "pre-built binary for Windows" (from the link above, unzipped to a folder), then you can simply drag your `.CWA` file(s) over the batch file `_cwa-to-steps.cmd` to generate a 1 minute epoch counts summary.


## File conversion

.CWA files are designed to be suitable as a file format on-board the AX3: efficient, suitable for pausing and resuming collections, error detection and recovery, and preserving the underlying accelerometer sensor's slightly variable sample rate (with more precise timestamps for reconstruction of per-sample timing). 
The format is described in [cwa.h](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/cwa.h). 
If you are interested in exporting .CWA file data while preserving their underlying, slightly variable sample-rate (no resampling), see [cwa-convert](https://github.com/digitalinteraction/openmovement/blob/master/Software/AX3/cwa-convert/c/README.md). 

The *omconvert* conversion program resamples the input raw `.CWA` file (which records data at the naturally slightly-variable rate of the underlying sensor) based on the more precise timestamps that up to one sample per sector are tagged with. 
This is suitable where processing algorithms require evenly-sampled data. 

The conversion process can be run as follows to generate a .WAV file and informational metadata file:

```bash
./omconvert datafile.cwa -interpolate-mode 1 -out datafile.wav -info datafile.yml
```

Where `-interpolate-mode 1` selectes nearest-neighbour, `2` for linear interpolation, `3` for cubic interpolation.

For details of the .WAV file the metadata output, see: [omconvert technical details](src/omconvert/README.md).

If you have multiple devices on the same body over a significant time, you may also be interested in [timesync](https://github.com/digitalinteraction/timesync/), which will synchronize data collected from multiple devices.

You may convert the data to a .CSV file (note that the output may be very large):

```bash
omconvert datafile.cwa -csv-file datafile.csv
```

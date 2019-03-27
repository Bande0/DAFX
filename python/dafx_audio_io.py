#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Jul 18 09:11:41 2018

@author: andraspalfi
"""
import numpy as np
from scipy.io import wavfile
import aifc


def convert_to_float32(x):
    """
    Converts numpy arrays to float32 type.
    Single and multi-channel arrays are supported.

    Parameters
    ----------
    input data array. Supported input types:
        - numpy.int16
        - numpy.int32
        - numpy.float32
        - numpy.float64

    Returns:
    --------
    Input data array converted to np.float32

    Raises:
    -------
    TypeError
        if input data type is neither int16, int32, float32 of float64

    """
    int16_max = 2**15 - 1
    int32_max = 2**31 - 1

    if(x.shape.__len__() > 1):
        x_res = np.zeros(x.shape, dtype=np.float32)
        print('{}-channel audio'.format(x.shape[1]))
        for n in range(x.shape[1]):
            x_res[:, n] = convert_to_float32(x[:, n])
        return x_res
    else:
        if type(x[0]) == np.int16:
            return x.astype('float32') / int16_max
        elif type(x[0]) == np.int32:
            return x.astype('float32') / int32_max
        elif type(x[0]) == np.float64:
            return x.astype('float32')
        elif type(x[0]) == np.float32:
            return x
        else:
            raise TypeError('Input type has to be int16, int32,'
                            ' float32 or float64!')


def convert_to_int16(x):
    """
    Converts numpy arrays to int16 type.
    Single and multi-channel arrays are supported.

    Parameters
    ----------
    input data array. Supported input types:
        - numpy.int16
        - numpy.int32
        - numpy.float32
        - numpy.float64

    Returns:
    --------
    Input data array converted to np.int16

    Raises:
    -------
    TypeError
        if input data type is neither int16, int32, float32 of float64
    """
    int16_max = 2**15 - 1
    int32_max = 2**31 - 1

    if(x.shape.__len__() > 1):
        x_res = np.zeros(x.shape, dtype=np.int16)
        print('{}-channel audio'.format(x.shape[1]))
        for n in range(x.shape[1]):
            x_res[:, n] = convert_to_int16(x[:, n])
        return x_res
    else:
        if type(x[0]) == np.int16:
            return x
        elif type(x[0]) == np.int32:
            return (x * (float(int16_max) / float(int32_max))).astype('int16')
        elif type(x[0]) == np.float32:
            return (x * int16_max).astype('int16')
        elif type(x[0]) == np.float64:
            return (x * int16_max).astype('int16')
        else:
            raise TypeError('Input type has to be int16, int32,'
                            ' float32 or float64!')


def read_audio(filename):
    """
    Reads in an audio file and returns it as a float32 numpy array.
    Supports .wav and .aif files.
    Also handles if an .aif file is accidentally saved as a .wav extension

    Parameters:
    -----------
    filename:
        full path to the audiofile to be read

    Returns:
    -----------
    (fs, audio_float):
        fs: sampling rate of audio file
        audio_float: np.float32 type array with audio samples

    Raises:
    -----------
    ValueError
    """
    # if audio file is a wav file, try block will succeed
    try:
        fs, x = wavfile.read(filename)
        print(".wav file read: {0}".format(filename.split('/')[-1]))
        audio_float = convert_to_float32(x)
        return (fs, audio_float)

    # if the audio file is AIFF, except block will execute
    except ValueError as err:
        err_str = err.args[0]

        if (err_str == "File format b'FORM'... not understood."):
            print(".aiff file read: {0}".format(filename.split('/')[-1]))

            af = aifc.open(filename, 'r')
            aiff_data = af.readframes(af.getnframes())
            fs = af.getframerate()

            dt = np.dtype('i2')
            dt = dt.newbyteorder('>')
            aiff_int = np.frombuffer(aiff_data, dtype=dt)

            audio_float = convert_to_float32(aiff_int)
            return (fs, audio_float)

        else:
            raise ValueError


def write_audio(x, fs, filename, form='i16'):
    """write_audio(x, filename, form='i16'):

    Exports a waveform as an audio file in various formats.

    Parameters:
    -----------
    x: numpy array
        the audio data to be written

    fs: int
        sampling rate of the input data

    filename: str
        full path to the audiofile to be read

    form: str
        'i16' for 16-bit int PCM .wav - This is the default option
        'f32' for 32-bit float .wav

    Raises:
    ------
    TypeError:
        if the 'form' parameter is not equal to either 'i16' or 'f32'
    """
    if(form == 'i16'):
        out_data = convert_to_int16(x)
    elif(form == 'f32'):
        out_data = convert_to_float32(x)
    else:
        raise TypeError("The 'form' parameter has to be either 'i16' or 'f32'!")

    wavfile.write(filename, fs, out_data)

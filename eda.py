import os
from tqdm import tqdm
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from math import ceil, floor
from python_speech_features import mfcc, logfbank
import librosa

def plot_signals(signals, quantidade_locutores):
    fig, axes = plt.subplots(nrows=1, ncols=quantidade_locutores, sharex=False,
                             sharey=True, figsize=(20,5))
    fig.suptitle('Time Series', size=16)
    i = 0
    for x in range(1):
        for y in range(quantidade_locutores):
            axes[y].set_title(list(signals.keys())[i])
            axes[y].plot(list(signals.values())[i])
            axes[y].get_xaxis().set_visible(False)
            axes[y].get_yaxis().set_visible(False)
            i += 1

def plot_fft(fft, quantidade_locutores):
    fig, axes = plt.subplots(nrows=1, ncols=quantidade_locutores, sharex=False,
                             sharey=True, figsize=(20,5))
    fig.suptitle('Fourier Transforms', size=16)
    i = 0
    for x in range(1):
        for y in range(quantidade_locutores):
            data = list(fft.values())[i]
            Y, freq = data[0], data[1]
            axes[y].set_title(list(fft.keys())[i])
            axes[y].plot(freq, Y)
            axes[y].get_xaxis().set_visible(False)
            axes[y].get_yaxis().set_visible(False)
            i += 1

def plot_fbank(fbank, quantidade_locutores):
    fig, axes = plt.subplots(nrows=1, ncols=quantidade_locutores, sharex=False,
                             sharey=True, figsize=(20,5))
    fig.suptitle('Filter Bank Coefficients', size=16)
    i = 0
    for x in range(1):
        for y in range(quantidade_locutores):
            axes[y].set_title(list(fbank.keys())[i])
            axes[y].imshow(list(fbank.values())[i],
                    cmap='hot', interpolation='nearest')
            axes[y].get_xaxis().set_visible(False)
            axes[y].get_yaxis().set_visible(False)
            i += 1

def plot_mfccs(mfccs, quantidade_locutores):
  fig, axes = plt.subplots(nrows=1, ncols=quantidade_locutores, sharex=False,
                            sharey=True, figsize=(20,5))
  fig.suptitle('Mel Frequency Cepstrum Coefficients', size=16)
  i = 0
  for x in range(1):
    for y in range(quantidade_locutores):
      axes[y].set_title(list(mfccs.keys())[i])
      axes[y].imshow(list(mfccs.values())[i],
              cmap='hot', interpolation='nearest')
      axes[y].get_xaxis().set_visible(False)
      axes[y].get_yaxis().set_visible(False)
      i += 1

def compute_fft(signal, rate):
  signal_length = len(signal)
  # d = amount of time between each sample on the graph T = 1 / f
  frequency_component = np.fft.rfftfreq(signal_length, d = 1 / rate)
  # magnitude_component = complex number resulting from fft calculations
  magnitude_component = abs(np.fft.rfft(signal) / signal_length)
  return (magnitude_component, frequency_component)

def envelope(signal, rate, threshold):
  mask = []
  # applying a function to the series
  y = pd.Series(signal).apply(np.abs)
  y_mean = y.rolling(window = int(rate / 10), min_periods = 1, center = True).mean()
  for mean in y_mean:
    if (mean > threshold):
      mask.append(True)
    else:
      mask.append(False)
  return mask

def hamming_function(N, k):
  return 0.54 - 0.46 * cos((2.0 * np.pi * k) / (N - 1))

def apply_window(signal):
  N = len(signal)
  for k in range(N):
    signal[k] = hamming_function(N, k) * signal[k]
  return signal

def main():
  df = pd.read_csv('dados.csv')
  df.set_index('fname', inplace=True)

  for f in df.index:
    rate, signal = wavfile.read('wavfiles/' + f)
    df.at[f, 'length'] = signal.shape[0] / rate

  print('Done')
  print(df)

  classes = list(np.unique(df.label))
  print(classes)

  quantidade_locutores = len(classes)

  classes_dist = df.groupby(['label'])['length'].mean()
  print(classes_dist)

  fig, ax = plt.subplots()
  ax.set_title('Class Distribution', y = 1.05)
  ax.pie(classes_dist, labels = classes_dist.index, autopct = '%1.1f%%',
                    shadow = False, startangle = 90)
  ax.axis('equal')
  # plt.show()
  df.reset_index(inplace=True)

  signals = {}
  fft = {}
  filterbank_energies = {}
  mfccs = {}

  for c in classes:
    print('Class: ', c)
    wav_file = df[df.label == c].iloc[0, 0]
    print('arq:', wav_file)
    rate, signal = wavfile.read('wavfiles/' + wav_file)
    print('Rate is ', rate)
    ## you can mess around with the threshold to see what fits best
    mask = envelope(signal, rate, 0.0005)
    # re-indexing our signal with the mask
    # the idea is noise-filtering/canceling
    signal = signal[mask]
    signals[c] = signal
    fft[c] = compute_fft(signal, rate)

    nfft_size = int(ceil(rate / 40.0))
    # nfft = rate / 40
    bank = logfbank(signal[:rate], rate, nfilt=26, nfft=nfft_size).T
    filterbank_energies[c] = bank

    mel = mfcc(signal[:rate], rate, numcep=13, nfilt=26, nfft=nfft_size).T
    mfccs[c] = mel

  print('Plotting signals..')
  plot_signals(signals, quantidade_locutores)
  # plt.show()

  print('Plotting fft..')
  plot_fft(fft, quantidade_locutores)
  # plt.show()

  print('Plotting filterbanks')
  plot_fbank(filterbank_energies, quantidade_locutores)
  # plt.show()

  print('Plotting mfccs')
  plot_mfccs(mfccs, quantidade_locutores)
  # plt.show()

  print('Creating clean samples')
  if (len(os.listdir('clean')) == 0):
    for f in tqdm(df.fname):
      print('file: ', f)
      # downsampling. there's not alot of data in the high frequencies
      # we can therefore compact our data by only taking what matters
      # which is located on the lower frequencies.
      rate, signal = wavfile.read('wavfiles/' + f)
      # signal, rate = librosa.load('wavfiles/' + f, sr=16000)
      mask = envelope(signal, rate, 0.01)
      wavfile.write(filename = 'clean/' + f, rate = rate, data = signal[mask])

if __name__ == '__main__':
  main()
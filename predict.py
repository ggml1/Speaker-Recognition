import pickle
import os
import time
import numpy as np
from tqdm import tqdm
from scipy.io import wavfile
from python_speech_features import mfcc
from keras.models import load_model
import pandas as pd
from sklearn.metrics import accuracy_score
import sounddevice as sd

def build_predictions(audio_directory, config, model):
  # look at this directory and make predictons on
  # all the audio files contained in it
  y_true = [] # normally we do not have this. pred on same stuff
  y_pred = [] #
  filename_prob = {}

  print('Extracting features from audio')
  for filename in tqdm(os.listdir(audio_directory)):
    rate, wav = wavfile.read(os.path.join(audio_directory + filename))
    y_prob = []

    for i in range(0, wav.shape[0] - config.step, config.step):
      sample = wav[i:i + config.step]
      X = mfcc(sample, rate, numcep=config.nfeat, nfilt=config.nfilt, nfft=config.nfft)
      X = (X - config._min) / (config._max - config._min)
      if (config.mode == 'conv'):
        X = X.reshape(1, X.shape[0], X.shape[1], 1)
      elif (config.mode == 'time'):
        X = np.expand_dims(X, axis = 0)
      y_hat = model.predict(X) # feed this sample to the model. it is the
      # probability of being each one of the classes
      y_prob.append(y_hat)
      y_pred.append(np.argmax(y_hat))

    filename_prob[filename] = np.mean(y_prob, axis = 0).flatten()

  return y_true, y_pred, filename_prob

def main():

  df = pd.read_csv('dados.csv')
  classes = list(np.unique(df.label))

  ## MIC INPUT
  duration = 5  # seconds
  sample_rate = 16000
  while (True):
    print('Press ENTER to begin recording')
    bla = input()
    my_recording = sd.rec(duration * sample_rate, samplerate=sample_rate, channels=1)
    for i in tqdm(range(5)):
      time.sleep(1)
    # scaled_data = np.int16(my_recording / np.max(np.abs(my_recording)) * 32767)
    wavfile.write('user_samples/test.wav', 16000, my_recording)
    print('Do you want to check the recording? Y/N')
    option = input()
    if (option == 'Y' or option == 'y'):
      os.system("aplay user_samples/test.wav")
      print('Would you link to continue with this sample? Y/N')
      option = input()
      if (option == 'Y' or option == 'y'):
        break
    else:
      break

  p_path = os.path.join('pickles', 'conv.p')

  with open(p_path, 'rb') as handle:
    config = pickle.load(handle)

  model = load_model(config.model_path)

  y_true, y_pred, filename_prob = build_predictions('user_samples/', config, model)

  y_probs = filename_prob['test.wav']
  for c, p in zip(classes, y_probs):
    print('Classe: ' + str(c) + ' {}'.format(p))
  resp = np.argmax(y_probs)

  print('The voice belongs to speaker: ', classes[resp])

if __name__ == '__main__':
  main()

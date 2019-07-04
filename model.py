import os
from scipy.io import wavfile
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import pickle
from keras.layers import Conv2D, MaxPool2D, Flatten, LSTM, GlobalAveragePooling2D
from keras.layers import Dropout, Dense, TimeDistributed
from keras.models import Sequential
from keras.callbacks import ModelCheckpoint
from keras.utils import to_categorical
from sklearn.utils.class_weight import compute_class_weight
from tqdm import tqdm
from python_speech_features import mfcc
from cfg import Config

def check_data(config):
  if (os.path.isfile(config.p_path)):
    print('Loading existing data for {} model.'.format(config.mode))
    with open(config.p_path, 'rb') as handle:
      tmp = pickle.load(handle)
      return tmp
  else:
    return None

def build_rand_feat(class_dist, prob_dist, n_samples, df, config, classes):
  tmp = check_data(config)
  if (tmp):
    # the saved X and y we had previously calculated from the training set
    return tmp.data[0], tmp.data[1]
  X = []
  y = []
  _min, _max = float('inf'), -float('inf')
  # we track these values so we can normalize our inputs to be between 0 and 1
  for file in tqdm(df.index):
    rate, wave = wavfile.read('clean/' + file)
    label = df.at[file, 'label']
    print('Got: ', wave.shape[0])
    for idx in range(0, wave.shape[0] - config.step, config.step):
      wave_sample = wave[idx:idx + config.step]
      X_sample = mfcc(wave_sample, rate, numcep=config.nfeat, nfilt=config.nfilt, nfft=config.nfft) # trust the guy on that
      _min = min(_min, np.amin(X_sample))
      _max = max(_max, np.amax(X_sample))
      X.append(X_sample)
      y.append(classes.index(label))
  got = len(X)
  for _ in tqdm(range(n_samples - got)):
    rand_class = np.random.choice(class_dist.index, p = prob_dist)
    file = np.random.choice(df[df.label == rand_class].index) # index == filename
    rate, wave = wavfile.read('clean/' + file)
    label = df.at[file, 'label']
    rand_index = np.random.randint(0, wave.shape[0] - config.step) # subtract so we dont exceed the size of the shape by summing the step. perhaps -1 too?
    sample = wave[rand_index:rand_index + config.step]
    X_sample = mfcc(sample, rate, numcep=config.nfeat, nfilt=config.nfilt, nfft=config.nfft) # trust the guy on that
    # print(len(X_sample))
    _min = min(_min, np.amin(X_sample))
    _max = max(_max, np.amax(X_sample))
    X.append(X_sample)
    y.append(classes.index(label))
  config._min = _min
  config._max = _max
  X = np.array(X)
  y = np.array(y)
  X = (X - _min) / (_max - _min) # normalizing the values
  if (config.mode == 'conv'):
    X = X.reshape(X.shape[0], X.shape[1], X.shape[2], 1) # n samples (13) by 9 x 1 (check it later)
  elif (config.mode == 'time'):
    X = X.reshape(X.shape[0], X.shape[1], X.shape[2])
  y = to_categorical(y, num_classes=10)
  config.data = (X, y)

  with open(config.p_path, 'wb') as handle:
    pickle.dump(config, handle, protocol = 2) # backwards compatibility with pypy2

  return X, y

def get_conv_model(input_shape):
  model = Sequential()
  model.add(Conv2D(16, (3, 3), activation = 'relu', strides = (1, 1), padding = 'same', input_shape = input_shape)) # 16 filters, 3x3 convo
  model.add(Conv2D(32, (3, 3), activation = 'relu', strides = (1, 1), padding = 'same', input_shape = input_shape))
  model.add(Conv2D(64, (3, 3), activation = 'relu', strides = (1, 1), padding = 'same', input_shape = input_shape))
  model.add(Conv2D(128, (3, 3), activation = 'relu', strides = (1, 1), padding = 'same', input_shape = input_shape))
  model.add(MaxPool2D((2, 2)))
  model.add(Dropout(0.5))
  model.add(Flatten())
  model.add(Dense(128, activation = 'relu'))
  model.add(Dense(64, activation = 'relu'))
  model.add(Dense(10, activation = 'softmax')) # final 10 class activation. softmax activation is because of crossentropy type
  model.summary()
  model.compile(loss = 'categorical_crossentropy', optimizer = 'adam', metrics = ['acc']) # because we're doing classification
  return model

def main():
  df = pd.read_csv('dados.csv')
  df.set_index('fname', inplace=True)

  for f in df.index:
      rate, signal = wavfile.read('clean/'+f)
      df.at[f, 'length'] = signal.shape[0]/rate

  classes = list(np.unique(df.label))
  class_dist = df.groupby(['label'])['length'].mean()

  n_samples = 2 * int(df['length'].sum() / 0.1)
  prob_dist = class_dist / class_dist.sum() # converts everything to the range between 0 and 1
  choices = np.random.choice(class_dist.index, p = prob_dist)

  fig, ax = plt.subplots()
  ax.set_title('Class Distribution', y=1.08)
  ax.pie(class_dist, labels=class_dist.index, autopct='%1.1f%%',
        shadow=False, startangle=90)
  ax.axis('equal')
  # plt.show()

  config = Config()
  ##class_dist, prob_dist, n_samples, df, config)
  X, y = build_rand_feat(class_dist, prob_dist, n_samples, df, config, classes)
  y_flat = np.argmax(y, axis=1)
  print('Tam: ', str(len(X)))
  input_shape = (X.shape[1], X.shape[2], 1) # give it to keras first layer

  model = get_conv_model(input_shape)

  class_weight = compute_class_weight('balanced', np.unique(y_flat), y_flat)

  checkpoint = ModelCheckpoint(config.model_path, monitor = 'val_acc', verbose = 1, mode = 'max', save_best_only = True, save_weights_only = False, period = 1)
  # period = every epoch

  model.fit(X, y, epochs = 10, batch_size = 32,
                  shuffle=True, class_weight = class_weight, validation_split = 0.1, callbacks = [checkpoint])
                  # take the top 10% of the matrix and send it to validation

  model.save(config.model_path)

if __name__ == '__main__':
  main()

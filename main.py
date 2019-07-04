import sounddevice as sd
import numpy as np
import time
import eda, model, predict
from scipy.io import wavfile
from tqdm import tqdm

DURATION = 10
NUMBER_OF_SAMPLES = 5
SAMPLE_RATE = 16000

def main():
  print('Ola! Quantos locutores voce deseja cadastrar?')
  print('Cada locutor ira gravar {} amostras de {} segundos cada.'.format(NUMBER_OF_SAMPLES, DURATION))
  qtd_locutores = int(input())
  with open('dados.csv', 'w') as f:
    f.write('fname,label\n')
    for i in range(qtd_locutores):
      print('Qual o nome do ' + str(i + 1) + 'o locutor?')
      nome = input()
      for j in range(NUMBER_OF_SAMPLES):
        print('Amostra ' + str(j + 1) + ', locutor = ' + nome + '. Pressione ENTER para iniciar a gravacao.')  
        enter = input()

        my_recording = sd.rec(DURATION * SAMPLE_RATE, samplerate=SAMPLE_RATE, channels=1)
        for t in tqdm(range(DURATION)):
          time.sleep(1)
        # scaled_data = np.int16(my_recording / np.max(np.abs(my_recording)) * 32767)
        print('Deseja confirmar essa gravacao? Y/N')
        option = input()
        if (option == 'Y' or option == 'y'):
          wav_dir = 'wavfiles/' + nome + str(j + 1) + '.wav' ## exemplo -> user_samples/locutor1.wav
          wavfile.write(wav_dir, SAMPLE_RATE, my_recording)
          f.write(nome + str(j + 1) + '.wav' + ',' + nome + '\n')
        else:
          j = j - 1
  # eda.main()
  # model.main()
  # predict.main()
  
if __name__ == '__main__':
  main()
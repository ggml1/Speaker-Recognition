from pydub import AudioSegment
import os

for file in os.listdir('wavfiles/'):
  sound = AudioSegment.from_wav('wavfiles/' + file)
  sound = sound.set_channels(1)
  sound.export('wavfiles/' + file, format='wav')
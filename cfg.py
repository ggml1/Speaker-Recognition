import os

class Config:
  def __init__(self, mode = 'conv', nfilt = 26, nfeat = 13, nfft = 512, rate = 16000):
    # mode = conv or recurrent network
    # nfeat (or ncep) = number of coefficients
    # nfilt = number of filters
    # nfft = 512. array was down-sampled. the window-size gave us 400 samples
    # we fill it with zeroes to complete the 2^9 power.
    self.mode = mode
    self.nfilt = nfilt
    self.nfeat = nfeat
    self.nfft = nfft
    self.rate = rate
    self.step = int(rate / 10)
    self.model_path = os.path.join('models', mode + '.model')
    self.p_path = os.path.join('pickles', mode + '.p')
import os

def main():
  print('Would you like to reset the network configurations?')
  choice = input()
  if (choice == 'Y' or choice == 'y'):
    # reset models, pickles and clean dir
    folders = ['clean/', 'models/', 'pickles/']
    for folder in folders:
      for file in os.listdir(folder):
        file_path = os.path.join(folder, file)
        try:
          if os.path.isfile(file_path):
            os.unlink(file_path)
        except Exception as e:
          print(e)
    print('Directories have been cleaned.')
    print('Ready to receive new inputs.')
    print('Remember to edit the .csv files accordingly.')

if __name__ == '__main__':
  main()
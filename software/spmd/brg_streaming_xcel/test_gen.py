import random
import argparse
import torch

random.seed(0xf00)

def generate(path, src, M, N, P):
  
  # Convert result bytes into list of ints
  tensor_arry = []
  for m in range(M):
    n_arry = []
    for n in range(N):
      p_arry = []
      for p in range(P):
        p_arry.append(src[p+n*P+m*P*N])
      n_arry.append(p_arry)
    tensor_arry.append(n_arry)
  ref = sum(torch.sum(torch.tensor(tensor_arry),0).tolist(),[]) 
  with open(path, 'w') as f:
    f.write("int M = {};\n".format(M))
    f.write("int N = {};\n".format(N))
    f.write("int P = {};\n".format(P))
    f.write('int src[] __attribute__ ( ( section(".dram") ) ) = {\n')
    for m in range(M):
      for n in range(N*P):
        f.write('{:4}, '.format(src[n+m*N*P]))
      f.write('\n')
    f.write('};\n')
    f.write('int ref[]  __attribute__ ( ( section(".dram") ) ) = {\n')
    for i in range(N*P):
      f.write('{:4}, '.format(ref[i]))
    f.write('};\n')
    f.write('int dest[{}]  __attribute__ ( ( section(".dram") ) );\n'.format(\
      N*P))


def main(argv):
  M = 2
  N = 2
  P = 2
  argc = len(argv)
  if argc > 0:
    M = argv[0]
  if argc > 1:
    N = argv[1]
  if argc > 2:
    P = argv[2]
  print("Generating Test File with Matrix Dimmension [{} {} {}]".format(\
    M,N,P))
  paths = [
    'streaming_xcel_test.dat', '../brg_stream/streaming_test.dat'
  ]
  src = [0]*M*N*P
  for i in range(M*N*P):
    src[i] = random.randint(0,100)
  for p in paths:
    generate(p,src, M, N, P)


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Process tests')
  parser.add_argument('integers', metavar='dim', type=int, nargs='*',
                    help='M N P to generate')
  args = parser.parse_args()
  main(list(args.integers))


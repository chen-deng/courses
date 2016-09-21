import numpy as np
import pdb

def uvec(i, dim):
    x = np.zeros((dim, 1), dtype=np.uint8)
    x[i] = 1
    return x
  
def generate_sequence(dim):
    idx = 2  
    seq = uvec(idx, dim)  
    while True:    
        idx = np.random.choice([idx + 1, idx - 1])
        if idx == -1:
            return seq, 0
        elif idx == DIM:
            return seq, 1
        else:
            seq = np.append(seq, uvec(idx, dim), axis=1)

def generate_training_set(dim):
    for _ in range(10):
        yield generate_sequence(dim)

def rms_diff(x1, x2):
    if x1.shape == x2.shape:
        return np.std(x1 - x2)
    else:
        raise RuntimeError("Mismatched dimensions")

if __name__ == "__main__":
    # Define constants
    DIM = 5
    
    # wts = np.random.random((DIM, 1))
    wts = np.ones((DIM, 1)) * 0.5
    lambda_ = 1.0
    alpha = 0.001
    wts_true = np.arange(1, 6).reshape((DIM,1))/6.0    
    
    # Initialize random number generator
    np.random.seed(1)
    
    for itrn in range(1000): 
        dwts = np.zeros((DIM, 1))
        for X, z in generate_training_set(DIM):     
            # pdb.set_trace()
            X = np.asmatrix(X)            
            P_next = np.dot(wts.T, X[:, 0])[0, 0]
            err = X[:, 0]
            _, m = X.shape    
            for t in range(m):
                P_prev = P_next                
                if t == m - 1:                
                    P_next = z
                    dwts += alpha * (P_next - P_prev) * err
                else:                
                    x_next = X[:, t + 1]
                    P_next = np.dot(wts.T, x_next)[0, 0]
                    dwts += alpha * (P_next - P_prev) * err
                    err = x_next + lambda_ * err               
        wts += dwts  
        print "iteration = %d, wts = %s, rms error = %0.2f" \
            % (itrn, wts.T, rms_diff(wts, wts_true))

    
    
    
    
    
      